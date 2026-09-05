
#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"


objects_t objects;


//
// Event handlers
//

lv_obj_t *tick_value_change_obj;


// ==================================================
// ARC → PERCENT LABEL
// ==================================================

static void arc_percent_event_cb(lv_event_t *e)
{
    // گرفتن Arc
    lv_obj_t *arc = lv_event_get_target(e);

    // گرفتن Label که به عنوان user_data فرستاده شده
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);

    // گرفتن مقدار Arc
    int value = lv_arc_get_value(arc);

    // ساخت متن درصد
    char text[8];
    snprintf(text, sizeof(text), "%d %%", value);

    // نمایش مقدار در Label
    lv_label_set_text(label, text);
}


//
// Screens
//

void create_screen_main()
{
    lv_obj_t *obj = lv_obj_create(0);

    objects.main = obj;

    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);

    {
        lv_obj_t *parent_obj = obj;


        // ==================================================
        // TITLE
        // ==================================================

        {
            
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_page_label = obj;

            lv_obj_set_pos(obj, 122, 134);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

            lv_label_set_text_static(obj, "Main Page");
        }


        // ==================================================
        // NEXT BUTTON
        // ==================================================

        {
            lv_obj_t *obj = lv_btn_create(parent_obj);

            lv_obj_set_pos(obj, 110, 167);
            lv_obj_set_size(obj, 100, 50);

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

                    lv_label_set_text_static(obj, "Next");
                }
            }
        }


        // ==================================================
        // ARC + PERCENT LABEL
        // ==================================================

        {
            // ------------------------------
            // ARC
            // ------------------------------

            lv_obj_t *arc = lv_arc_create(parent_obj);

            lv_obj_set_pos(arc, 25, 13);
            lv_obj_set_size(arc, 85, 104);

            // محدوده Arc
            lv_arc_set_range(
                arc,
                0,
                100
            );

            // مقدار اولیه
            lv_arc_set_value(
                arc,
                100
            );


            // ------------------------------
            // LABEL
            // ------------------------------

            lv_obj_t *label = lv_label_create(parent_obj);

            lv_obj_set_pos(
                label,
                53,
                48
            );

            lv_obj_set_size(
                label,
                LV_SIZE_CONTENT,
                LV_SIZE_CONTENT
            );

            // مقدار اولیه Label
            lv_label_set_text(
                label,
                "100 %"
            );


            // ------------------------------
            // ARC EVENT
            // ------------------------------

            lv_obj_add_event_cb(
                arc,
                arc_percent_event_cb,
                LV_EVENT_VALUE_CHANGED,
                label
            );
        }


        // ==================================================
        // SPINNER
        // ==================================================

        {
            lv_obj_t *obj = lv_spinner_create(
                parent_obj,
                1000,
                60
            );

            lv_obj_set_pos(
                obj,
                223,
                16
            );

            lv_obj_set_size(
                obj,
                80,
                80
            );
        }
    }

    tick_screen_main();
}


void tick_screen_main()
{
}


// ==================================================
// PAGE 2
// ==================================================

void create_screen_page_2()
{
    lv_obj_t *obj = lv_obj_create(0);

    objects.page_2 = obj;

    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);

    {
        lv_obj_t *parent_obj = obj;


        // ==================================================
        // BACK BUTTON
        // ==================================================

        {
            lv_obj_t *obj = lv_btn_create(parent_obj);

            lv_obj_set_pos(
                obj,
                110,
                177
            );

            lv_obj_set_size(
                obj,
                100,
                50
            );

            lv_obj_add_event_cb(
                obj,
                action_prev_page,
                LV_EVENT_PRESSED,
                (void *)0
            );

            {
                lv_obj_t *parent_obj = obj;

                {
                    lv_obj_t *obj = lv_label_create(parent_obj);

                    lv_obj_set_pos(
                        obj,
                        0,
                        0
                    );

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
                        "Back"
                    );
                }
            }
        }


        // ==================================================
        // COLOR WHEEL
        // ==================================================

        {
            lv_obj_t *obj = lv_colorwheel_create(
                parent_obj,
                false
            );

            lv_obj_set_pos(
                obj,
                102,
                20
            );

            lv_obj_set_size(
                obj,
                125,
                122
            );
        }
    }

    tick_screen_page_2();
}


void tick_screen_page_2()
{
}


// ==================================================
// SCREEN TICK FUNCTIONS
// ==================================================

typedef void (*tick_screen_func_t)();

tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_page_2,
};


void tick_screen(int screen_index)
{
    if (
        screen_index >= 0 &&
        screen_index < 2
    )
    {
        tick_screen_funcs[screen_index]();
    }
}


void tick_screen_by_id(enum ScreensEnum screenId)
{
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

void create_screens()
{
    // Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();

    lv_theme_t *theme =
        lv_theme_default_init(
            dispp,
            lv_palette_main(LV_PALETTE_BLUE),
            lv_palette_main(LV_PALETTE_RED),
            false,
            LV_FONT_DEFAULT
        );

    lv_disp_set_theme(
        dispp,
        theme
    );


    // Initialize screens

    create_screen_main();

    create_screen_page_2();
}

