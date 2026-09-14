#include <Arduino.h>
#include <lvgl.h>

#include "screen_manager.h"

extern "C"
{
#include "ui/screens.h"
}

// ==================================================
// Screen Manager State
// ==================================================

static enum ScreensEnum current_screen =
    SCREEN_ID_MAIN;

static enum ScreensEnum pending_screen =
    SCREEN_ID_MAIN;

static bool screen_change_pending =
    false;

static bool screen_reload_pending =
    false;

// ==================================================
// Get LVGL Object for Screen
// ==================================================

static lv_obj_t *get_screen_object(
    enum ScreensEnum screen
)
{
    switch (screen)
    {
        case SCREEN_ID_MAIN:
            return objects.main;

        case SCREEN_ID_SETTINGS_PAGE:
            return objects.settings_page;

        case SCREEN_ID_BUZZER_SETTINGS:
            return objects.buzzer_settings;

        case SCREEN_ID_V_C_RANGE_SETTINGS:
            return objects.v_c_range_settings;

        default:
            return NULL;
    }
}

// ==================================================
// Load Screen Immediately
// ==================================================

static void load_screen_now(
    enum ScreensEnum screen
)
{
    lv_obj_t *screen_obj =
        get_screen_object(
            screen
        );

    if (screen_obj == NULL)
    {
        return;
    }

    lv_scr_load(
        screen_obj
    );

    lv_obj_invalidate(
        screen_obj
    );

    lv_refr_now(
        lv_disp_get_default()
    );

    current_screen =
        screen;
}

// ==================================================
// Initialize Screen Manager
// ==================================================

void screen_manager_init(void)
{
    current_screen =
        SCREEN_ID_MAIN;

    pending_screen =
        SCREEN_ID_MAIN;

    screen_change_pending =
        false;

    screen_reload_pending =
        false;

    lv_obj_t *main_screen =
        objects.main;

    if (main_screen == NULL)
    {
        return;
    }

    lv_scr_load(
        main_screen
    );

    lv_obj_invalidate(
        main_screen
    );

    lv_refr_now(
        lv_disp_get_default()
    );
}

// ==================================================
// Request Screen Change
// ==================================================

void screen_manager_show(
    enum ScreensEnum screen
)
{
    pending_screen =
        screen;

    screen_change_pending =
        true;

    screen_reload_pending =
        false;
}

// ==================================================
// Request Current Screen Reload
// ==================================================

void screen_manager_reload(void)
{
    pending_screen =
        current_screen;

    screen_change_pending =
        true;

    screen_reload_pending =
        true;
}

// ==================================================
// Process Pending Screen Request
// ==================================================

void screen_manager_process(void)
{
    if (!screen_change_pending)
    {
        return;
    }

    enum ScreensEnum requested_screen =
        pending_screen;

    bool reload =
        screen_reload_pending;

    screen_change_pending =
        false;

    screen_reload_pending =
        false;

    if (
        !reload &&
        requested_screen ==
        current_screen
    )
    {
        return;
    }

    load_screen_now(
        requested_screen
    );
}

// ==================================================
// Get Current Screen
// ==================================================

enum ScreensEnum screen_manager_get(void)
{
    return current_screen;
}

// ==================================================
// Check Current Screen
// ==================================================

bool screen_manager_is(
    enum ScreensEnum screen
)
{
    return current_screen ==
           screen;
}