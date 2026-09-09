#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

extern "C"
{
    #include "ui/ui.h"
    #include "ui/screens.h"
    #include "ui/actions.h"
}

#include "uart.h"
#include "tasks.h"
#include "screen_manager.h"


// ==================================================
// PHYSICAL BUTTONS
// ==================================================

#define BTN_RIGHT   D1
#define BTN_SELECT  D4


// ==================================================
// BUZZER
// ==================================================

#define BUZZER_PIN       D0
#define BUZZER_FREQUENCY 2000


// ==================================================
// BUZZER PATTERN TIMING
// ==================================================

#define BUZZER_MODE1_ON      200
#define BUZZER_MODE1_OFF     700

#define BUZZER_MODE2_BEEP    100
#define BUZZER_MODE2_PAUSE   100
#define BUZZER_MODE2_REPEAT  900

#define BUZZER_MODE3_ON      500
#define BUZZER_MODE3_OFF     500


// ==================================================
// BUTTON DEBOUNCE
// ==================================================

#define BUTTON_DEBOUNCE 50UL


// ==================================================
// MAIN OPTIONS
// ==================================================

#define MAIN_OPTION_SETTINGS 0


// ==================================================
// SETTINGS OPTIONS
// ==================================================

#define SETTINGS_OPTION_BUZZER       0
#define SETTINGS_OPTION_CALIBRATION  1
#define SETTINGS_OPTION_VC_RANGE     2
#define SETTINGS_OPTION_BACK         3


// ==================================================
// BUZZER MODES
// ==================================================

#define BUZZER_MODE_1 0
#define BUZZER_MODE_2 1
#define BUZZER_MODE_3 2

#define BUZZER_MODE_COUNT 3


// ==================================================
// BUTTON STATE
// ==================================================

static uint8_t selected_option =
    MAIN_OPTION_SETTINGS;


static bool right_raw_state =
    HIGH;

static bool select_raw_state =
    HIGH;


static bool right_stable_state =
    HIGH;

static bool select_stable_state =
    HIGH;


static uint32_t right_last_change =
    0;

static uint32_t select_last_change =
    0;


// ==================================================
// LAST SCREEN
// ==================================================

static enum ScreensEnum button_last_screen =
    SCREEN_ID_MAIN;


// ==================================================
// LED COLORS
// ==================================================

#define LED_BLUE    0x0000FF
#define LED_GREEN   0x00FF00
#define LED_RED     0xFF0000
#define LED_ORANGE  0xFFA500


// ==================================================
// SYSTEM LIMITS
// ==================================================

#define VOLTAGE_MIN 20.0f
#define VOLTAGE_MAX 25.0f

#define CURRENT_MIN 0.0f
#define CURRENT_MAX 1.0f

#define UART_TIMEOUT 3000UL


// ==================================================
// SYSTEM STATE
// ==================================================

typedef struct
{
    float voltage;
    float current;

    bool data_received;

    bool uart_timeout;
    bool connection_lost;

    bool voltage_ok;
    bool current_ok;
    bool system_ok;

    bool low_voltage;

} SystemState;


static SystemState system_state =
{
    0.0f,
    0.0f,

    false,

    false,
    false,

    false,
    false,
    false,

    false
};


// ==================================================
// TIMING
// ==================================================

static uint32_t last_uart_data =
    0;

static uint32_t last_lvgl =
    0;


// ==================================================
// BUZZER STATE
// ==================================================

static bool buzzer_state =
    false;

static uint8_t buzzer_mode =
    BUZZER_MODE_1;

static uint8_t buzzer_step =
    0;

static uint32_t buzzer_step_start =
    0;


// ==================================================
// GUI TEXT
// ==================================================

static char voltage_text[32];

static char current_text[32];


// ==================================================
// LED BLINK
// ==================================================

static lv_timer_t *led_blink_timer =
    NULL;

static bool led_blink_state =
    false;

static uint32_t led_blink_color =
    LED_RED;


// ==================================================
// FUNCTION DECLARATIONS
// ==================================================

static void buttons_init(void);

static void buttons_run(void);

static void buttons_task(void);

static void reset_selection_for_screen(
    enum ScreensEnum screen
);

static void update_button_selection(void);


static void uart_task(void);

static void safety_task(void);


static void buzzer_stop(void);

