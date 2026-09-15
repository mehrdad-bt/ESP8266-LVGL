#include "actions.h"
#include "screens.h"
#include "screen_manager.h"
#include "touchCalibration.h"
#include "tasks.h"

#include <lvgl.h>

// ==================================================
// Theme State
// ==================================================

static bool dark_theme_active = false;

// ==================================================
// Theme Objects
// ==================================================

static lv_theme_t *light_theme = NULL;
static lv_theme_t *dark_theme  = NULL;

// ==================================================
// Apply Light Theme
// ==================================================

static void apply_light_theme(void)
{
    lv_disp_t *disp = lv_disp_get_default();

    if (disp == NULL)
        return;

    // Create Light Theme only once
    if (light_theme == NULL)
    {
        light_theme =
            lv_theme_default_init(
                disp,
                lv_palette_main(LV_PALETTE_BLUE),
                lv_palette_main(LV_PALETTE_RED),
                false,
                LV_FONT_DEFAULT
            );
    }

    if (light_theme == NULL)
        return;

    lv_disp_set_theme(
        disp,
        light_theme
    );

    lv_obj_report_style_change(NULL);

    if (lv_scr_act() != NULL)
    {
        lv_obj_invalidate(
            lv_scr_act()
        );
    }

    dark_theme_active = false;
}

// ==================================================
// Apply Dark Theme
// ==================================================

static void apply_dark_theme(void)
{
    lv_disp_t *disp = lv_disp_get_default();

    if (disp == NULL)
        return;

    // Create Dark Theme only once
    if (dark_theme == NULL)
    {
        dark_theme =
            lv_theme_default_init(
                disp,
                lv_palette_main(LV_PALETTE_BLUE),
                lv_palette_main(LV_PALETTE_RED),
                true,
                LV_FONT_DEFAULT
            );
    }

    if (dark_theme == NULL)
        return;

    lv_disp_set_theme(
        disp,
        dark_theme
    );

    lv_obj_report_style_change(NULL);

    if (lv_scr_act() != NULL)
    {
        lv_obj_invalidate(
            lv_scr_act()
        );
    }

    dark_theme_active = true;
}

// ==================================================
// Update Theme Checkboxes
// ==================================================

static void update_theme_checkboxes(void)
{
    if (objects.light_theme != NULL)
    {
        if (dark_theme_active)
        {
            lv_obj_clear_state(
                objects.light_theme,
                LV_STATE_CHECKED
            );
        }
        else
        {
            lv_obj_add_state(
                objects.light_theme,
                LV_STATE_CHECKED
            );
        }
    }

    if (objects.dark_theme != NULL)
    {
        if (dark_theme_active)
        {
            lv_obj_add_state(
                objects.dark_theme,
                LV_STATE_CHECKED
            );
        }
        else
        {
            lv_obj_clear_state(
                objects.dark_theme,
                LV_STATE_CHECKED
            );
        }
    }
}

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
// Hide / Show Voltage and Current
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
    // Voltage
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
    // Current
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
    // Voltage Label
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
    // Current Label
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
}

// ==================================================
// Voltage Minimum
// ==================================================

void action_voltage_min_changed(lv_event_t *e)
{
    (void)e;

    if (objects.voltage_minimum == NULL)
        return;

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

    if (objects.voltage_maximum == NULL)
        return;

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

    if (objects.current_minimum == NULL)
        return;

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

    if (objects.current_maximum == NULL)
        return;

    int32_t value =
        lv_slider_get_value(
            objects.current_maximum
        );

    set_var_current_max(
        (float)value
    );
}

// ==================================================
// Select Light Theme
// ==================================================

void action_select_light_theme(lv_event_t *e)
{
    (void)e;

    apply_light_theme();

    update_theme_checkboxes();
}

// ==================================================
// Select Dark Theme
// ==================================================

void action_select_dark_theme(lv_event_t *e)
{
    (void)e;

    apply_dark_theme();

    update_theme_checkboxes();
}

// ==================================================
// Go To Themes Page
// ==================================================

void action_go_to_themes_page(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_THEMES
    );
}

// ==================================================
// Exit Themes Page
// ==================================================

void action_exit_from_themes_page(lv_event_t *e)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS_PAGE
    );
}