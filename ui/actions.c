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
// SETTINGS -> V/C RANGE
// ==================================================

void action_go_to_v_c_range_settings(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_V_C_RANGE
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

    /*
     * Calibration is blocking.
     *
     * IMPORTANT:
     * Do not touch LVGL input driver here.
     */

    touch_calibration_start();

    /*
     * After calibration TFT was probably cleared
     * directly, therefore reload current screen.
     */

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


// ==================================================
// V/C RANGE -> SETTINGS
// ==================================================

void action_go_from_v_c_range_settings_page_to_settings_page(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS
    );
}