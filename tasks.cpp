
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

#define BUZZER_MODE1_ON      200
#define BUZZER_MODE1_OFF     700

#define BUZZER_MODE2_BEEP    100
#define BUZZER_MODE2_PAUSE   100
#define BUZZER_MODE2_REPEAT  900

#define BUZZER_MODE3_ON      500
#define BUZZER_MODE3_OFF     500


// ==================================================
// BUTTON
// ==================================================

#define BUTTON_DEBOUNCE 50UL


// ==================================================
// MAIN OPTION
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

static uint8_t selected_option = MAIN_OPTION_SETTINGS;

static bool right_raw_state  = HIGH;
static bool select_raw_state = HIGH;

static bool right_stable_state  = HIGH;
static bool select_stable_state = HIGH;

static uint32_t right_last_change  = 0;
static uint32_t select_last_change = 0;

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
// LIMITS
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

static uint32_t last_uart_data = 0;
static uint32_t last_lvgl      = 0;


// ==================================================
// BUZZER STATE
// ==================================================

static bool buzzer_state = false;

static uint8_t buzzer_mode = BUZZER_MODE_1;

static uint8_t buzzer_step = 0;

static uint32_t buzzer_step_start = 0;


// ==================================================
// TEXT
// ==================================================

static char voltage_text[32];
static char current_text[32];


// ==================================================
// LED TEST STATE
// ==================================================

static bool led_blink_state = false;
static uint32_t led_blink_color = LED_RED;


// ==================================================
// FUNCTION DECLARATIONS
// ==================================================

static void buttons_init(void);
static void buttons_run(void);

static void reset_selection_for_screen(void);
static void update_button_selection(void);

static void set_status_led(uint32_t color);

static void show_low_voltage_error(void);
static void show_connection_lost_error(void);
static void hide_status_error(void);

static void buzzer_run(void);


// ==================================================
// BUZZER SET MODE
// ==================================================

void buzzer_set_mode(uint8_t mode)
{
    if (mode >= BUZZER_MODE_COUNT)
        return;

    buzzer_mode = mode;

    buzzer_state = false;
    buzzer_step = 0;
    buzzer_step_start = millis();

    noTone(BUZZER_PIN);

    Serial.print("BUZZER MODE = ");

    switch (buzzer_mode)
    {
        case BUZZER_MODE_1:
            Serial.println("MODE 1");
            break;

        case BUZZER_MODE_2:
            Serial.println("MODE 2");
            break;

        case BUZZER_MODE_3:
            Serial.println("MODE 3");
            break;

        default:
            Serial.println("UNKNOWN");
            break;
    }
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

void buzzer_stop(void)
{
    noTone(BUZZER_PIN);

    buzzer_state = false;

    buzzer_step = 0;

    buzzer_step_start = millis();
}


// ==================================================
// BUZZER RUN
// ==================================================

static void buzzer_run(void)
{
    uint32_t now = millis();

    switch (buzzer_mode)
    {
        case BUZZER_MODE_1:

            if (!buzzer_state)
            {
                tone(
                    BUZZER_PIN,
                    BUZZER_FREQUENCY
                );

                buzzer_state = true;

                buzzer_step_start = now;
            }
            else
            {
                if ((now - buzzer_step_start) >=
                    BUZZER_MODE1_ON)
                {
                    noTone(BUZZER_PIN);

                    buzzer_state = false;

                    buzzer_step_start = now;
                }
            }

            break;


        case BUZZER_MODE_2:

            switch (buzzer_step)
            {
                case 0:

                    tone(
                        BUZZER_PIN,
                        BUZZER_FREQUENCY
                    );

                    buzzer_step = 1;
                    buzzer_step_start = now;

                    break;


                case 1:

                    if ((now - buzzer_step_start) >=
                        BUZZER_MODE2_BEEP)
                    {
                        noTone(BUZZER_PIN);

                        buzzer_step = 2;
                        buzzer_step_start = now;
                    }

                    break;


                case 2:

                    if ((now - buzzer_step_start) >=
                        BUZZER_MODE2_PAUSE)
                    {
                        tone(
                            BUZZER_PIN,
                            BUZZER_FREQUENCY
                        );

                        buzzer_step = 3;
                        buzzer_step_start = now;
                    }

                    break;


                case 3:

                    if ((now - buzzer_step_start) >=
                        BUZZER_MODE2_BEEP)
                    {
                        noTone(BUZZER_PIN);

                        buzzer_step = 4;
                        buzzer_step_start = now;
                    }

                    break;


                case 4:

                    if ((now - buzzer_step_start) >=
                        BUZZER_MODE2_REPEAT)
                    {
                        buzzer_step = 0;
                    }

                    break;


                default:

                    buzzer_step = 0;
                    noTone(BUZZER_PIN);

                    break;
            }

            break;


        case BUZZER_MODE_3:

            if (!buzzer_state)
            {
                tone(
                    BUZZER_PIN,
                    BUZZER_FREQUENCY
                );

                buzzer_state = true;

                buzzer_step_start = now;
            }
            else
            {
                if ((now - buzzer_step_start) >=
                    BUZZER_MODE3_ON)
                {
                    noTone(BUZZER_PIN);

                    buzzer_state = false;

                    buzzer_step_start = now;
                }
            }

            break;


        default:

            buzzer_stop();

            break;
    }
}


// ==================================================
// LED STATUS
//
// LED is intentionally disabled for this test.
// ==================================================

static void set_status_led(uint32_t color)
{
    led_blink_color = color;
    led_blink_state = false;

    // IMPORTANT:
    // No LVGL LED function is called here.
    return;
}


// ==================================================
// LOW VOLTAGE ERROR
// ==================================================

static void show_low_voltage_error(void)
{
    if (objects.error_box == NULL)
        return;

    if (objects.low_voltage_label == NULL)
        return;

    lv_label_set_text(
        objects.low_voltage_label,
        "LOW VOLTAGE"
    );

    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );
}