static void buzzer_run(void);

static void buzzer_task(void);


static void led_blink_cb(
    lv_timer_t *timer
);

static void set_status_led(
    uint32_t color
);

static void led_task(void);


static void show_low_voltage_error(void);

static void show_connection_lost_error(void);

static void hide_status_error(void);


static void gui_task(void);

static void watchdog_task(void);


// ==================================================
// BUZZER SET MODE
// ==================================================

void buzzer_set_mode(
    uint8_t mode
)
{
    // ----------------------------------------------
    // Validate mode
    // ----------------------------------------------

    if (
        mode >=
        BUZZER_MODE_COUNT
    )
    {
        mode =
            BUZZER_MODE_1;
    }


    // ----------------------------------------------
    // Save mode
    // ----------------------------------------------

    buzzer_mode =
        mode;


    // ----------------------------------------------
    // Reset buzzer pattern
    // ----------------------------------------------

    buzzer_step =
        0;

    buzzer_step_start =
        millis();


    // ----------------------------------------------
    // Stop current tone
    // ----------------------------------------------

    buzzer_state =
        false;


    noTone(
        BUZZER_PIN
    );


    // ----------------------------------------------
    // Debug
    // ----------------------------------------------

    Serial.print(
        "BUZZER MODE -> "
    );

    Serial.println(
        buzzer_mode + 1
    );
}


// ==================================================
// BUZZER GET MODE
// ==================================================

uint8_t buzzer_get_mode(void)
{
    return buzzer_mode;
}


// ==================================================
// BUZZER STOP
// ==================================================

static void buzzer_stop(void)
{
    buzzer_state =
        false;


    noTone(
        BUZZER_PIN
    );


    buzzer_step =
        0;


    buzzer_step_start =
        millis();
}


// ==================================================
// BUZZER RUN
// ==================================================

static void buzzer_run(void)
{
    uint32_t now =
        millis();


    // =================================================
    // MODE 1
    //
    // 200 ms ON
    // 700 ms OFF
    // =================================================

    if (
        buzzer_mode ==
        BUZZER_MODE_1
    )
    {
        if (
            buzzer_step ==
            0
        )
        {
            if (
                now - buzzer_step_start >=
                BUZZER_MODE1_OFF
            )
            {
                tone(
                    BUZZER_PIN,
                    BUZZER_FREQUENCY
                );


                buzzer_state =
                    true;


                buzzer_step =
                    1;


                buzzer_step_start =
                    now;
            }
        }
        else
        {
            if (
                now - buzzer_step_start >=
                BUZZER_MODE1_ON
            )
            {
                noTone(
                    BUZZER_PIN
                );


                buzzer_state =
                    false;


                buzzer_step =
                    0;


                buzzer_step_start =
                    now;
            }
        }


        return;
    }


    // =================================================
    // MODE 2
    //
    // BEEP
    // PAUSE
    // BEEP
    // LONG PAUSE
    // =================================================

    if (
        buzzer_mode ==
        BUZZER_MODE_2
    )
    {
        switch (
            buzzer_step
        )
        {
            // -----------------------------------------
            // Long pause
            // -----------------------------------------

            case 0:

                if (
                    now - buzzer_step_start >=
                    BUZZER_MODE2_REPEAT
                )
                {
                    tone(
                        BUZZER_PIN,
                        BUZZER_FREQUENCY
                    );


                    buzzer_state =
                        true;


                    buzzer_step =
                        1;


                    buzzer_step_start =
                        now;
                }

                break;


            // -----------------------------------------
            // First beep
            // -----------------------------------------

            case 1:

                if (
                    now - buzzer_step_start >=
                    BUZZER_MODE2_BEEP
                )
                {
                    noTone(
                        BUZZER_PIN
                    );


                    buzzer_state =
                        false;


                    buzzer_step =
                        2;


                    buzzer_step_start =
                        now;
                }

                break;


            // -----------------------------------------
            // Pause between beeps
            // -----------------------------------------

            case 2:

                if (
                    now - buzzer_step_start >=
                    BUZZER_MODE2_PAUSE
                )
                {
                    tone(
                        BUZZER_PIN,
                        BUZZER_FREQUENCY
                    );


                    buzzer_state =
                        true;


                    buzzer_step =
                        3;


                    buzzer_step_start =
                        now;
                }

                break;


            // -----------------------------------------
            // Second beep
            // -----------------------------------------

            case 3:

                if (
                    now - buzzer_step_start >=
                    BUZZER_MODE2_BEEP
                )
                {
                    noTone(
                        BUZZER_PIN
                    );


                    buzzer_state =
                        false;


                    buzzer_step =
                        0;


                    buzzer_step_start =
                        now;
                }

                break;


            default:

                buzzer_step =
                    0;

                buzzer_step_start =
                    now;

                noTone(
                    BUZZER_PIN
                );

                buzzer_state =
                    false;

                break;
        }


        return;
    }


    // =================================================
    // MODE 3
    //
    // 500 ms ON
    // 500 ms OFF
    // =================================================

    if (
        buzzer_mode ==
        BUZZER_MODE_3
    )
    {
        if (
            buzzer_step ==
            0
        )
        {
            if (
                now - buzzer_step_start >=
                BUZZER_MODE3_OFF
            )
            {
                tone(
                    BUZZER_PIN,
                    BUZZER_FREQUENCY
                );


                buzzer_state =
                    true;


                buzzer_step =
                    1;


                buzzer_step_start =
                    now;
            }
        }
        else
        {
            if (
                now - buzzer_step_start >=
                BUZZER_MODE3_ON
            )
            {
                noTone(
                    BUZZER_PIN
                );


                buzzer_state =
                    false;


                buzzer_step =
                    0;


                buzzer_step_start =
                    now;
            }
        }
    }
}


