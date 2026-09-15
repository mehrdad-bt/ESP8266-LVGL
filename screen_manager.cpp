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

        case SCREEN_ID_THEMES:
            return objects.themes;

        default:
            return NULL;
    }
}

// ==================================================
// Print Screen Name
// ==================================================

static void print_screen_name(
    enum ScreensEnum screen
)
{
    switch (screen)
    {
        case SCREEN_ID_MAIN:
            Serial.println(
                "MAIN"
            );
            break;

        case SCREEN_ID_SETTINGS_PAGE:
            Serial.println(
                "SETTINGS"
            );
            break;

        case SCREEN_ID_BUZZER_SETTINGS:
            Serial.println(
                "BUZZER"
            );
            break;

        case SCREEN_ID_V_C_RANGE_SETTINGS:
            Serial.println(
                "V/C RANGE"
            );
            break;

        case SCREEN_ID_THEMES:
            Serial.println(
                "THEMES"
            );
            break;

        default:
            Serial.println(
                "UNKNOWN"
            );
            break;
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
        Serial.println(
            "SCREEN ERROR: NULL SCREEN OBJECT"
        );

        return;
    }

    Serial.print(
        "SCREEN APPLY START -> "
    );

    print_screen_name(
        screen
    );

    // ------------------------------------------------
    // Load screen
    // ------------------------------------------------

    lv_scr_load(
        screen_obj
    );

    // ------------------------------------------------
    // Update current screen state
    // ------------------------------------------------

    current_screen =
        screen;

    // ------------------------------------------------
    // Request redraw only
    // ------------------------------------------------

    lv_obj_invalidate(
        screen_obj
    );

    Serial.println(
        "SCREEN APPLY END"
    );
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
        Serial.println(
            "SCREEN MANAGER ERROR: MAIN IS NULL"
        );

        return;
    }

    lv_scr_load(
        main_screen
    );

    lv_obj_invalidate(
        main_screen
    );

    Serial.println(
        "SCREEN MANAGER: MAIN LOADED"
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

    Serial.print(
        "SCREEN REQUEST -> "
    );

    print_screen_name(
        screen
    );
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

    Serial.println(
        "SCREEN REQUEST -> RELOAD"
    );
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

    // ------------------------------------------------
    // Clear pending request BEFORE processing
    // ------------------------------------------------

    screen_change_pending =
        false;

    screen_reload_pending =
        false;

    // ------------------------------------------------
    // Ignore same screen unless reload requested
    // ------------------------------------------------

    if (
        !reload &&
        requested_screen ==
        current_screen
    )
    {
        Serial.println(
            "SCREEN PROCESS -> SAME SCREEN"
        );

        return;
    }

    Serial.println(
        "SCREEN PROCESS"
    );

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