// ==================================================
// CONNECTION LOST
// ==================================================

static void show_connection_lost_error(void)
{
    if (objects.error_box == NULL)
        return;

    if (objects.low_voltage_label == NULL)
        return;

    lv_label_set_text(
        objects.low_voltage_label,
        "CONNECTION LOST"
    );

    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );
}


// ==================================================
// HIDE ERROR
// ==================================================

static void hide_status_error(void)
{
    if (objects.error_box == NULL)
        return;

    lv_obj_add_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );
}


// ==================================================
// BUTTON SELECTION
// ==================================================

static void update_button_selection(void)
{
    // ==================================================
    // MAIN
    // ==================================================

    if (screen_manager_is(SCREEN_ID_MAIN))
    {
        if (objects.btn_settings == NULL)
            return;

        lv_obj_set_style_border_width(
            objects.btn_settings,
            3,
            LV_PART_MAIN
        );

        lv_obj_set_style_border_color(
            objects.btn_settings,
            lv_color_hex(0x00FF00),
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

        return;
    }


    // ==================================================
    // SETTINGS
    // ==================================================

    if (screen_manager_is(SCREEN_ID_SETTINGS))
    {
        if (objects.exit_settings == NULL ||
            objects.buzzer == NULL ||
            objects.touch_calibration == NULL ||
            objects.voltage_range == NULL)
        {
            return;
        }


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


        lv_obj_t *selected_button = NULL;


        switch (selected_option)
        {
            case SETTINGS_OPTION_BUZZER:

                selected_button =
                    objects.buzzer;

                break;


            case SETTINGS_OPTION_CALIBRATION:

                selected_button =
                    objects.touch_calibration;

                break;


            case SETTINGS_OPTION_VC_RANGE:

                selected_button =
                    objects.voltage_range;

                break;


            case SETTINGS_OPTION_BACK:

                selected_button =
                    objects.exit_settings;

                break;


            default:

                selected_button =
                    objects.buzzer;

                break;
        }


        if (selected_button != NULL)
        {
            lv_obj_set_style_border_color(
                selected_button,
                lv_color_hex(0x00FF00),
                LV_PART_MAIN
            );

            lv_obj_set_style_border_width(
                selected_button,
                4,
                LV_PART_MAIN
            );

            lv_obj_set_style_border_opa(
                selected_button,
                LV_OPA_COVER,
                LV_PART_MAIN
            );

            lv_obj_invalidate(
                selected_button
            );
        }

        return;
    }


    if (screen_manager_is(SCREEN_ID_BUZZER))
        return;


    if (screen_manager_is(SCREEN_ID_V_C_RANGE))
        return;
}


// ==================================================
// RESET SELECTION
// ==================================================

static void reset_selection_for_screen(void)
{
    if (screen_manager_is(SCREEN_ID_MAIN))
    {
        selected_option =
            MAIN_OPTION_SETTINGS;

        return;
    }


    if (screen_manager_is(SCREEN_ID_SETTINGS))
    {
        selected_option =
            SETTINGS_OPTION_BUZZER;

        return;
    }


    if (screen_manager_is(SCREEN_ID_BUZZER))
    {
        if (objects.buzzer_options != NULL)
        {
            lv_dropdown_set_selected(
                objects.buzzer_options,
                buzzer_mode
            );

            lv_obj_invalidate(
                objects.buzzer_options
            );
        }

        return;
    }
}


// ==================================================
// BUTTON INIT
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
        digitalRead(BTN_RIGHT);

    select_raw_state =
        digitalRead(BTN_SELECT);


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
    uint32_t now = millis();


    enum ScreensEnum current_screen =
        screen_manager_get();


    if (current_screen != button_last_screen)
    {
        reset_selection_for_screen();

        button_last_screen =
            current_screen;

        update_button_selection();
    }


    bool right_read =
        digitalRead(BTN_RIGHT);

    bool select_read =
        digitalRead(BTN_SELECT);


    // ==================================================
    // RIGHT
    // ==================================================

    if (right_read != right_raw_state)
    {
        right_raw_state =
            right_read;

        right_last_change =
            now;
    }


    if ((now - right_last_change) >=
        BUTTON_DEBOUNCE)
    {
        if (right_stable_state !=
            right_raw_state)
        {
            right_stable_state =
                right_raw_state;


            if (right_stable_state == LOW)
            {
                Serial.println(
                    "BUTTON: RIGHT"
                );


                if (screen_manager_is(
                        SCREEN_ID_MAIN))
                {
                    selected_option =
                        MAIN_OPTION_SETTINGS;

                    update_button_selection();

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_SETTINGS))
                {
                    selected_option++;

                    if (selected_option >
                        SETTINGS_OPTION_BACK)
                    {
                        selected_option =
                            SETTINGS_OPTION_BUZZER;
                    }

                    update_button_selection();

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_BUZZER))
                {
                    buzzer_mode++;

                    if (buzzer_mode >=
                        BUZZER_MODE_COUNT)
                    {
                        buzzer_mode =
                            BUZZER_MODE_1;
                    }


                    buzzer_set_mode(
                        buzzer_mode
                    );


                    if (objects.buzzer_options != NULL)
                    {
                        lv_dropdown_set_selected(
                            objects.buzzer_options,
                            buzzer_mode
                        );

                        lv_obj_invalidate(
                            objects.buzzer_options
                        );
                    }

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_V_C_RANGE))
                {
                    return;
                }
            }
        }
    }


    // ==================================================
    // SELECT
    // ==================================================

    if (select_read != select_raw_state)
    {
        select_raw_state =
            select_read;

        select_last_change =
            now;
    }


    if ((now - select_last_change) >=
        BUTTON_DEBOUNCE)
    {
        if (select_stable_state !=
            select_raw_state)
        {
            select_stable_state =
                select_raw_state;


            if (select_stable_state == LOW)
            {
                Serial.println(
                    "BUTTON: SELECT"
                );


                if (screen_manager_is(
                        SCREEN_ID_MAIN))
                {
                    Serial.println(
                        "ACTION: SETTINGS"
                    );

                    action_go_to_settings_page(
                        NULL
                    );

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_SETTINGS))
                {
                    switch (selected_option)
                    {
                        case SETTINGS_OPTION_BUZZER:

                            Serial.println(
                                "ACTION: BUZZER"
                            );

                            action_go_to_buzzer_settings(
                                NULL
                            );

                            break;


                        case SETTINGS_OPTION_CALIBRATION:

                            Serial.println(
                                "ACTION: CALIBRATION"
                            );

                            action_go_to_touch_calibration(
                                NULL
                            );

                            break;


                        case SETTINGS_OPTION_VC_RANGE:

                            Serial.println(
                                "ACTION: V/C RANGE"
                            );

                            action_go_to_v_c_range_settings(
                                NULL
                            );

                            break;


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

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_BUZZER))
                {
                    Serial.println(
                        "ACTION: BUZZER -> SETTINGS"
                    );

                    action_go_from_buzzer_settings_page_to_settings_page(
                        NULL
                    );

                    return;
                }


                if (screen_manager_is(
                        SCREEN_ID_V_C_RANGE))
                {
                    Serial.println(
                        "ACTION: V/C RANGE -> SETTINGS"
                    );

                    action_go_from_v_c_range_settings_page_to_settings_page(
                        NULL
                    );

                    return;
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
// UART TASK
// ==================================================

static void uart_task(void)
{
    uart_receive();


    float new_voltage = 0.0f;
    float new_current = 0.0f;


    if (uart_get_values(
            &new_voltage,
            &new_current))
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


        if (objects.voltage != NULL)
        {
            lv_label_set_text(
                objects.voltage,
                voltage_text
            );
        }


        if (objects.current != NULL)
        {
            lv_label_set_text(
                objects.current,
                current_text
            );
        }


        Serial.print(
            "V = "
        );

        Serial.print(
            system_state.voltage,
            2
        );

        Serial.print(
            " | I = "
        );

        Serial.print(
            system_state.current,
            2
        );

        Serial.println();
    }
}


// ==================================================
// SAFETY TASK
// ==================================================

static void safety_task(void)
{
    uint32_t now =
        millis();


    if (system_state.data_received)
    {
        if ((now - last_uart_data) >=
            UART_TIMEOUT)
        {
            system_state.uart_timeout =
                true;

            system_state.connection_lost =
                true;
        }
        else
        {
            system_state.uart_timeout =
                false;

            system_state.connection_lost =
                false;
        }
    }


    system_state.voltage_ok =
        (
            system_state.voltage >= VOLTAGE_MIN &&
            system_state.voltage <= VOLTAGE_MAX
        );


    system_state.current_ok =
        (
            system_state.current >= CURRENT_MIN &&
            system_state.current <= CURRENT_MAX
        );


    system_state.low_voltage =
        (
            system_state.voltage <
            VOLTAGE_MIN
        );


    system_state.system_ok =
        (
            system_state.voltage_ok &&
            system_state.current_ok &&
            !system_state.connection_lost
        );


    if (system_state.connection_lost)
    {
        show_connection_lost_error();

        buzzer_stop();
    }
    else if (system_state.low_voltage)
    {
        show_low_voltage_error();
    }
    else
    {
        hide_status_error();

        buzzer_stop();
    }
}


// ==================================================
// BUZZER TASK
// ==================================================

static void buzzer_task(void)
{
    if (!system_state.data_received)
    {
        buzzer_stop();
        return;
    }


    if (system_state.connection_lost)
    {
        buzzer_stop();
        return;
    }


    if (!system_state.low_voltage)
    {
        buzzer_stop();
        return;
    }


    buzzer_run();
}


// ==================================================
// LED TASK
// ==================================================

static void led_task(void)
{
    if (!system_state.data_received)
    {
        set_status_led(
            LED_BLUE
        );

        return;
    }


    if (system_state.connection_lost)
    {
        set_status_led(
            LED_ORANGE
        );

        return;
    }


    if (!system_state.system_ok)
    {
        set_status_led(
            LED_RED
        );

        return;
    }


    set_status_led(
        LED_GREEN
    );
}


// ==================================================
// GUI TASK
// ==================================================

static void gui_task(void)
{
    uint32_t now =
        millis();


    if ((now - last_lvgl) < 5)
        return;


    last_lvgl =
        now;


    lv_timer_handler();

    ui_tick();
}


// ==================================================
// WATCHDOG
// ==================================================

static void watchdog_task(void)
{
    ESP.wdtFeed();
}


// ==================================================
// TASK INIT
// ==================================================

void tasks_init(void)
{
    last_lvgl =
        millis();

    last_uart_data =
        millis();


    system_state.voltage =
        0.0f;

    system_state.current =
        0.0f;

    system_state.data_received =
        false;

    system_state.uart_timeout =
        false;

    system_state.connection_lost =
        false;

    system_state.voltage_ok =
        false;

    system_state.current_ok =
        false;

    system_state.system_ok =
        false;

    system_state.low_voltage =
        false;


    // ==================================================
    // BUZZER
    // ==================================================

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    buzzer_stop();


    // ==================================================
    // BUTTONS
    // ==================================================

    buttons_init();


    // ==================================================
    // LED
    // ==================================================

    set_status_led(
        LED_BLUE
    );


    // ==================================================
    // IMPORTANT DIAGNOSTIC STEP
    //
    // LED object is completely hidden.
    //
    // This prevents LVGL from trying to render
    // the lv_led object and entering lv_led_event().
    // ==================================================

    if (objects.obj0 != NULL)
    {
        lv_obj_add_flag(
            objects.obj0,
            LV_OBJ_FLAG_HIDDEN
        );

        Serial.println(
            "LED OBJECT = HIDDEN FOR TEST"
        );
    }
    else
    {
        Serial.println(
            "LED OBJECT = NULL"
        );
    }


    // ==================================================
    // HIDE ERROR
    // ==================================================

    hide_status_error();


    // ==================================================
    // INFO
    // ==================================================

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
        "LED RENDER = DISABLED"
    );

    Serial.println(
        "=============================="
    );
}


// ==================================================
// TASK RUNNER
// ==================================================

void tasks_run(void)
{
    // ------------------------------------------
    // Apply pending screen change
    // ------------------------------------------

    screen_manager_process();


    // ------------------------------------------
    // Buttons
    // ------------------------------------------

    buttons_task();


    // ------------------------------------------
    // UART
    // ------------------------------------------

    uart_task();


    // ------------------------------------------
    // Safety
    // ------------------------------------------

    safety_task();


    // ------------------------------------------
    // Buzzer
    // ------------------------------------------

    buzzer_task();


    // ------------------------------------------
    // LED
    // ------------------------------------------

    led_task();


    // ------------------------------------------
    // GUI
    // ------------------------------------------

    gui_task();


    // ------------------------------------------
    // Watchdog
    // ------------------------------------------

    watchdog_task();
}

