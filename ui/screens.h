#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_BUZZER = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *buzzer;
    lv_obj_t *voltage;
    lv_obj_t *settings_button;
    lv_obj_t *current;
    lv_obj_t *obj0;
    lv_obj_t *error_box;
    lv_obj_t *low_voltage_label;
    lv_obj_t *back_button;
    lv_obj_t *buzzer_button;
    lv_obj_t *theme_button;
    lv_obj_t *buzzer_options;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void create_screen_buzzer();
void tick_screen_buzzer();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/