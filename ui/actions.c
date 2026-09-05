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