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


// ==================================================
// PHYSICAL BUTTONS
// ==================================================

#define BTN_RIGHT   D1
#define BTN_SELECT  D4

// Buzzer module - Low Level Trigger
#define BUZZER_PIN  D0

#define BUTTON_DEBOUNCE 50UL


// ==================================================
// OPTIONS
// ==================================================

#define OPTION_CALIBRATE 0
#define OPTION_NEXT_PAGE 1

static uint8_t selected_option = OPTION_CALIBRATE;


// ==================================================
// BUTTON STATES
// ==================================================

static bool right_raw_state  = HIGH;
static bool select_raw_state = HIGH;

static bool right_stable_state  = HIGH;
static bool select_stable_state = HIGH;

static uint32_t right_last_change  = 0;
static uint32_t select_last_change = 0;


// ==================================================
// LED COLORS
// ==================================================

#define LED_BLUE    0x0000FF
#define LED_GREEN   0x00FF00
#define LED_RED     0xFF0000
#define LED_ORANGE  0xFFA500


// ==================================================
// LED BLINK
// ==================================================

static lv_timer_t *led_blink_timer = NULL;
static bool led_blink_state = false;
static uint32_t led_blink_color = LED_RED;


// ==================================================
// TASK TIMING
// ==================================================

static uint32_t last_lvgl = 0;


// ==================================================
// UART VALUES
// ==================================================

static float voltage = 0.0f;
static float current = 0.0f;

static bool data_received = false;
static bool uart_timeout = false;

static uint32_t last_uart_data = 0;

static char voltage_text[32];
static char current_text[32];


// ==================================================
// LIMITS
// ==================================================

#define VOLTAGE_MIN 20.0f
#define VOLTAGE_MAX 25.0f

#define CURRENT_MIN 0.0f
#define CURRENT_MAX 1.0f

#define UART_TIMEOUT 3000UL


// ==================================================
// FUNCTION DECLARATIONS
// ==================================================

static void led_blink_cb(lv_timer_t *timer);

static void buttons_init(void);
static void buttons_run(void);

static void update_button_selection(void);

static void set_status_led(uint32_t color);

static void show_low_voltage_error(void);
static void show_connection_lost_error(void);
static void hide_status_error(void);

static void buzzer_on(void);
static void buzzer_off(void);


// ==================================================
// BUZZER
// LOW LEVEL TRIGGER
// ==================================================

static void buzzer_on(void)
{
    // Low-Level Trigger:
    // LOW = ON
    digitalWrite(BUZZER_PIN, LOW);
}


static void buzzer_off(void)
{
    // Low-Level Trigger:
    // HIGH = OFF
    digitalWrite(BUZZER_PIN, HIGH);
}


// ==================================================
// STATUS ERROR: LOW VOLTAGE
// ==================================================

static void show_low_voltage_error(void)
{
    if (objects.error_box == NULL ||
        objects.low_voltage_label == NULL)
    {
        return;
    }


    // Error box
    lv_obj_set_style_bg_color(
        objects.error_box,
        lv_color_hex(0x8B0000),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        objects.error_box,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_color(
        objects.error_box,
        lv_color_hex(0xFF4444),
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        objects.error_box,
        2,
        LV_PART_MAIN
    );


    // Error text
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
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN
    );


    // Show
    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );

    lv_obj_clear_flag(
        objects.low_voltage_label,
        LV_OBJ_FLAG_HIDDEN
    );


    lv_obj_invalidate(objects.error_box);
    lv_obj_invalidate(objects.low_voltage_label);
}


// ==================================================
// STATUS ERROR: CONNECTION LOST
// ==================================================