// ==================================================
// BUZZER TASK
// ==================================================

static void buzzer_task(void)
{
    // ----------------------------------------------
    // No data received
    // ----------------------------------------------

    if (
        !system_state.data_received
    )
    {
        buzzer_stop();

        return;
    }


    // ----------------------------------------------
    // Connection lost
    // ----------------------------------------------

    if (
        system_state.connection_lost
    )
    {
        buzzer_stop();

        return;
    }


    // ----------------------------------------------
    // No low voltage
    // ----------------------------------------------

    if (
        !system_state.low_voltage
    )
    {
        buzzer_stop();

        return;
    }


    // ----------------------------------------------
    // Run selected mode
    // ----------------------------------------------

    buzzer_run();
}


// ==================================================
// STATUS LED
// ==================================================

static void set_status_led(
    uint32_t color
)
{
    if (
        objects.obj0 == NULL
    )
    {
        return;
    }


    // =================================================
    // CONSTANT COLOR
    // =================================================

    if (
        color == LED_BLUE ||
        color == LED_GREEN
    )
    {
        if (
            led_blink_timer != NULL
        )
        {
            lv_timer_del(
                led_blink_timer
            );

            led_blink_timer =
                NULL;
        }


        led_blink_state =
            false;


        lv_led_set_color(
            objects.obj0,
            lv_color_hex(color)
        );


        lv_led_on(
            objects.obj0
        );


        return;
    }


    // =================================================
    // BLINK COLOR
    // =================================================

    if (
        color == LED_RED ||
        color == LED_ORANGE
    )
    {
        if (
            led_blink_timer != NULL &&
            led_blink_color == color
        )
        {
            return;
        }


        if (
            led_blink_timer != NULL
        )
        {
            lv_timer_del(
                led_blink_timer
            );

            led_blink_timer =
                NULL;
        }


        led_blink_color =
            color;


        led_blink_state =
            true;


        lv_led_set_color(
            objects.obj0,
            lv_color_hex(color)
        );


        lv_led_on(
            objects.obj0
        );


        led_blink_timer =
            lv_timer_create(
                led_blink_cb,
                500,
                NULL
            );
    }
}


// ==================================================
// LED BLINK CALLBACK
// ==================================================

static void led_blink_cb(
    lv_timer_t *timer
)
{
    (void)timer;


    if (
        objects.obj0 == NULL
    )
    {
        return;
    }


    led_blink_state =
        !led_blink_state;


    if (
        led_blink_state
    )
    {
        lv_led_set_color(
            objects.obj0,
            lv_color_hex(
                led_blink_color
            )
        );


        lv_led_on(
            objects.obj0
        );
    }
    else
    {
        lv_led_off(
            objects.obj0
        );
    }
}


