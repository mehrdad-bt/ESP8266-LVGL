#include "actions.h"
#include "screens.h"

#include "../screen_manager.h"
#include "../touchCalibration.h"


// ==================================================
// GO TO SETTINGS
// ==================================================

void action_go_to_settings_page(
    lv_event_t *e
)
{
    (void)e;


    screen_manager_show(
        SCREEN_ID_SETTINGS
    );
}


// ==================================================
// EXIT SETTINGS -> MAIN
// ==================================================

void action_exit_to_main_page(
    lv_event_t *e
)
{
    (void)e;


    screen_manager_show(
        SCREEN_ID_MAIN
    );
}


// ==================================================
// SETTINGS -> BUZZER
// ==================================================

void action_go_to_buzzer_settings(
    lv_event_t *e
)
{
    (void)e;


    screen_manager_show(
        SCREEN_ID_BUZZER
    );
}


// ==================================================
// TOUCH CALIBRATION
// ==================================================

void action_go_to_touch_calibration(
    lv_event_t *e
)
{
    (void)e;


    // ==================================================
    // DEBUG
    // ==================================================

    // شروع Calibration
    // این تابع blocking است و تا اتمام Calibration
    // برنمی‌گردد.
    // ==================================================

    touch_calibration_start();


    // ==================================================
    // بعد از پایان Calibration
    //
    // current_screen همچنان SETTINGS است،
    // بنابراین باید Settings را FORCE RELOAD کنیم.
    // ==================================================

    screen_manager_reload();
}


// ==================================================
// BUZZER -> SETTINGS
// ==================================================

void action_go_from_buzzer_settings_page_to_settings_page(
    lv_event_t *e
)
{
    (void)e;


    screen_manager_show(
        SCREEN_ID_SETTINGS
    );
}