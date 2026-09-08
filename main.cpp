#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

#include "touchCalibration.h"
#include "uart.h"
#include "tasks.h"

extern "C"
{
    #include "ui/ui.h"
    #include "ui/screens.h"
}


// ==================================================
// TFT
// ==================================================

TFT_eSPI tft = TFT_eSPI();


// ==================================================
// DISPLAY SIZE
// ==================================================

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


// ==================================================
// TOUCH CALIBRATION
// ==================================================

uint16_t calData[5] =
{
    351,
    3465,
    306,
    3446,
    7
};


// ==================================================
// LVGL DRAW BUFFER
// ==================================================

static lv_disp_draw_buf_t draw_buf;

static lv_color_t buf[
    SCREEN_WIDTH * 10
];


// ==================================================
// DISPLAY FLUSH
// ==================================================

void my_disp_flush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p
)
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
        (uint16_t *)color_p,
        w * h,
        true
    );

    tft.endWrite();


    lv_disp_flush_ready(disp);
}


// ==================================================
// TOUCH READ
// ==================================================

void my_touchpad_read(
    lv_indev_drv_t *indev_drv,
    lv_indev_data_t *data
)
{
    (void)indev_drv;

    uint16_t x;
    uint16_t y;


    bool pressed =
        tft.getTouch(
            &x,
            &y
        );


    if (pressed)
    {
        data->point.x = x;
        data->point.y = y;

        data->state =
            LV_INDEV_STATE_PR;
    }
    else
    {
        data->state =
            LV_INDEV_STATE_REL;
    }
}


// ==================================================
// SETUP
// ==================================================

void setup()
{
    // ==================================================
    // SERIAL
    // ==================================================

    serial_init();

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


    // ==================================================
    // WATCHDOG
    // ==================================================

    /*
     * Enable ESP8266 software watchdog.
     *
     * If the application gets stuck and
     * watchdog is not fed, ESP8266 resets.
     */

    ESP.wdtEnable(WDTO_4S);

    Serial.println(
        "Watchdog enabled: 4 seconds"
    );


    // ==================================================
    // TFT
    // ==================================================

    tft.begin();

    tft.setRotation(1);

    tft.setTouch(calData);

    tft.fillScreen(TFT_BLACK);

    Serial.println(
        "TFT initialized"
    );


    // ==================================================
    // LVGL
    // ==================================================

    lv_init();

    Serial.println(
        "LVGL initialized"
    );


    // ==================================================
    // DRAW BUFFER
    // ==================================================

    lv_disp_draw_buf_init(
        &draw_buf,
        buf,
        NULL,
        SCREEN_WIDTH * 10
    );


    // ==================================================
    // DISPLAY DRIVER
    // ==================================================

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

    disp_drv.full_refresh =
        0;


    lv_disp_drv_register(
        &disp_drv
    );


    Serial.println(
        "Display driver registered"
    );


    // ==================================================
    // TOUCH DRIVER
    // ==================================================

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


    // ==================================================
    // EEZ STUDIO UI
    // ==================================================

    ui_init();

    Serial.println(
        "EEZ Studio UI initialized"
    );


    // ==================================================
    // APPLICATION TASKS
    // ==================================================

    tasks_init();


    // ==================================================
    // CHECK UI OBJECTS
    // ==================================================

    if (objects.voltage != NULL)
    {
        Serial.println(
            "Voltage label found"
        );
    }
    else
    {
        Serial.println(
            "ERROR: Voltage label not found!"
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
            "ERROR: Current label not found!"
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
            "ERROR: Status LED not found!"
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
            "ERROR: Error box not found!"
        );
    }


    if (objects.low_voltage_label != NULL)
    {
        Serial.println(
            "Low voltage label found"
        );
    }
    else
    {
        Serial.println(
            "ERROR: Low voltage label not found!"
        );
    }


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


    // ==================================================
    // FIRST LVGL REFRESH
    // ==================================================

    lv_timer_handler();
}


// ==================================================
// LOOP
// ==================================================

void loop()
{
    /*
     * All application logic is executed
     * through the task manager.
     */

    tasks_run();


    /*
     * Give ESP8266 background services CPU time.
     */

    yield();

    delay(5);
}