// ==================================================
// LED TASK
// ==================================================

static void led_task(void)
{
    if (
        !system_state.data_received
    )
    {
        set_status_led(
            LED_BLUE
        );

        return;
    }


    if (
        system_state.connection_lost
    )
    {
        set_status_led(
            LED_ORANGE
        );

        return;
    }


    if (
        system_state.system_ok
    )
    {
        set_status_led(
            LED_GREEN
        );

        return;
    }


    set_status_led(
        LED_RED
    );
}


// ==================================================
// UART TASK
// ==================================================

static void uart_task(void)
{
    uart_receive();


    float new_voltage =
        0.0f;


    float new_current =
        0.0f;


    if (
        uart_get_values(
            &new_voltage,
            &new_current
        )
    )
    {
        system_state.voltage =
            new_voltage;


        system_state.current =
            new_current;


        system_state.data_received =
            true;


        system_state.uart_timeout =
            false;


        system_state.connection_lost =
            false;


        last_uart_data =
            millis();


        Serial.print(
            "TASK UART -> V = "
        );


        Serial.print(
            system_state.voltage,
            2
        );


        Serial.print(
            " | I = "
        );


        Serial.println(
            system_state.current,
            2
        );
    }
}


// ==================================================
// SAFETY TASK
// ==================================================

static void safety_task(void)
{
    if (
        !system_state.data_received
    )
    {
        system_state.voltage_ok =
            false;


        system_state.current_ok =
            false;


        system_state.system_ok =
            false;


        system_state.low_voltage =
            false;


        system_state.uart_timeout =
            false;


        system_state.connection_lost =
            false;


        return;
    }


    // =================================================
    // CONNECTION TIMEOUT
    // =================================================

    system_state.connection_lost =
        (
            millis() -
            last_uart_data >=
            UART_TIMEOUT
        );


    system_state.uart_timeout =
        system_state.connection_lost;


    // =================================================
    // VOLTAGE
    // =================================================

    system_state.voltage_ok =
        (
            system_state.voltage >=
            VOLTAGE_MIN
            &&
            system_state.voltage <=
            VOLTAGE_MAX
        );


    // =================================================
    // CURRENT
    // =================================================

    system_state.current_ok =
        (
            system_state.current >=
            CURRENT_MIN
            &&
            system_state.current <=
            CURRENT_MAX
        );


    // =================================================
    // LOW VOLTAGE
    // =================================================

    system_state.low_voltage =
        (
            system_state.voltage <
            VOLTAGE_MIN
        );


    // =================================================
    // SYSTEM OK
    // =================================================

    system_state.system_ok =
        (
            system_state.voltage_ok
            &&
            system_state.current_ok
            &&
            !system_state.connection_lost
        );
}


// ==================================================
// LOW VOLTAGE ERROR
// ==================================================

static void show_low_voltage_error(void)
{
    if (
        objects.error_box == NULL ||
        objects.low_voltage_label == NULL
    )
    {
        return;
    }


    lv_obj_set_style_bg_color(
        objects.error_box,
        lv_color_hex(
            0x8B0000
        ),
        LV_PART_MAIN
    );


    lv_obj_set_style_bg_opa(
        objects.error_box,
        LV_OPA_COVER,
        LV_PART_MAIN
    );


    lv_obj_set_style_border_color(
        objects.error_box,
        lv_color_hex(
            0xFF4444
        ),
        LV_PART_MAIN
    );


    lv_obj_set_style_border_width(
        objects.error_box,
        2,
        LV_PART_MAIN
    );


    lv_label_set_text(
        objects.low_voltage_label,
        "LOW VOLTAGE !"
    );


    lv_obj_set_pos(
        objects.low_voltage_label,
        103,
        121
    );


    lv_obj_set_style_text_color(
        objects.low_voltage_label,
        lv_color_hex(
            0xFFFFFF
        ),
        LV_PART_MAIN
    );


    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );


    lv_obj_clear_flag(
        objects.low_voltage_label,
        LV_OBJ_FLAG_HIDDEN
    );
}


// ==================================================
// CONNECTION LOST ERROR
// ==================================================

