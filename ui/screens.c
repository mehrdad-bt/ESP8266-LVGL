#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Voltage
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.voltage = obj;
            lv_obj_set_pos(obj, 92, 45);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Voltage");
        }
        {
            // BTN_SETTINGS
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.btn_settings = obj;
            lv_obj_set_pos(obj, 233, 167);
            lv_obj_set_size(obj, 82, 50);
            lv_obj_add_event_cb(obj, action_go_to_settings_page, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "SETTINGS");
                }
            }
        }
        {
            // Current
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.current = obj;
            lv_obj_set_pos(obj, 92, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Current");
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 242, 44);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0x0000ff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 233, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "STATUS");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 11, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "VOLTAGE:");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 11, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "CURRENT:");
        }
        {
            // Error Box
            lv_obj_t *obj = lv_msgbox_create(parent_obj, "", "", 0, true);
            objects.error_box = obj;
            lv_obj_set_pos(obj, 70, 80);
            lv_obj_set_size(obj, 180, 100);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // error_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.error_label = obj;
            lv_obj_set_pos(obj, 103, 126);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_settings_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // EXIT_SETTINGS
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.exit_settings = obj;
            lv_obj_set_pos(obj, 257, 195);
            lv_obj_set_size(obj, 52, 34);
            lv_obj_add_event_cb(obj, action_exit_to_main_page, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "EXIT");
                }
            }
        }
        {
            // BUZZER
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.buzzer = obj;
            lv_obj_set_pos(obj, 8, 49);
            lv_obj_set_size(obj, 110, 42);
            lv_obj_add_event_cb(obj, action_go_to_buzzer_settings, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BUZZER");
                }
            }
        }
        {
            // TOUCH_CALIBRATION
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.touch_calibration = obj;
            lv_obj_set_pos(obj, 8, 109);
            lv_obj_set_size(obj, 110, 41);
            lv_obj_add_event_cb(obj, action_go_to_touch_calibration, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, " CALIBRATION");
                }
            }
        }
        {
            // VOLTAGE RANGE
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.voltage_range = obj;
            lv_obj_set_pos(obj, 9, 167);
            lv_obj_set_size(obj, 110, 43);
            lv_obj_add_event_cb(obj, action_go_to_v_c_range_settings, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "V/C RANGE");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 134, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "SETTINGS");
        }
    }
    
    tick_screen_settings_page();
}

void tick_screen_settings_page() {
}

void create_screen_buzzer_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.buzzer_settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_add_event_cb(obj, action_go_from_buzzer_settings_page_to_settings_page, LV_EVENT_PRESSED, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            lv_obj_set_pos(obj, 85, 102);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_dropdown_set_options_static(obj, "MODE 1\nMODE 2\nMODE 3");
            lv_dropdown_set_selected(obj, 0);
        }
        {
            // buzzer_settings_page_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.buzzer_settings_page_label = obj;
            lv_obj_set_pos(obj, 94, 24);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "BUZZER SETTINGS");
        }
        {
            // buzzer_settings_page_back_button
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.buzzer_settings_page_back_button = obj;
            lv_obj_set_pos(obj, 16, 189);
            lv_obj_set_size(obj, 78, 41);
            lv_obj_add_event_cb(obj, action_go_from_buzzer_settings_page_to_settings_page, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BACK");
                }
            }
        }
    }
    
    tick_screen_buzzer_settings();
}

void tick_screen_buzzer_settings() {
}

void create_screen_v_c_range_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.v_c_range_settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 47, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "VOLTAGE AND CURRENT SETTINGS");
        }
        {
            // voltage_minimum
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.voltage_minimum = obj;
            lv_obj_set_pos(obj, 61, 83);
            lv_obj_set_size(obj, 150, 10);
            lv_slider_set_range(obj, 0, 30);
            lv_slider_set_value(obj, 20, LV_ANIM_ON);
            lv_obj_add_event_cb(obj, action_voltage_min_changed, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 6, 80);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "V Min");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 6, 119);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "V Max");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 4, 154);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "C Min");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 4, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "C Max");
        }
        {
            // voltage_min_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.voltage_min_value = obj;
            lv_obj_set_pos(obj, 237, 80);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Text");
        }
        {
            // voltage_max_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.voltage_max_value = obj;
            lv_obj_set_pos(obj, 237, 112);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Text");
        }
        {
            // current_min_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.current_min_value = obj;
            lv_obj_set_pos(obj, 237, 154);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Text");
        }
        {
            // current_max_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.current_max_value = obj;
            lv_obj_set_pos(obj, 237, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "Text");
        }
        {
            // voltage_maximum
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.voltage_maximum = obj;
            lv_obj_set_pos(obj, 61, 123);
            lv_obj_set_size(obj, 150, 10);
            lv_slider_set_range(obj, 0, 30);
            lv_slider_set_value(obj, 24, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, action_voltage_max_changed, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // current_minimum
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.current_minimum = obj;
            lv_obj_set_pos(obj, 61, 160);
            lv_obj_set_size(obj, 150, 10);
            lv_slider_set_range(obj, 0, 3);
            lv_slider_set_value(obj, 1, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, action_current_min_changed, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // current_maximum
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.current_maximum = obj;
            lv_obj_set_pos(obj, 61, 194);
            lv_obj_set_size(obj, 150, 10);
            lv_slider_set_range(obj, 0, 3);
            lv_slider_set_value(obj, 2, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, action_current_max_changed, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
    }
    
    tick_screen_v_c_range_settings();
}

void tick_screen_v_c_range_settings() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_settings_page,
    tick_screen_buzzer_settings,
    tick_screen_v_c_range_settings,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 4) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_settings_page();
    create_screen_buzzer_settings();
    create_screen_v_c_range_settings();
}