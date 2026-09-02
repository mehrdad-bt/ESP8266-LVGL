#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

extern "C" {
    #include "ui/ui.h"
}

TFT_eSPI tft = TFT_eSPI();

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * 5];


// --------------------------------------------------
// LVGL -> TFT_eSPI
// --------------------------------------------------

void my_disp_flush(lv_disp_drv_t *disp,
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
        (uint16_t *)&color_p->full,
        w * h,
        true
    );

    tft.endWrite();

    lv_disp_flush_ready(disp);
}


// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    // -------------------------
    // TFT
    // -------------------------

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);


    // -------------------------
    // LVGL
    // -------------------------

    lv_init();


    // -------------------------
    // LVGL Draw Buffer
    // -------------------------

    lv_disp_draw_buf_init(
        &draw_buf,
        buf,
        NULL,
        320 * 5
    );


    // -------------------------
    // LVGL Display Driver
    // -------------------------

    static lv_disp_drv_t disp_drv;

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;

    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);


    // -------------------------
    // EEZ Studio UI
    // -------------------------

    ui_init();
}


// --------------------------------------------------
// Loop
// --------------------------------------------------

void loop()
{
    lv_timer_handler();

    ui_tick();

    delay(5);
}