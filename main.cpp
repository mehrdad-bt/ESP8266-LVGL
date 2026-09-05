#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

#include "touchCalibration.h"
#include "uart.h"

extern "C" {
    #include "ui/ui.h"
}


// ==================================================
// TFT
// ==================================================

TFT_eSPI tft = TFT_eSPI();


// ==================================================
// DISPLAY
// ==================================================

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


// ==================================================
// TOUCH CALIBRATION
// ==================================================

uint16_t calData[5] = {
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

static lv_color_t buf[SCREEN_WIDTH * 20];


// ==================================================
// DISPLAY FLUSH
// ==================================================

void my_disp_flush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

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
    lv_indev_data_t *data)
{
    uint16_t x;
    uint16_t y;

    bool pressed = tft.getTouch(&x, &y);

    if (pressed)
    {
        data->point.x = x;
        data->point.y = y;

        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}


// ==================================================
// SETUP
// ==================================================

void setup()
{
    // ==================================================
    // UART
    // ==================================================

    serial_init();

    Serial.println();
    Serial.println("================================");
    Serial.println("ESP8266 + LVGL + TFT + TOUCH");
    Serial.println("================================");


    // ==================================================
    // TFT
    // ==================================================

    tft.begin();

    tft.setRotation(1);

    // اعمال Calibration اولیه
    tft.setTouch(calData);

    tft.fillScreen(TFT_BLACK);

    Serial.println("TFT initialized");


    // ==================================================
    // LVGL
    // ==================================================

    lv_init();

    Serial.println("LVGL initialized");


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

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;

    disp_drv.flush_cb = my_disp_flush;

    disp_drv.draw_buf = &draw_buf;

    // مناسب برای ESP8266
    disp_drv.full_refresh = 0;

    lv_disp_drv_register(&disp_drv);

    Serial.println("Display driver registered");


    // ==================================================
    // TOUCH DRIVER
    // ==================================================

    static lv_indev_drv_t indev_drv;

    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;

    indev_drv.read_cb = my_touchpad_read;

    lv_indev_drv_register(&indev_drv);

    Serial.println("Touch driver registered");


    // ==================================================
    // EEZ STUDIO UI
    // ==================================================

    ui_init();

    Serial.println("EEZ Studio UI initialized");


    // ==================================================
    // CHECK MAIN PAGE LABEL
    // ==================================================

    if (objects.main_page_label != NULL)
    {
        Serial.println("Main Page label found");
    }
    else
    {
        Serial.println("ERROR: Main Page label not found!");
    }


    // ==================================================
    // SETUP COMPLETE
    // ==================================================

    Serial.println("--------------------------------");
    Serial.println("Setup complete");
    Serial.println("Send text from Serial Monitor...");
    Serial.println("--------------------------------");
}


// ==================================================
// LOOP
// ==================================================

void loop()
{
    // ==================================================
    // UART
    // ==================================================

    uart_receive();


    // ==================================================
    // LVGL
    // ==================================================

    lv_timer_handler();


    // ==================================================
    // EEZ STUDIO
    // ==================================================

    ui_tick();


    // ==================================================
    // Small delay
    // ==================================================

    delay(5);
}