static void show_connection_lost_error(void)
{
    if (
        objects.error_box == NULL ||
        objects.low_voltage_label == NULL
    )
    {
        return;
    }


    lv_obj_set_style_bg_color(
        objects.error_box,
        lv_color_hex(
            0xCC6600
        ),
        LV_PART_MAIN
    );


    lv_obj_set_style_bg_opa(
        objects.error_box,
        LV_OPA_COVER,
        LV_PART_MAIN
    );


    lv_obj_set_style_border_color(
        objects.error_box,
        lv_color_hex(
            0xFFAA00
        ),
        LV_PART_MAIN
    );


    lv_obj_set_style_border_width(
        objects.error_box,
        2,
        LV_PART_MAIN
    );


    lv_label_set_text(
        objects.low_voltage_label,
        "CONNECTION LOST"
    );


    lv_obj_set_pos(
        objects.low_voltage_label,
        88,
        126
    );


    lv_obj_set_style_text_color(
        objects.low_voltage_label,
        lv_color_hex(
            0xFFFFFF
        ),
        LV_PART_MAIN
    );


    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );


    lv_obj_clear_flag(
        objects.low_voltage_label,
        LV_OBJ_FLAG_HIDDEN
    );
}


// ==================================================
// HIDE ERROR
// ==================================================

static void hide_status_error(void)
{
    if (
        objects.error_box != NULL
    )
    {
        lv_obj_add_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );
    }


    if (
        objects.low_voltage_label != NULL
    )
    {
        lv_obj_add_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}


// ==================================================
// GUI TASK
// ==================================================

static void gui_task(void)
{
    // =================================================
    // VOLTAGE / CURRENT
    // =================================================

    if (
        system_state.data_received
    )
    {
        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            system_state.voltage
        );


        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            system_state.current
        );


        if (
            objects.voltage != NULL
        )
        {
            lv_label_set_text(
                objects.voltage,
                voltage_text
            );
        }


        if (
            objects.current != NULL
        )
        {
            lv_label_set_text(
                objects.current,
                current_text
            );
        }
    }


    // =================================================
    // ERROR
    // =================================================

    if (
        system_state.connection_lost
    )
    {
        show_connection_lost_error();
    }
    else if (
        system_state.low_voltage
    )
    {
        show_low_voltage_error();
    }
    else
    {
        hide_status_error();
    }


    // =================================================
    // LVGL
    // =================================================

    uint32_t now =
        millis();


    if (
        now - last_lvgl >=
        5
    )
    {
        last_lvgl =
            now;


        lv_timer_handler();


        ui_tick();
    }
}


// ==================================================
// RESET SELECTION FOR SCREEN
// ==================================================

static void reset_selection_for_screen(
    enum ScreensEnum screen
)
{
    switch (
        screen
    )
    {
        // ---------------------------------------------
        // MAIN
        // ---------------------------------------------

        case SCREEN_ID_MAIN:

            selected_option =
                MAIN_OPTION_SETTINGS;

            break;


        // ---------------------------------------------
        // SETTINGS
        // ---------------------------------------------

        case SCREEN_ID_SETTINGS:

            selected_option =
                SETTINGS_OPTION_BUZZER;

            break;


        // ---------------------------------------------
        // BUZZER
        // ---------------------------------------------

        case SCREEN_ID_BUZZER:

            selected_option =
                0;


            if (
                objects.buzzer_options != NULL
            )
            {
                lv_dropdown_set_selected(
                    objects.buzzer_options,
                    buzzer_get_mode()
                );


                lv_obj_invalidate(
                    objects.buzzer_options
                );
            }

            break;


        // ---------------------------------------------
        // DEFAULT
        // ---------------------------------------------

        default:

            selected_option =
                0;

            break;
    }
}


// ==================================================
// UPDATE BUTTON SELECTION
// ==================================================

