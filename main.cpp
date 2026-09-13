
#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

extern "C"
{
    #include "ui/ui.h"
    #include "ui/screens.h"
}

#include "touchCalibration.h"
#include "uart.h"
#include "tasks.h"
#include "screen_manager.h"

// ==================================================
// TFT
// ==================================================

TFT_eSPI tft = TFT_eSPI();

// Touch calibration data
uint16_t calData[5] = {351, 3465, 306, 3446, 7};

// ==================================================
// SCREEN
// ==================================================

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ==================================================
// LVGL DRAW BUFFER
// ==================================================

static lv_disp_draw_buf_t draw_buf;

static lv_color_t buf1[SCREEN_WIDTH * 10];

// ==================================================
// DISPLAY FLUSH
// ==================================================

void my_disp_flush(
    lv_disp_drv_t *disp_drv,
    const lv_area_t *area,
    lv_color_t *color_p)
{
    uint32_t w =
        area->x2 - area->x1 + 1;

    uint32_t h =
        area->y2 - area->y1 + 1;

    tft.startWrite();

    tft.setAddrWindow(
        area->x1,
        area->y1,
        w,
        h
    );

    tft.pushColors(
        (uint16_t *)&color_p->full,
        w * h,
        true
    );

    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}

// ==================================================
// TOUCH DEBUG STATE
// ==================================================

static bool touch_debug_active = false;

static uint32_t touch_debug_last_print = 0;

#define TOUCH_DEBUG_INTERVAL 200UL

// ==================================================
// TOUCH INPUT
// ==================================================

void my_touchpad_read(
    lv_indev_drv_t *indev_drv,
    lv_indev_data_t *data)
{
    (void)indev_drv;

    uint16_t x = 0;
    uint16_t y = 0;

    bool touched =
        tft.getTouch(
            &x,
            &y
        );

    // =================================================
    // TOUCHED
    // =================================================

    if (touched)
    {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;

        uint32_t now = millis();

        // Print immediately on first touch
        if (!touch_debug_active)
        {
            touch_debug_active = true;

            touch_debug_last_print = now;

            Serial.print("TOUCH: X=");
            Serial.print(x);

            Serial.print(" Y=");
            Serial.println(y);
        }

        // Print coordinates while finger is moving
        else if (
            now - touch_debug_last_print >=
            TOUCH_DEBUG_INTERVAL
        )
        {
            touch_debug_last_print = now;

            Serial.print("TOUCH: X=");
            Serial.print(x);

            Serial.print(" Y=");
            Serial.println(y);
        }
    }

    // =================================================
    // RELEASED
    // =================================================

    else
    {
        data->state = LV_INDEV_STATE_REL;

        if (touch_debug_active)
        {
            touch_debug_active = false;

            Serial.println(
                "TOUCH: RELEASE"
            );
        }
    }
}

// ==================================================
// LVGL MEMORY DEBUG
// ==================================================

void print_lvgl_memory(
    const char *point
)
{
    lv_mem_monitor_t mon;

    lv_mem_monitor(&mon);

    Serial.println();
    Serial.println(
        "========== LVGL MEMORY =========="
    );

    Serial.print("POINT: ");
    Serial.println(point);

    Serial.print("TOTAL SIZE       = ");
    Serial.println(mon.total_size);

    Serial.print("FREE SIZE        = ");
    Serial.println(mon.free_size);

    Serial.print("FREE BIGGEST     = ");
    Serial.println(mon.free_biggest_size);

    Serial.print("USED PERCENT     = ");
    Serial.print(mon.used_pct);
    Serial.println(" %");

    Serial.print("FRAGMENTATION    = ");
    Serial.print(mon.frag_pct);
    Serial.println(" %");

    Serial.println(
        "================================="
    );
}

// ==================================================
// SETUP
// ==================================================

