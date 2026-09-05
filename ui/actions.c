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


static void led_blink_timer(lv_timer_t *timer)
{
    static bool state = false;

    state = !state;

    if (state)
    {
        lv_led_on(objects.obj0);
    }
    else
    {
        lv_led_off(objects.obj0);
    }
}


void action_led_change_color(lv_event_t *e)
{
    static bool blinking = false;
    static lv_timer_t *blink_timer = NULL;

    blinking = !blinking;

    if (blinking)
    {
        lv_led_on(objects.obj0);

        blink_timer = lv_timer_create(
            led_blink_timer,
            500,
            NULL
        );
    }
    else
    {
        if (blink_timer != NULL)
        {
            lv_timer_del(blink_timer);
            blink_timer = NULL;
        }

        lv_led_on(objects.obj0);
    }
}