static void show_connection_lost_error(void)
{
    if (objects.error_box == NULL ||
        objects.low_voltage_label == NULL)
    {
        return;
    }


    lv_obj_set_style_bg_color(
        objects.error_box,
        lv_color_hex(0xCC6600),
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        objects.error_box,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_color(
        objects.error_box,
        lv_color_hex(0xFFAA00),
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
        lv_color_hex(0xFFFFFF),
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


    lv_obj_invalidate(objects.error_box);
    lv_obj_invalidate(objects.low_voltage_label);
}


// ==================================================
// HIDE STATUS ERROR
// ==================================================

static void hide_status_error(void)
{
    if (objects.error_box != NULL)
    {
        lv_obj_add_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );
    }


    if (objects.low_voltage_label != NULL)
    {
        lv_obj_add_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}


// ==================================================
// STATUS LED
// ==================================================

static void set_status_led(uint32_t color)
{
    if (objects.obj0 == NULL)
    {
        return;
    }


    // ------------------------------------------------
    // BLUE / GREEN = CONSTANT
    // ------------------------------------------------

    if (color == LED_BLUE ||
        color == LED_GREEN)
    {
        if (led_blink_timer != NULL)
        {
            lv_timer_del(
                led_blink_timer
            );

            led_blink_timer = NULL;
        }


        led_blink_state = false;


        lv_led_set_color(
            objects.obj0,
            lv_color_hex(color)
        );

        lv_led_on(
            objects.obj0
        );

        return;
    }


    // ------------------------------------------------
    // RED / ORANGE = BLINKING
    // ------------------------------------------------

    if (color == LED_RED ||
        color == LED_ORANGE)
    {
        // Already blinking with same color
        if (led_blink_timer != NULL &&
            led_blink_color == color)
        {
            return;
        }


        // Delete old timer
        if (led_blink_timer != NULL)
        {
            lv_timer_del(
                led_blink_timer
            );

            led_blink_timer = NULL;
        }


        led_blink_color = color;
        led_blink_state = true;


        lv_led_set_color(
            objects.obj0,
            lv_color_hex(color)
        );

        lv_led_on(
            objects.obj0
        );


        // LED status blink = 500 ms
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

static void led_blink_cb(lv_timer_t *timer)
{
    (void)timer;


    if (objects.obj0 == NULL)
    {
        return;
    }


    led_blink_state =
        !led_blink_state;


    if (led_blink_state)
    {
        lv_led_set_color(
            objects.obj0,
            lv_color_hex(led_blink_color)
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
// BUTTON SELECTION
// ==================================================

static void update_button_selection(void)
{
    if (objects.calibrate_button == NULL ||
        objects.next_page_button == NULL)
    {
        Serial.println(
            "ERROR: Button objects are NULL"
        );

        return;
    }


    // ------------------------------------------------
    // REMOVE BORDER FROM BOTH BUTTONS
    // ------------------------------------------------

    lv_obj_set_style_border_width(
        objects.calibrate_button,
        0,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        objects.next_page_button,
        0,
        LV_PART_MAIN
    );


    // ------------------------------------------------
    // SELECT CURRENT BUTTON
    // ------------------------------------------------

    lv_obj_t *selected_btn = NULL;


    if (selected_option == OPTION_CALIBRATE)
    {
        selected_btn =
            objects.calibrate_button;

        Serial.println(
            "LCD SELECTED: CALIBRATE"
        );
    }
    else
    {
        selected_btn =
            objects.next_page_button;

        Serial.println(
            "LCD SELECTED: NEXT PAGE"
        );
    }


    // ------------------------------------------------
    // GREEN BORDER
    // ------------------------------------------------

    lv_obj_set_style_border_color(
        selected_btn,
        lv_color_hex(0x00FF00),
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


    // No blinking
    lv_obj_invalidate(
        selected_btn
    );
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


    // Start with CALIBRATE selected
    selected_option =
        OPTION_CALIBRATE;


    update_button_selection();
}


// ==================================================
// BUTTON PROCESSING
// ==================================================

static void buttons_run(void)
{
    uint32_t now =
        millis();


    // ------------------------------------------------
    // READ BUTTONS
    // ------------------------------------------------

    bool right_read =
        digitalRead(BTN_RIGHT);

    bool select_read =
        digitalRead(BTN_SELECT);


    // =================================================
    // RIGHT BUTTON DEBOUNCE
    // =================================================

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


            // Button pressed
            if (right_stable_state == LOW)
            {
                Serial.println(
                    "BUTTON: NEXT"
                );


                // Toggle selection
                if (selected_option ==
                    OPTION_CALIBRATE)
                {
                    selected_option =
                        OPTION_NEXT_PAGE;
                }
                else
                {
                    selected_option =
                        OPTION_CALIBRATE;
                }


                update_button_selection();
            }
        }
    }


    // =================================================
    // SELECT BUTTON DEBOUNCE
    // =================================================

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


            // Button pressed
            if (select_stable_state == LOW)
            {
                Serial.println(
                    "BUTTON: SELECT"
                );


                if (selected_option ==
                    OPTION_CALIBRATE)
                {
                    Serial.println(
                        "ACTION: CALIBRATE"
                    );

                    action_calibrate(NULL);
                }
                else
                {
                    Serial.println(
                        "ACTION: NEXT PAGE"
                    );

                    action_next_page(NULL);
                }
            }
        }
    }
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

    data_received =
        false;

    uart_timeout =
        false;


    // =================================================
    // BUZZER INIT
    // =================================================

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );


    // Low-Level Trigger
    // HIGH = Buzzer OFF
    buzzer_off();


    // =================================================
    // PHYSICAL BUTTONS
    // =================================================

    buttons_init();


    // =================================================
    // STATUS LED
    // =================================================

    set_status_led(
        LED_BLUE
    );


    // =================================================
    // HIDE ERROR
    // =================================================

    hide_status_error();


    Serial.println();
    Serial.println(
        "=============================="
    );

    Serial.println(
        "TASKS INITIALIZED"
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
        "LEFT   = REMOVED"
    );

    Serial.println(
        "BUTTON BLINK = OFF"
    );

    Serial.println(
        "=============================="
    );
}


// ==================================================
// TASK RUN
// ==================================================

void tasks_run(void)
{
    uint32_t now =
        millis();


    // =================================================
    // PHYSICAL BUTTONS
    // =================================================

    buttons_run();


    // =================================================
    // UART RECEIVE
    // =================================================

    uart_receive();


    // =================================================
    // GET UART VALUES
    // =================================================

    if (uart_get_values(
            &voltage,
            &current))
    {
        data_received =
            true;

        last_uart_data =
            now;

        uart_timeout =
            false;


        // ------------------------------------------------
        // FORMAT VALUES
        // ------------------------------------------------

        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            voltage
        );

        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            current
        );


        // ------------------------------------------------
        // UPDATE LCD
        // ------------------------------------------------

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


        // =================================================
        // CHECK VOLTAGE
        // =================================================

        bool voltage_ok =
            (
                voltage >= VOLTAGE_MIN &&
                voltage <= VOLTAGE_MAX
            );


        // =================================================
        // CHECK CURRENT
        // =================================================

        bool current_ok =
            (
                current >= CURRENT_MIN &&
                current <= CURRENT_MAX
            );


        // =================================================
        // LOW VOLTAGE ERROR + BUZZER
        // =================================================

        if (voltage < VOLTAGE_MIN)
        {
            // Show LOW VOLTAGE
            show_low_voltage_error();


            // Low-Level Trigger:
            // LOW = Buzzer ON
            buzzer_on();
        }
        else
        {
            // Hide error
            hide_status_error();


            // Buzzer OFF
            buzzer_off();
        }


        // =================================================
        // SYSTEM STATUS
        // =================================================

        bool system_ok =
            voltage_ok &&
            current_ok;


        if (system_ok)
        {
            set_status_led(
                LED_GREEN
            );


            Serial.print(
                "V = "
            );

            Serial.print(
                voltage,
                2
            );

            Serial.print(
                " | I = "
            );

            Serial.print(
                current,
                2
            );

            Serial.println(
                " | STATUS = NORMAL"
            );
        }
        else
        {
            set_status_led(
                LED_RED
            );


            Serial.print(
                "V = "
            );

            Serial.print(
                voltage,
                2
            );

            Serial.print(
                " | I = "
            );

            Serial.print(
                current,
                2
            );

            Serial.println(
                " | STATUS = ERROR"
            );
        }
    }


    // =================================================
    // UART TIMEOUT
    // =================================================

    if (data_received &&
        (now - last_uart_data >=
         UART_TIMEOUT))
    {
        if (!uart_timeout)
        {
            uart_timeout =
                true;


            // Orange LED
            set_status_led(
                LED_ORANGE
            );


            // Show connection lost
            show_connection_lost_error();


            // Buzzer OFF
            // Only LOW VOLTAGE causes buzzer
            buzzer_off();


            Serial.println(
                "UART TIMEOUT - CONNECTION LOST"
            );
        }
    }


    // =================================================
    // LVGL
    // =================================================

    if ((now - last_lvgl) >= 5)
    {
        last_lvgl =
            now;


        lv_timer_handler();


        ui_tick();
    }
}