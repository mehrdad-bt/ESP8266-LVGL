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
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS,
    SCREEN_ID_BUZZER,
    SCREEN_ID_V_C_RANGE
};


// ==================================================
// OBJECTS
// ==================================================

typedef struct
{
    // -------------------------
    // Screens
    // -------------------------

    lv_obj_t *main;

    lv_obj_t *settings_page;

    lv_obj_t *buzzer_settings;

    lv_obj_t *v_c_range_settings;


    // -------------------------
    // Main
    // -------------------------

    lv_obj_t *voltage;

    lv_obj_t *current;

    lv_obj_t *btn_settings;

    lv_obj_t *obj0;

    lv_obj_t *error_box;

    lv_obj_t *low_voltage_label;


    // -------------------------
    // Settings
    // -------------------------

    lv_obj_t *exit_settings;

    lv_obj_t *buzzer;

    lv_obj_t *touch_calibration;

    lv_obj_t *voltage_range;


    // -------------------------
    // Buzzer
    // -------------------------

    lv_obj_t *buzzer_options;

    lv_obj_t *buzzer_settings_page_label;

    lv_obj_t *buzzer_settings_page_back_button;


    // -------------------------
    // V/C Range
    // -------------------------

    lv_obj_t *voltage_minimum;

    lv_obj_t *voltage_maximum;

    lv_obj_t *current_minimum;

    lv_obj_t *current_maximum;

    lv_obj_t *voltage_min_value;

    lv_obj_t *voltage_max_value;

    lv_obj_t *current_min_value;

    lv_obj_t *current_max_value;

} objects_t;


extern objects_t objects;

extern lv_obj_t *tick_value_change_obj;


// ==================================================
// SCREEN FUNCTIONS
// ==================================================

void create_screen_main(void);

void create_screen_settings_page(void);

void create_screen_buzzer_settings(void);

void create_screen_v_c_range_settings(void);

void tick_screen_main(void);

void tick_screen_settings_page(void);

void tick_screen_buzzer_settings(void);

void tick_screen_v_c_range_settings(void);

void tick_screen(
    int screen_index
);

void tick_screen_by_id(
    enum ScreensEnum screenId
);

void create_screens(void);


#ifdef __cplusplus
}
#endif

#endif