static void update_button_selection(void)
{
    enum ScreensEnum screen =
        screen_manager_get();


    // =================================================
    // MAIN
    // =================================================

    if (
        screen ==
        SCREEN_ID_MAIN
    )
    {
        if (
            objects.btn_settings == NULL
        )
        {
            Serial.println(
                "ERROR: SETTINGS BUTTON NULL"
            );

            return;
        }


        // ---------------------------------------------
        // Reset
        // ---------------------------------------------

        lv_obj_set_style_border_width(
            objects.btn_settings,
            0,
            LV_PART_MAIN
        );


        // ---------------------------------------------
        // Selected
        // ---------------------------------------------

        lv_obj_set_style_border_color(
            objects.btn_settings,
            lv_color_hex(
                0x00FF00
            ),
            LV_PART_MAIN
        );


        lv_obj_set_style_border_width(
            objects.btn_settings,
            4,
            LV_PART_MAIN
        );


        lv_obj_set_style_border_opa(
            objects.btn_settings,
            LV_OPA_COVER,
            LV_PART_MAIN
        );


        lv_obj_invalidate(
            objects.btn_settings
        );


        Serial.println(
            "LCD SELECTED: SETTINGS"
        );


        return;
    }


    // =================================================
    // SETTINGS
    // =================================================

    if (
        screen ==
        SCREEN_ID_SETTINGS
    )
    {
        if (
            objects.buzzer == NULL ||
            objects.touch_calibration == NULL ||
            objects.voltage_range == NULL ||
            objects.exit_settings == NULL
        )
        {
            Serial.println(
                "ERROR: SETTINGS BUTTON OBJECT NULL"
            );

            return;
        }


        // ---------------------------------------------
        // Remove all borders
        // ---------------------------------------------

        lv_obj_set_style_border_width(
            objects.buzzer,
            0,
            LV_PART_MAIN
        );


        lv_obj_set_style_border_width(
            objects.touch_calibration,
            0,
            LV_PART_MAIN
        );


        lv_obj_set_style_border_width(
            objects.voltage_range,
            0,
            LV_PART_MAIN
        );


        lv_obj_set_style_border_width(
            objects.exit_settings,
            0,
            LV_PART_MAIN
        );


        lv_obj_t *selected_btn =
            NULL;


        // ---------------------------------------------
        // BUZZER
        // ---------------------------------------------

        if (
            selected_option ==
            SETTINGS_OPTION_BUZZER
        )
        {
            selected_btn =
                objects.buzzer;


            Serial.println(
                "LCD SELECTED: BUZZER"
            );
        }


        // ---------------------------------------------
        // CALIBRATION
        // ---------------------------------------------

        else if (
            selected_option ==
            SETTINGS_OPTION_CALIBRATION
        )
        {
            selected_btn =
                objects.touch_calibration;


            Serial.println(
                "LCD SELECTED: CALIBRATION"
            );
        }


        // ---------------------------------------------
        // V/C RANGE
        // ---------------------------------------------

        else if (
            selected_option ==
            SETTINGS_OPTION_VC_RANGE
        )
        {
            selected_btn =
                objects.voltage_range;


            Serial.println(
                "LCD SELECTED: V/C RANGE"
            );
        }


        // ---------------------------------------------
        // EXIT
        // ---------------------------------------------

        else if (
            selected_option ==
            SETTINGS_OPTION_BACK
        )
        {
            selected_btn =
                objects.exit_settings;


            Serial.println(
                "LCD SELECTED: EXIT"
            );
        }


        // ---------------------------------------------
        // Green border
        // ---------------------------------------------

        if (
            selected_btn != NULL
        )
        {
            lv_obj_set_style_border_color(
                selected_btn,
                lv_color_hex(
                    0x00FF00
                ),
                LV_PART_MAIN
            );


            lv_obj_set_style_border_width(
                selected_btn,
                4,
                LV_PART_MAIN
            );


            lv_obj_set_style_border_opa(
                selected_btn,
                LV_OPA_COVER,
                LV_PART_MAIN
            );


            lv_obj_invalidate(
                selected_btn
            );
        }


        return;
    }


    // =================================================
    // BUZZER SCREEN
    // =================================================

    if (
        screen ==
        SCREEN_ID_BUZZER
    )
    {
        if (
            objects.buzzer_options != NULL
        )
        {
            Serial.print(
                "BUZZER OPTION: "
            );


            Serial.println(
                buzzer_get_mode() + 1
            );
        }


        return;
    }
}


// ==================================================
// BUTTON INITIALIZATION
// ==================================================

static void buttons_init(void)
{
    pinMode(
        BTN_RIGHT,
        INPUT_PULLUP
    );


    pinMode(
        BTN_SELECT,
        INPUT_PULLUP
    );


    right_raw_state =
        digitalRead(
            BTN_RIGHT
        );


    select_raw_state =
        digitalRead(
            BTN_SELECT
        );


    right_stable_state =
        right_raw_state;


    select_stable_state =
        select_raw_state;


    right_last_change =
        millis();


    select_last_change =
        millis();


    button_last_screen =
        SCREEN_ID_MAIN;


    selected_option =
        MAIN_OPTION_SETTINGS;


    update_button_selection();
}


