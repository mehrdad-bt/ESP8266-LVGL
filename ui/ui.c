
#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

#include <string.h>

static int16_t currentScreen = -1;


// ==================================================
// LOAD SCREEN
// ==================================================

void loadScreen(enum ScreensEnum screenId)
{
    currentScreen = screenId - 1;

    lv_obj_t *screen = NULL;

    switch (screenId)
    {
        case SCREEN_ID_MAIN:
            screen = objects.main;
            break;

        case SCREEN_ID_SETTINGS:
            screen = objects.settings_page;
            break;

        case SCREEN_ID_BUZZER:
            screen = objects.buzzer_settings;
            break;

        case SCREEN_ID_V_C_RANGE:
            screen = objects.v_c_range_settings;
            break;

        default:
            currentScreen = -1;
            return;
    }

    if (screen == NULL)
    {
        currentScreen = -1;
        return;
    }

    // =================================================
    // NO SCREEN ANIMATION
    // =================================================

    lv_scr_load(screen);
}


// ==================================================
// UI INIT
// ==================================================

void ui_init(void)
{
    create_screens();

    loadScreen(SCREEN_ID_MAIN);
}


// ==================================================
// UI TICK
// ==================================================

void ui_tick(void)
{
    if (
        currentScreen >= 0 &&
        currentScreen < 4
    )
    {
        tick_screen(currentScreen);
    }
}

