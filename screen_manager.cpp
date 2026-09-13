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

static enum ScreensEnum current_screen = SCREEN_ID_MAIN;

static enum ScreensEnum pending_screen = SCREEN_ID_MAIN;

static bool screen_change_pending = false;


// ==================================================
// GET SCREEN OBJECT
// این تابع فقط هنگام APPLY کردن صفحه استفاده می‌شود
// نه هنگام REQUEST
// ==================================================

static lv_obj_t *get_screen_object(enum ScreensEnum screen)
{
    switch (screen)
    {
        case SCREEN_ID_MAIN:
            return objects.main;

        case SCREEN_ID_SETTINGS:
            return objects.settings_page;

        case SCREEN_ID_BUZZER:
            return objects.buzzer_settings;

        case SCREEN_ID_V_C_RANGE:
            return objects.v_c_range_settings;

        default:
            return NULL;
    }
}


// ==================================================
// APPLY SCREEN
// فقط خارج از event handler و خارج از lv_timer_handler
// ==================================================

static void load_screen_now(enum ScreensEnum screen)
{
    lv_obj_t *screen_obj = get_screen_object(screen);

    if (screen_obj == NULL)
    {
        Serial.println("SCREEN ERROR: OBJECT NULL");
        return;
    }

    Serial.println("SCREEN APPLY START");

    // فقط خود صفحه را Load می‌کنیم
    lv_scr_load(screen_obj);

    current_screen = screen;

    Serial.print("SCREEN APPLIED -> ");

    switch (screen)
    {
        case SCREEN_ID_MAIN:
            Serial.println("MAIN");
            break;

        case SCREEN_ID_SETTINGS:
            Serial.println("SETTINGS");
            break;

        case SCREEN_ID_BUZZER:
            Serial.println("BUZZER");
            break;

        case SCREEN_ID_V_C_RANGE:
            Serial.println("V/C RANGE");
            break;

        default:
            Serial.println("UNKNOWN");
            break;
    }

    Serial.println("SCREEN APPLY END");
}


// ==================================================
// INIT
// ==================================================

void screen_manager_init(void)
{
    current_screen = SCREEN_ID_MAIN;
    pending_screen = SCREEN_ID_MAIN;
    screen_change_pending = false;

    lv_obj_t *main_screen = objects.main;

    if (main_screen == NULL)
    {
        Serial.println("SCREEN ERROR: MAIN OBJECT NULL");
        return;
    }

    lv_scr_load(main_screen);

    Serial.println("SCREEN MANAGER: MAIN");
}


// ==================================================
// REQUEST SCREEN
//
// مهم:
// این تابع نباید هیچ عملیات LVGL انجام دهد.
// فقط request را ثبت می‌کند.
// ==================================================

void screen_manager_show(enum ScreensEnum screen)
{
    pending_screen = screen;
    screen_change_pending = true;

    Serial.print("SCREEN REQUEST -> ");

    switch (screen)
    {
        case SCREEN_ID_MAIN:
            Serial.println("MAIN");
            break;

        case SCREEN_ID_SETTINGS:
            Serial.println("SETTINGS");
            break;

        case SCREEN_ID_BUZZER:
            Serial.println("BUZZER");
            break;

        case SCREEN_ID_V_C_RANGE:
            Serial.println("V/C RANGE");
            break;

        default:
            Serial.println("UNKNOWN");
            break;
    }
}


// ==================================================
// PROCESS PENDING SCREEN
//
// این تابع باید قبل از lv_timer_handler() اجرا شود.
// ==================================================

void screen_manager_process(void)
{
    if (!screen_change_pending)
        return;

    enum ScreensEnum requested_screen = pending_screen;

    // اول pending را پاک می‌کنیم
    screen_change_pending = false;

    Serial.println("SCREEN PROCESS");

    // اگر همان صفحه فعلی است، کاری نکن
    if (requested_screen == current_screen)
    {
        Serial.println("SCREEN PROCESS: SAME SCREEN");
        return;
    }

    load_screen_now(requested_screen);
}


// ==================================================
// RELOAD CURRENT SCREEN
// ==================================================

void screen_manager_reload(void)
{
    pending_screen = current_screen;
    screen_change_pending = true;

    Serial.println("SCREEN RELOAD REQUESTED");
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

bool screen_manager_is(enum ScreensEnum screen)
{
    return current_screen == screen;
}