// ==================================================
// BUTTON RUN
// ==================================================

static void buttons_run(void)
{
    uint32_t now =
        millis();


    // =================================================
    // CURRENT SCREEN
    // =================================================

    enum ScreensEnum current_screen =
        screen_manager_get();


    // =================================================
    // SCREEN CHANGE
    // =================================================

    if (
        current_screen !=
        button_last_screen
    )
    {
        Serial.print(
            "BUTTON SCREEN CHANGE -> "
        );


        reset_selection_for_screen(
            current_screen
        );


        button_last_screen =
            current_screen;


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


        update_button_selection();
    }


    // =================================================
    // READ BUTTONS
    // =================================================

    bool right_read =
        digitalRead(
            BTN_RIGHT
        );


    bool select_read =
        digitalRead(
            BTN_SELECT
        );


    // =================================================
    // RIGHT RAW CHANGE
    // =================================================

    if (
        right_read !=
        right_raw_state
    )
    {
        right_raw_state =
            right_read;


        right_last_change =
            now;
    }


    // =================================================
    // RIGHT DEBOUNCE
    // =================================================

    if (
        now - right_last_change >=
        BUTTON_DEBOUNCE
    )
    {
        if (
            right_stable_state !=
            right_raw_state
        )
        {
            right_stable_state =
                right_raw_state;


            if (
                right_stable_state ==
                LOW
            )
            {
                Serial.println(
                    "BUTTON: RIGHT"
                );


                // =========================================
                // MAIN
                // =========================================

                if (
                    screen_manager_is(
                        SCREEN_ID_MAIN
                    )
                )
                {
                    selected_option =
                        MAIN_OPTION_SETTINGS;


                    update_button_selection();
                }


                // =========================================
                // SETTINGS
                // =========================================

                else if (
                    screen_manager_is(
                        SCREEN_ID_SETTINGS
                    )
                )
                {
                    selected_option++;


                    if (
                        selected_option >
                        SETTINGS_OPTION_BACK
                    )
                    {
                        selected_option =
                            SETTINGS_OPTION_BUZZER;
                    }


                    update_button_selection();
                }


                // =========================================
                // BUZZER
                // =========================================

                else if (
                    screen_manager_is(
                        SCREEN_ID_BUZZER
                    )
                )
                {
                    if (
                        objects.buzzer_options != NULL
                    )
                    {
                        uint16_t count =
                            lv_dropdown_get_option_cnt(
                                objects.buzzer_options
                            );


                        if (
                            count > 0
                        )
                        {
                            uint16_t current =
                                lv_dropdown_get_selected(
                                    objects.buzzer_options
                                );


                            // ---------------------------------
                            // Next option
                            // ---------------------------------

                            current++;


                            if (
                                current >= count
                            )
                            {
                                current =
                                    0;
                            }


                            // ---------------------------------
                            // Update buzzer mode
                            // ---------------------------------

                            buzzer_set_mode(
                                (uint8_t)current
                            );


                            // ---------------------------------
                            // Update dropdown
                            // ---------------------------------

                            lv_dropdown_set_selected(
                                objects.buzzer_options,
                                current
                            );


                            // ---------------------------------
                            // Force redraw
                            // ---------------------------------

                            lv_obj_invalidate(
                                objects.buzzer_options
                            );


                            Serial.print(
                                "D1 BUZZER MODE -> "
                            );


                            Serial.println(
                                current + 1
                            );
                        }
                    }
                }
            }
        }
    }


    // =================================================
    // SELECT RAW CHANGE
    // =================================================

    if (
        select_read !=
        select_raw_state
    )
    {
        select_raw_state =
            select_read;


        select_last_change =
            now;
    }


    // =================================================
    // SELECT DEBOUNCE
    // =================================================

    if (
        now - select_last_change >=
        BUTTON_DEBOUNCE
    )
    {
        if (
            select_stable_state !=
            select_raw_state
        )
        {
            select_stable_state =
                select_raw_state;


            if (
                select_stable_state ==
                LOW
            )
            {
                Serial.println(
                    "BUTTON: SELECT"
                );


                // =========================================
                // MAIN
                // =========================================

                if (
                    screen_manager_is(
                        SCREEN_ID_MAIN
                    )
                )
                {
                    Serial.println(
                        "ACTION: SETTINGS"
                    );


                    action_go_to_settings_page(
                        NULL
                    );
                }


                // =========================================
                // SETTINGS
                // =========================================

                else if (
                    screen_manager_is(
                        SCREEN_ID_SETTINGS
                    )
                )
                {
                    switch (
                        selected_option
                    )
                    {
                        // ---------------------------------
                        // BUZZER
                        // ---------------------------------

                        case SETTINGS_OPTION_BUZZER:

                            Serial.println(
                                "ACTION: BUZZER"
                            );


                            action_go_to_buzzer_settings(
                                NULL
                            );

                            break;


                        // ---------------------------------
                        // CALIBRATION
                        // ---------------------------------

                        case SETTINGS_OPTION_CALIBRATION:

                            Serial.println(
                                "ACTION: CALIBRATION"
                            );


                            action_go_to_touch_calibration(
                                NULL
                            );

                            break;


                        // ---------------------------------
                        // V/C RANGE
                        // ---------------------------------

                        case SETTINGS_OPTION_VC_RANGE:

                            Serial.println(
                                "ACTION: V/C RANGE"
                            );

                            break;


                        // ---------------------------------
                        // EXIT
                        // ---------------------------------

                        case SETTINGS_OPTION_BACK:

                            Serial.println(
                                "ACTION: MAIN"
                            );


                            action_exit_to_main_page(
                                NULL
                            );

                            break;


                        default:

                            break;
                    }
                }


                // =========================================
                // BUZZER
                // =========================================

                else if (
                    screen_manager_is(
                        SCREEN_ID_BUZZER
                    )
                )
                {
                    Serial.println(
                        "ACTION: SETTINGS"
                    );


                    action_go_from_buzzer_settings_page_to_settings_page(
                        NULL
                    );
                }
            }
        }
    }
}