void setup()
{
    Serial.begin(115200);

    delay(200);

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "UART initialized"
    );

    Serial.println(
        "Baud rate: 115200"
    );

    Serial.println(
        "================================"
    );

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "ESP8266 + LVGL + TFT + TOUCH"
    );

    Serial.println(
        "================================"
    );

    Serial.print(
        "ESP FREE HEAP AT START = "
    );

    Serial.println(
        ESP.getFreeHeap()
    );

    // =================================================
    // WATCHDOG
    // =================================================

    ESP.wdtEnable(WDTO_4S);

    Serial.println(
        "Watchdog enabled: 4 seconds"
    );

    // =================================================
    // TFT
    // =================================================

    tft.begin();

    tft.setRotation(1);

    tft.setTouch(calData);

    Serial.println(
        "TFT initialized"
    );

    // =================================================
    // LVGL
    // =================================================

    lv_init();

    Serial.println(
        "LVGL initialized"
    );

    print_lvgl_memory(
        "AFTER LV_INIT"
    );

    // =================================================
    // DRAW BUFFER
    // =================================================

    lv_disp_draw_buf_init(
        &draw_buf,
        buf1,
        NULL,
        SCREEN_WIDTH * 10
    );

    Serial.println(
        "Draw buffer initialized"
    );

    print_lvgl_memory(
        "AFTER DRAW BUFFER"
    );

    // =================================================
    // DISPLAY DRIVER
    // =================================================

    static lv_disp_drv_t disp_drv;

    lv_disp_drv_init(
        &disp_drv
    );

    disp_drv.hor_res =
        SCREEN_WIDTH;

    disp_drv.ver_res =
        SCREEN_HEIGHT;

    disp_drv.flush_cb =
        my_disp_flush;

    disp_drv.draw_buf =
        &draw_buf;

    lv_disp_drv_register(
        &disp_drv
    );

    Serial.println(
        "Display driver registered"
    );

    print_lvgl_memory(
        "AFTER DISPLAY DRIVER"
    );

    // =================================================
    // TOUCH DRIVER
    // =================================================

    static lv_indev_drv_t indev_drv;

    lv_indev_drv_init(
        &indev_drv
    );

    indev_drv.type =
        LV_INDEV_TYPE_POINTER;

    indev_drv.read_cb =
        my_touchpad_read;

    lv_indev_drv_register(
        &indev_drv
    );

    Serial.println(
        "Touch driver registered"
    );

    print_lvgl_memory(
        "AFTER INPUT DRIVER"
    );

    // =================================================
    // UART
    // =================================================

    serial_init();

    // =================================================
    // EEZ UI
    // =================================================

    ui_init();

    Serial.println(
        "EEZ Studio UI initialized"
    );

    print_lvgl_memory(
        "AFTER UI"
    );

    Serial.print(
        "ESP FREE HEAP AFTER UI = "
    );

    Serial.println(
        ESP.getFreeHeap()
    );

    // =================================================
    // SCREEN MANAGER
    // =================================================

    screen_manager_init();

    // =================================================
    // TASKS
    // =================================================

    tasks_init();

    Serial.println(
        "Tasks initialized"
    );

    print_lvgl_memory(
        "AFTER TASKS"
    );

    // =================================================
    // DEBUG OBJECT CHECK
    // =================================================

    if (objects.voltage != NULL)
    {
        Serial.println(
            "Voltage label found"
        );
    }
    else
    {
        Serial.println(
            "Voltage label NOT found"
        );
    }

    if (objects.current != NULL)
    {
        Serial.println(
            "Current label found"
        );
    }
    else
    {
        Serial.println(
            "Current label NOT found"
        );
    }

    if (objects.obj0 != NULL)
    {
        Serial.println(
            "Status LED found"
        );
    }
    else
    {
        Serial.println(
            "Status LED NOT found"
        );
    }

    if (objects.error_box != NULL)
    {
        Serial.println(
            "Error box found"
        );
    }
    else
    {
        Serial.println(
            "Error box NOT found"
        );
    }

    if (
        objects.low_voltage_label != NULL
    )
    {
        Serial.println(
            "Low voltage label found"
        );
    }
    else
    {
        Serial.println(
            "Low voltage label NOT found"
        );
    }

    if (
        objects.v_c_range_settings != NULL
    )
    {
        Serial.println(
            "V/C Range screen found"
        );
    }
    else
    {
        Serial.println(
            "V/C Range screen NOT found"
        );
    }

    print_lvgl_memory(
        "BEFORE LOOP"
    );

    Serial.print(
        "ESP FREE HEAP BEFORE LOOP = "
    );

    Serial.println(
        ESP.getFreeHeap()
    );

    Serial.println(
        "--------------------------------"
    );

    Serial.println(
        "Setup complete"
    );

    Serial.println(
        "Send: 23.75,0.82"
    );

    Serial.println(
        "--------------------------------"
    );
}

// ==================================================
// LOOP
// ==================================================

void loop()
{
    ESP.wdtFeed();

    tasks_run();

    ESP.wdtFeed();

    yield();
}

