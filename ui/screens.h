#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================================================
// SCREEN ENUM
// ==================================================

enum ScreensEnum
{
    _SCREEN_ID_FIRST = 1,

    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_BUZZER = 3,

    _SCREEN_ID_LAST = 3
};


// ==================================================
// OBJECTS
// ==================================================

typedef struct _objects_t
{
    // ----------------------------------------------
    // MAIN
    // ----------------------------------------------

    lv_obj_t *main;

    lv_obj_t *voltage;
    lv_obj_t *current;

    lv_obj_t *btn_settings;

    lv_obj_t *obj0;

    lv_obj_t *error_box;
    lv_obj_t *low_voltage_label;


    // ----------------------------------------------
    // SETTINGS
    // ----------------------------------------------

    lv_obj_t *settings_page;

    lv_obj_t *exit_settings;

    lv_obj_t *buzzer;

    lv_obj_t *touch_calibration;

    lv_obj_t *voltage_range;


    // ----------------------------------------------
    // BUZZER SETTINGS
    // ----------------------------------------------

    lv_obj_t *buzzer_settings;

    lv_obj_t *buzzer_options;

    lv_obj_t *buzzer_settings_page_label;

    lv_obj_t *buzzer_settings_page_back_button;

} objects_t;


// ==================================================
// GLOBAL OBJECTS
// ==================================================

extern objects_t objects;


// ==================================================
// MAIN
// ==================================================

void create_screen_main(void);
void tick_screen_main(void);


// ==================================================
// SETTINGS
// ==================================================

void create_screen_settings_page(void);
void tick_screen_settings_page(void);


// ==================================================
// BUZZER
// ==================================================

void create_screen_buzzer_settings(void);
void tick_screen_buzzer_settings(void);


// ==================================================
// SCREEN TICK
// ==================================================

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);


// ==================================================
// CREATE ALL
// ==================================================

void create_screens(void);

#ifdef __cplusplus
}
#endif

#endif