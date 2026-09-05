#include <Arduino.h>

#include "actions.h"
#include "screens.h"
#include "touchCalibration.h"


// ==================================================
// TOUCH CALIBRATION
// ==================================================

void action_calibrate(lv_event_t *e)
{
    touch_calibration_start();

    // برگشت به صفحه اصلی
    loadScreen(SCREEN_ID_MAIN);

    // درخواست redraw کامل صفحه
    lv_obj_invalidate(lv_scr_act());

    // اجرای فوری redraw
    lv_refr_now(NULL);
}


// ==================================================
// NEXT PAGE
// ==================================================

void action_next_page(lv_event_t *e)
{
    loadScreen(SCREEN_ID_PAGE_2);
}


// ==================================================
// PREVIOUS PAGE
// ==================================================

void action_prev_page(lv_event_t *e)
{
    loadScreen(SCREEN_ID_MAIN);
}


void action_led_change_color(lv_event_t *e)
{
        static uint8_t color_index = 0;

    switch (color_index)
    {
        case 0:
            // قرمز
            lv_led_set_color(
                objects.obj0,
                lv_color_hex(0xFF0000)
            );
            break;

        case 1:
            // سبز
            lv_led_set_color(
                objects.obj0,
                lv_color_hex(0x00FF00)
            );
            break;

        case 2:
            // آبی
            lv_led_set_color(
                objects.obj0,
                lv_color_hex(0x0000FF)
            );
            break;
    }

    color_index++;

    if (color_index >= 3)
    {
        color_index = 0;
    }
}