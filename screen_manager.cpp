#include <Arduino.h>
#include <lvgl.h>

#include "screen_manager.h"

extern "C"
{
#include "ui/screens.h"
}

// ==================================================
// SCREEN STATE
// ==================================================

static enum ScreensEnum current_screen =
    SCREEN_ID_MAIN;

static enum ScreensEnum pending_screen =
    SCREEN_ID_MAIN;

static bool screen_change_pending =
    false;

// ==================================================
// RELOAD STATE
// ==================================================

static bool screen_reload_pending =
    false;

// ==================================================
// GET SCREEN OBJECT
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
// PRINT SCREEN NAME
// ==================================================

static void print_screen_name(
    enum ScreensEnum screen
)
{
    switch (screen)
    {
        case SCREEN_ID_MAIN:
            Serial.print("MAIN");
            break;

        case SCREEN_ID_SETTINGS_PAGE:
            Serial.print("SETTINGS");
            break;

        case SCREEN_ID_BUZZER_SETTINGS:
            Serial.print("BUZZER");
            break;

        case SCREEN_ID_V_C_RANGE_SETTINGS:
            Serial.print("V/C RANGE");
            break;

        default:
            Serial.print("UNKNOWN");
            break;
    }
}

// ==================================================
// LOAD SCREEN NOW
// ==================================================

static void load_screen_now(
    enum ScreensEnum screen
)
{
    lv_obj_t *screen_obj =
        get_screen_object(screen);

    if (screen_obj == NULL)
    {
        Serial.println(
            "SCREEN ERROR: OBJECT NULL"
        );

        return;
    }

    Serial.println(
        "SCREEN APPLY START"
    );

    // --------------------------------------------------
    // Load screen
    // --------------------------------------------------

    lv_scr_load(
        screen_obj
    );

    // --------------------------------------------------
    // Force complete screen invalidation
    // --------------------------------------------------

    lv_obj_invalidate(
        screen_obj
    );

    // --------------------------------------------------
    // Force immediate refresh
    // --------------------------------------------------

    lv_refr_now(
        lv_disp_get_default()
    );

    // --------------------------------------------------
    // Update state
    // --------------------------------------------------

    current_screen =
        screen;

    Serial.print(
        "SCREEN APPLIED -> "
    );

    print_screen_name(
        screen
    );

    Serial.println();

    Serial.println(
        "SCREEN APPLY END"
    );
}

// ==================================================
// INIT
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
            "SCREEN MANAGER ERROR: MAIN NULL"
        );

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

    Serial.println(
        "SCREEN MANAGER: MAIN"
    );
}

// ==================================================
// REQUEST SCREEN
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

    Serial.println();
}

// ==================================================
// REQUEST RELOAD
// ==================================================

void screen_manager_reload(void)
{
    pending_screen =
        current_screen;

    screen_change_pending =
        true;

    screen_reload_pending =
        true;

    Serial.print(
        "SCREEN RELOAD REQUESTED -> "
    );

    print_screen_name(
        current_screen
    );

    Serial.println();
}

// ==================================================
// PROCESS SCREEN REQUEST
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

    Serial.println(
        "SCREEN PROCESS"
    );

    // ==================================================
    // NORMAL SCREEN CHANGE
    // ==================================================

    if (
        !reload &&
        requested_screen ==
        current_screen
    )
    {
        Serial.println(
            "SCREEN PROCESS: SAME SCREEN"
        );

        return;
    }

    // ==================================================
    // RELOAD
    // ==================================================

    if (reload)
    {
        Serial.println(
            "SCREEN PROCESS: RELOAD"
        );
    }
    else
    {
        Serial.println(
            "SCREEN PROCESS: CHANGE"
        );
    }

    load_screen_now(
        requested_screen
    );
}

// ==================================================
// GET CURRENT SCREEN
// ==================================================

enum ScreensEnum screen_manager_get(void)
{
    return current_screen;
}

// ==================================================
// CHECK CURRENT SCREEN
// ==================================================

bool screen_manager_is(
    enum ScreensEnum screen
)
{
    return current_screen ==
           screen;
}