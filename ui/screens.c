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
            // Next Page Button
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 233, 167);
            lv_obj_set_size(obj, 82, 50);
            lv_obj_add_event_cb(
                obj,
                action_next_page,
                LV_EVENT_PRESSED,
                (void *)0
            );

            {
                lv_obj_t *parent_obj = obj;

                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(
                        obj,
                        LV_SIZE_CONTENT,
                        LV_SIZE_CONTENT
                    );

                    lv_obj_set_style_align(
                        obj,
                        LV_ALIGN_CENTER,
                        LV_PART_MAIN | LV_STATE_DEFAULT
                    );

                    lv_label_set_text_static(
                        obj,
                        "next page"
                    );
                }
            }
        }

        {
            // Calibrate Button
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 13, 167);
            lv_obj_set_size(obj, 79, 50);
            lv_obj_add_event_cb(
                obj,
                action_calibrate,
                LV_EVENT_PRESSED,
                (void *)0
            );

            {
                lv_obj_t *parent_obj = obj;

                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(
                        obj,
                        LV_SIZE_CONTENT,
                        LV_SIZE_CONTENT
                    );

                    lv_obj_set_style_align(
                        obj,
                        LV_ALIGN_CENTER,
                        LV_PART_MAIN | LV_STATE_DEFAULT
                    );

                    lv_label_set_text_static(
                        obj,
                        "Calibrate"
                    );
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
            // Status LED
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 242, 44);
            lv_obj_set_size(obj, 32, 32);

            lv_led_set_color(
                obj,
                lv_color_hex(0x0000FF)
            );

            lv_led_set_brightness(
                obj,
                255
            );
        }

        {
            // STATUS
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 233, 12);
            lv_obj_set_size(
                obj,
                LV_SIZE_CONTENT,
                LV_SIZE_CONTENT
            );

            lv_label_set_text_static(
                obj,
                "STATUS"
            );
        }

        {
            // VOLTAGE:
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 11, 44);
            lv_obj_set_size(
                obj,
                LV_SIZE_CONTENT,
                LV_SIZE_CONTENT
            );

            lv_label_set_text_static(
                obj,
                "VOLTAGE:"
            );
        }

        {
            // CURRENT:
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 11, 68);
            lv_obj_set_size(
                obj,
                LV_SIZE_CONTENT,
                LV_SIZE_CONTENT
            );

            lv_label_set_text_static(
                obj,
                "CURRENT:"
            );
        }

        {
            // ==========================================
            // ERROR BOX
            // ==========================================

            lv_obj_t *obj = lv_msgbox_create(
                parent_obj,
                "",
                "",
                0,
                true
            );

            objects.error_box = obj;

            lv_obj_set_pos(
                obj,
                70,
                80
            );

            lv_obj_set_size(
                obj,
                180,
                100
            );

            // Background
            lv_obj_set_style_bg_color(
                obj,
                lv_color_hex(0x8B0000),
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            lv_obj_set_style_bg_opa(
                obj,
                LV_OPA_COVER,
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            // Border
            lv_obj_set_style_border_color(
                obj,
                lv_color_hex(0xFF4444),
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            lv_obj_set_style_border_width(
                obj,
                2,
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            // Rounded corners
            lv_obj_set_style_radius(
                obj,
                10,
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            // Hide at startup
            lv_obj_add_flag(
                obj,
                LV_OBJ_FLAG_HIDDEN
            );
        }

        {
            // ==========================================
            // LOW VOLTAGE LABEL
            // ==========================================

            lv_obj_t *obj = lv_label_create(parent_obj);

            objects.low_voltage_label = obj;

            lv_obj_set_pos(
                obj,
                103,
                126
            );

            lv_obj_set_size(
                obj,
                LV_SIZE_CONTENT,
                LV_SIZE_CONTENT
            );

            lv_label_set_text_static(
                obj,
                "LOW VOLTAGE !"
            );

            // White text
            lv_obj_set_style_text_color(
                obj,
                lv_color_hex(0xFFFFFF),
                LV_PART_MAIN | LV_STATE_DEFAULT
            );

            // Hide at startup
            lv_obj_add_flag(
                obj,
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    tick_screen_main();
}
void tick_screen_main() {
}

void create_screen_page_2() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.page_2 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 110, 177);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_prev_page, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Back");
                }
            }
        }
    }
    
    tick_screen_page_2();
}

void tick_screen_page_2() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_page_2,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 2) {
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
    create_screen_page_2();
}