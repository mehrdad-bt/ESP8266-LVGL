#include <Arduino.h>
#include <lvgl.h>

#include "screen_manager.h"

extern "C"
{
    #include "ui/screens.h"
}


// ==================================================
// CURRENT SCREEN
// ==================================================

static enum ScreensEnum current_screen =
    SCREEN_ID_MAIN;


// ==================================================
// SHOW SCREEN OBJECT
// ==================================================

static void show_screen_object(
    enum ScreensEnum screen
)
{
    lv_obj_t *screen_obj =
        NULL;


    // ==================================================
    // FIND SCREEN OBJECT
    // ==================================================

    switch (
        screen
    )
    {
        case SCREEN_ID_MAIN:

            screen_obj =
                objects.main;

            break;


        case SCREEN_ID_SETTINGS:

            screen_obj =
                objects.settings_page;

            break;


        case SCREEN_ID_BUZZER:

            screen_obj =
                objects.buzzer_settings;

            break;


        default:

            Serial.println(
                "SCREEN ERROR: UNKNOWN SCREEN"
            );

            return;
    }


    // ==================================================
    // CHECK OBJECT
    // ==================================================

    if (
        screen_obj == NULL
    )
    {
        Serial.println(
            "SCREEN ERROR: SCREEN OBJECT NULL"
        );

        return;
    }


    // ==================================================
    // LOAD SCREEN
    // ==================================================

    lv_scr_load(
        screen_obj
    );


    // ==================================================
    // FORCE INVALIDATE
    // ==================================================

    lv_obj_invalidate(
        screen_obj
    );


    // ==================================================
    // FORCE DISPLAY REFRESH
    // ==================================================

    lv_disp_t *disp =
        lv_disp_get_default();


    if (
        disp != NULL
    )
    {
        lv_refr_now(
            disp
        );
    }
}


// ==================================================
// INITIALIZE
// ==================================================

void screen_manager_init(void)
{
    current_screen =
        SCREEN_ID_MAIN;


    show_screen_object(
        SCREEN_ID_MAIN
    );


    Serial.println(
        "SCREEN MANAGER: MAIN"
    );
}


// ==================================================
// SHOW SCREEN
// ==================================================

void screen_manager_show(
    enum ScreensEnum screen
)
{
    // ==================================================
    // VALIDATE SCREEN
    // ==================================================

    switch (
        screen
    )
    {
        case SCREEN_ID_MAIN:

            if (
                objects.main == NULL
            )
            {
                Serial.println(
                    "SCREEN ERROR: MAIN OBJECT NULL"
                );

                return;
            }

            break;


        case SCREEN_ID_SETTINGS:

            if (
                objects.settings_page == NULL
            )
            {
                Serial.println(
                    "SCREEN ERROR: SETTINGS OBJECT NULL"
                );

                return;
            }

            break;


        case SCREEN_ID_BUZZER:

            if (
                objects.buzzer_settings == NULL
            )
            {
                Serial.println(
                    "SCREEN ERROR: BUZZER OBJECT NULL"
                );

                return;
            }

            break;


        default:

            Serial.println(
                "SCREEN ERROR: INVALID SCREEN"
            );

            return;
    }


    // ==================================================
    // IMPORTANT
    //
    // حتی اگر صفحه فعلی همان صفحه باشد،
    // باز هم آن را reload می‌کنیم.
    //
    // برای Calibration لازم است چون TFT مستقیماً
    // توسط tft.fillScreen() پاک شده است.
    // ==================================================

    bool same_screen =
        (
            current_screen ==
            screen
        );


    // ==================================================
    // SET CURRENT SCREEN
    // ==================================================

    current_screen =
        screen;


    // ==================================================
    // LOAD / RELOAD SCREEN
    // ==================================================

    show_screen_object(
        screen
    );


    // ==================================================
    // DEBUG
    // ==================================================

    Serial.print(
        "SCREEN -> "
    );


    switch (
        screen
    )
    {
        case SCREEN_ID_MAIN:

            Serial.println(
                same_screen
                ? "MAIN (RELOAD)"
                : "MAIN"
            );

            break;


        case SCREEN_ID_SETTINGS:

            Serial.println(
                same_screen
                ? "SETTINGS (RELOAD)"
                : "SETTINGS"
            );

            break;


        case SCREEN_ID_BUZZER:

            Serial.println(
                same_screen
                ? "BUZZER (RELOAD)"
                : "BUZZER"
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
// FORCE RELOAD CURRENT SCREEN
// ==================================================

void screen_manager_reload(void)
{
    Serial.println(
        "SCREEN RELOAD"
    );


    show_screen_object(
        current_screen
    );


    Serial.print(
        "SCREEN RELOADED -> "
    );


    switch (
        current_screen
    )
    {
        case SCREEN_ID_MAIN:

            Serial.println(
                "MAIN"
            );

            break;


        case SCREEN_ID_SETTINGS:

            Serial.println(
                "SETTINGS"
            );

            break;


        case SCREEN_ID_BUZZER:

            Serial.println(
                "BUZZER"
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
    return (
        current_screen ==
        screen
    );
}