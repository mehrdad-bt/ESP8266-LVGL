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
// LOAD SCREEN NOW
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
            "SCREEN ERROR: OBJECT NULL"
        );

        return;
    }

    Serial.println(
        "SCREEN APPLY START"
    );

    // --------------------------------------------------
    // No animation
    // --------------------------------------------------

    lv_scr_load(
        screen_obj
    );

    // --------------------------------------------------
    // Update custom screen state
    // --------------------------------------------------

    current_screen =
        screen;

    Serial.print(
        "SCREEN APPLIED -> "
    );

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

        default:

            Serial.println(
                "UNKNOWN"
            );

            break;
    }

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

    lv_obj_t *main_screen =
        objects.main;

    if (main_screen == NULL)
    {
        Serial.println(
            "SCREEN ERROR: MAIN OBJECT NULL"
        );

        return;
    }

    lv_scr_load(
        main_screen
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

    Serial.print(
        "SCREEN REQUEST -> "
    );

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

        default:

            Serial.println(
                "UNKNOWN"
            );

            break;
    }
}

// ==================================================
// PROCESS SCREEN CHANGE
// ==================================================

void screen_manager_process(void)
{
    if (!screen_change_pending)
    {
        return;
    }

    enum ScreensEnum requested_screen =
        pending_screen;

    screen_change_pending =
        false;

    Serial.println(
        "SCREEN PROCESS"
    );

    if (
        requested_screen ==
        current_screen
    )
    {
        Serial.println(
            "SCREEN PROCESS: SAME SCREEN"
        );

        return;
    }

    load_screen_now(
        requested_screen
    );
}

// ==================================================
// RELOAD
// ==================================================

void screen_manager_reload(void)
{
    pending_screen =
        current_screen;

    screen_change_pending =
        true;

    Serial.println(
        "SCREEN RELOAD REQUESTED"
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
// IS SCREEN
// ==================================================

bool screen_manager_is(
    enum ScreensEnum screen
)
{
    return (
        current_screen ==
        screen
    );
}