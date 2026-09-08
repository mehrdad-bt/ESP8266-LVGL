#include <Arduino.h>

#include "screen_manager.h"
#include "ui/ui.h"


// ==================================================
// CURRENT SCREEN
// ==================================================

static enum ScreensEnum current_screen =
    SCREEN_ID_MAIN;


// ==================================================
// INITIALIZE
// ==================================================

void screen_manager_init(void)
{
    current_screen =
        SCREEN_ID_MAIN;

    loadScreen(
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
    if (
        current_screen ==
        screen
    )
    {
        return;
    }


    current_screen =
        screen;


    loadScreen(
        screen
    );


    Serial.print(
        "SCREEN -> "
    );


    switch (screen)
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