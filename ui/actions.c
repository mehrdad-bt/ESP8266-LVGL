
#include "actions.h"
#include "screens.h"
#include "screen_manager.h"
#include "touchCalibration.h"
#include "tasks.h"

// ==================================================
// Go To Settings
// ==================================================

void action_go_to_settings_page(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS_PAGE
    );
}

// ==================================================
// Exit To Main
// ==================================================

void action_exit_to_main_page(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_MAIN
    );
}

// ==================================================
// Go To Buzzer Settings
// ==================================================

void action_go_to_buzzer_settings(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_BUZZER_SETTINGS
    );
}

// ==================================================
// Go To Touch Calibration
// ==================================================

void action_go_to_touch_calibration(lv_event_t *e)
{
    (void)e;

    touch_calibration_start();

    screen_manager_reload();
}

// ==================================================
// Go To V/C Range Settings
// ==================================================

void action_go_to_v_c_range_settings(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_V_C_RANGE_SETTINGS
    );
}

// ==================================================
// Buzzer Settings -> Settings
// ==================================================

void action_go_from_buzzer_settings_page_to_settings_page(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS_PAGE
    );
}

// ==================================================
// V/C Range -> Settings
// ==================================================

void action_exit_from_v_c_menu_to_settings(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS_PAGE
    );
}

// ==================================================
// Hide / Show Voltage and Current Parameters
// ==================================================

void action_hide_voltage_and_current_parameters_main_screen(
    lv_event_t *e
)
{
    (void)e;

    if (objects.voltage_current_hide == NULL)
        return;

    bool hide =
        lv_obj_has_state(
            objects.voltage_current_hide,
            LV_STATE_CHECKED
        );

    // --------------------------------------------------
    // Voltage value
    // --------------------------------------------------

    if (objects.voltage != NULL)
    {
        if (hide)
        {
            lv_obj_add_flag(
                objects.voltage,
                LV_OBJ_FLAG_HIDDEN
            );
        }
        else
        {
            lv_obj_clear_flag(
                objects.voltage,
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    // --------------------------------------------------
    // Current value
    // --------------------------------------------------

    if (objects.current != NULL)
    {
        if (hide)
        {
            lv_obj_add_flag(
                objects.current,
                LV_OBJ_FLAG_HIDDEN
            );
        }
        else
        {
            lv_obj_clear_flag(
                objects.current,
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    // --------------------------------------------------
    // Voltage label
    // --------------------------------------------------

    if (objects.voltage_label_main != NULL)
    {
        if (hide)
        {
            lv_obj_add_flag(
                objects.voltage_label_main,
                LV_OBJ_FLAG_HIDDEN
            );
        }
        else
        {
            lv_obj_clear_flag(
                objects.voltage_label_main,
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    // --------------------------------------------------
    // Current label
    // --------------------------------------------------

    if (objects.current_label_main != NULL)
    {
        if (hide)
        {
            lv_obj_add_flag(
                objects.current_label_main,
                LV_OBJ_FLAG_HIDDEN
            );
        }
        else
        {
            lv_obj_clear_flag(
                objects.current_label_main,
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    // --------------------------------------------------
    // Refresh changed objects
    // --------------------------------------------------

    if (objects.voltage != NULL)
    {
        lv_obj_invalidate(
            objects.voltage
        );
    }

    if (objects.current != NULL)
    {
        lv_obj_invalidate(
            objects.current
        );
    }

    if (objects.voltage_label_main != NULL)
    {
        lv_obj_invalidate(
            objects.voltage_label_main
        );
    }

    if (objects.current_label_main != NULL)
    {
        lv_obj_invalidate(
            objects.current_label_main
        );
    }
}

// ==================================================
// Voltage Minimum
// ==================================================

void action_voltage_min_changed(lv_event_t *e)
{
    (void)e;

    if (
        objects.voltage_minimum == NULL
    )
    {
        return;
    }

    int32_t value =
        lv_slider_get_value(
            objects.voltage_minimum
        );

    set_var_voltage_min(
        (float)value
    );
}

// ==================================================
// Voltage Maximum
// ==================================================

void action_voltage_max_changed(lv_event_t *e)
{
    (void)e;

    if (
        objects.voltage_maximum == NULL
    )
    {
        return;
    }

    int32_t value =
        lv_slider_get_value(
            objects.voltage_maximum
        );

    set_var_voltage_max(
        (float)value
    );
}

// ==================================================
// Current Minimum
// ==================================================

void action_current_min_changed(lv_event_t *e)
{
    (void)e;

    if (
        objects.current_minimum == NULL
    )
    {
        return;
    }

    int32_t value =
        lv_slider_get_value(
            objects.current_minimum
        );

    set_var_current_min(
        (float)value
    );
}

// ==================================================
// Current Maximum
// ==================================================

void action_current_max_changed(lv_event_t *e)
{
    (void)e;

    if (
        objects.current_maximum == NULL
    )
    {
        return;
    }

    int32_t value =
        lv_slider_get_value(
            objects.current_maximum
        );

    set_var_current_max(
        (float)value
    );
}

