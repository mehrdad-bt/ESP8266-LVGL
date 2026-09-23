#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

extern "C"
{
#include "ui/ui.h"
#include "ui/screens.h"
}

#include "uart.h"
#include "tasks.h"
#include "screen_manager.h"

// ==================================================
// Watchdog Configuration
// ==================================================

#define WDT_TIMEOUT_SECONDS 4

// ==================================================
// Display Configuration
// ==================================================

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// --------------------------------------------------
// LVGL Draw Buffer
// RGB565 = 2 bytes per pixel
//
// 320 x 10 x 2 = 6400 bytes
// --------------------------------------------------

#define LVGL_BUF_LINES 20

TFT_eSPI tft = TFT_eSPI();

static lv_disp_draw_buf_t draw_buf;

static lv_color_t buf1[
    SCREEN_WIDTH * LVGL_BUF_LINES
];

// ==================================================
// Touch Calibration
// ==================================================

static uint16_t calData[5] =
{
    351,
    3465,
    306,
    3446,
    7
};

// ==================================================
// LVGL Display Flush
// ==================================================

static void my_disp_flush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p
)
{
    uint32_t w =
        area->x2 - area->x1 + 1;

    uint32_t h =
        area->y2 - area->y1 + 1;

    // --------------------------------------------------
    // Start SPI transaction
    // --------------------------------------------------

    tft.startWrite();

    // --------------------------------------------------
    // Set display drawing window
    // --------------------------------------------------

    tft.setAddrWindow(
        area->x1,
        area->y1,
        w,
        h
    );

    // --------------------------------------------------
    // Send pixel data
    // --------------------------------------------------

    tft.pushColors(
        (uint16_t *)&color_p->full,
        w * h,
        true
    );

    // --------------------------------------------------
    // End SPI transaction
    // --------------------------------------------------

    tft.endWrite();

    // --------------------------------------------------
    // Tell LVGL that flushing is finished
    // --------------------------------------------------

    lv_disp_flush_ready(
        disp
    );
}

// ==================================================
// LVGL Touch Input
// ==================================================

static void my_touchpad_read(
    lv_indev_drv_t *indev_drv,
    lv_indev_data_t *data
)
{
    uint16_t x;
    uint16_t y;

    (void)indev_drv;

    // --------------------------------------------------
    // Read touch
    // --------------------------------------------------

    if (tft.getTouch(&x, &y))
    {
        data->state =
            LV_INDEV_STATE_PR;

        data->point.x =
            x;

        data->point.y =
            y;
    }
    else
    {
        data->state =
            LV_INDEV_STATE_REL;
    }
}

// ==================================================
// Setup
// ==================================================

void setup()
{
    // --------------------------------------------------
    // Initialize Serial / UART
    // --------------------------------------------------

    serial_init();

    // --------------------------------------------------
    // Enable ESP8266 watchdog
    // --------------------------------------------------

    ESP.wdtEnable(
        WDT_TIMEOUT_SECONDS * 1000
    );

    // --------------------------------------------------
    // Initialize TFT
    // --------------------------------------------------

    tft.begin();

    // Landscape
    // 320 x 240
    tft.setRotation(1);

    // --------------------------------------------------
    // Initialize Touch
    // --------------------------------------------------

    tft.setTouch(
        calData
    );

    // --------------------------------------------------
    // Initialize LVGL
    // --------------------------------------------------

    lv_init();

    // ==================================================
    // LVGL Draw Buffer
    // ==================================================

    lv_disp_draw_buf_init(
        &draw_buf,
        buf1,
        NULL,
        SCREEN_WIDTH * LVGL_BUF_LINES
    );

    // ==================================================
    // Register LVGL Display Driver
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

    lv_disp_drv_register(
        &disp_drv
    );

    // ==================================================
    // Register LVGL Touch Driver
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

    // ==================================================
    // Initialize EEZ Studio UI
    // ==================================================

    ui_init();

    // ==================================================
    // LED Diagnostic Style
    //
    // Keep LED visible but disable expensive
    // visual properties before screen manager redraw.
    // ==================================================

    if (objects.obj0 != NULL)
    {
        // ------------------------------------------------
        // Disable rounded corners
        // ------------------------------------------------

        lv_obj_set_style_radius(
            objects.obj0,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        // ------------------------------------------------
        // Disable shadow
        // ------------------------------------------------

        lv_obj_set_style_shadow_width(
            objects.obj0,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_shadow_spread(
            objects.obj0,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        // ------------------------------------------------
        // Disable border
        // ------------------------------------------------

        lv_obj_set_style_border_width(
            objects.obj0,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        // ------------------------------------------------
        // Full background opacity
        // ------------------------------------------------

        lv_obj_set_style_bg_opa(
            objects.obj0,
            LV_OPA_COVER,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // ==================================================
    // Initialize Screen Manager
    // ==================================================

    screen_manager_init();

    // ==================================================
    // Initialize Application Tasks
    // ==================================================

    tasks_init();
}

// ==================================================
// Main Loop
// ==================================================

void loop()
{
    // --------------------------------------------------
    // Keep watchdog alive
    // --------------------------------------------------

    ESP.wdtFeed();

    // --------------------------------------------------
    // Run application tasks
    // --------------------------------------------------

    tasks_run();

    // --------------------------------------------------
    // Process LVGL
    // --------------------------------------------------

    lv_timer_handler();

    // --------------------------------------------------
    // Feed watchdog again
    // --------------------------------------------------

    ESP.wdtFeed();

    // --------------------------------------------------
    // Give ESP8266 background processes CPU time
    // --------------------------------------------------

    yield();
}