// ==================================================
// BUTTON TASK
// ==================================================

static void buttons_task(void)
{
    buttons_run();
}


// ==================================================
// WATCHDOG
// ==================================================

static void watchdog_task(void)
{
    ESP.wdtFeed();
}


// ==================================================
// TASK INITIALIZATION
// ==================================================

void tasks_init(void)
{
    // =================================================
    // TIMING
    // =================================================

    last_uart_data =
        millis();


    last_lvgl =
        millis();


    // =================================================
    // BUZZER
    // =================================================

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );


    buzzer_stop();


    // =================================================
    // BUTTONS
    // =================================================

    buttons_init();


    // =================================================
    // LED
    // =================================================

    set_status_led(
        LED_BLUE
    );


    // =================================================
    // ERROR
    // =================================================

    hide_status_error();


    // =================================================
    // SERIAL
    // =================================================

    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        "TASK SYSTEM INITIALIZED"
    );

    Serial.println(
        "RIGHT  = D1"
    );

    Serial.println(
        "SELECT = D4"
    );

    Serial.println(
        "BUZZER = D0"
    );

    Serial.println(
        "BUZZER MODE 1 = SINGLE BEEP"
    );

    Serial.println(
        "BUZZER MODE 2 = DOUBLE BEEP"
    );

    Serial.println(
        "BUZZER MODE 3 = LONG BEEP"
    );

    Serial.println(
        "WATCHDOG = ENABLED"
    );

    Serial.println(
        "=============================="
    );
}


// ==================================================
// MAIN TASK RUNNER
// ==================================================

void tasks_run(void)
{
    // =================================================
    // 1. BUTTON
    // =================================================

    buttons_task();


    // =================================================
    // 2. UART
    // =================================================

    uart_task();


    // =================================================
    // 3. SAFETY
    // =================================================

    safety_task();


    // =================================================
    // 4. BUZZER
    // =================================================

    buzzer_task();


    // =================================================
    // 5. LED
    // =================================================

    led_task();


    // =================================================
    // 6. GUI
    // =================================================

    gui_task();


    // =================================================
    // 7. WATCHDOG
    // =================================================

    watchdog_task();
}