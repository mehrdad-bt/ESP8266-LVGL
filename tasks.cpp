#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

extern "C"
{
    #include "ui/ui.h"
}

#include "uart.h"


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
// FUNCTION DECLARATION
// ==================================================

static void led_blink_cb(lv_timer_t *timer);


// ==================================================
// TASK TIMING
// ==================================================

static uint32_t last_lvgl = 0;


// ==================================================
// UART VALUES
// ==================================================

static float voltage = 0.0f;

static float current = 0.0f;


// ==================================================
// UART STATUS
// ==================================================

static bool data_received = false;

static bool uart_timeout = false;

static uint32_t last_uart_data = 0;


// ==================================================
// DISPLAY TEXT
// ==================================================

static char voltage_text[32];

static char current_text[32];


// ==================================================
// LIMITS
// ==================================================

#define VOLTAGE_MIN 20.0f
#define VOLTAGE_MAX 25.0f

#define CURRENT_MIN 0.0f
#define CURRENT_MAX 1.0f


// ==================================================
// UART TIMEOUT
// ==================================================

#define UART_TIMEOUT 3000UL


// ==================================================
// ERROR BOX
// ==================================================

// --------------------------------------------------
// LOW VOLTAGE
// --------------------------------------------------

static void show_low_voltage_error(void)
{
    if (objects.error_box != NULL)
    {
        // Dark red background
        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(0x8B0000),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_set_style_bg_opa(
            objects.error_box,
            LV_OPA_COVER,
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        // Red border
        lv_obj_set_style_border_color(
            objects.error_box,
            lv_color_hex(0xFF4444),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_set_style_border_width(
            objects.error_box,
            2,
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        // Show box
        lv_obj_clear_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    if (objects.low_voltage_label != NULL)
    {
        lv_label_set_text(
            objects.low_voltage_label,
            "LOW VOLTAGE !"
        );

        lv_obj_set_style_text_color(
            objects.low_voltage_label,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_clear_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}


// --------------------------------------------------
// CONNECTION LOST
// --------------------------------------------------

static void show_connection_lost_error(void)
{
    if (objects.error_box != NULL)
    {
        // Orange background
        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(0xCC6600),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_set_style_bg_opa(
            objects.error_box,
            LV_OPA_COVER,
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        // Orange border
        lv_obj_set_style_border_color(
            objects.error_box,
            lv_color_hex(0xFFAA00),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_set_style_border_width(
            objects.error_box,
            2,
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        // Show box
        lv_obj_clear_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    if (objects.low_voltage_label != NULL)
    {
        lv_label_set_text(
            objects.low_voltage_label,
            "CONNECTION LOST"
        );

        lv_obj_set_style_text_color(
            objects.low_voltage_label,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN | LV_STATE_DEFAULT
        );

        lv_obj_clear_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }
}


// --------------------------------------------------
// HIDE ERROR BOX
// --------------------------------------------------

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
// SET STATUS LED
// ==================================================

static void set_status_led(uint32_t color)
{
    if (objects.obj0 == NULL)
        return;


    // ==================================================
    // BLUE / GREEN = SOLID
    // ==================================================

    if (
        color == LED_BLUE ||
        color == LED_GREEN
    )
    {
        if (led_blink_timer != NULL)
        {
            lv_timer_del(led_blink_timer);
            led_blink_timer = NULL;
        }

        led_blink_state = false;

        lv_led_set_color(
            objects.obj0,
            lv_color_hex(color)
        );

        lv_led_on(objects.obj0);

        return;
    }


    // ==================================================
    // RED / ORANGE = BLINK
    // ==================================================

    if (
        color == LED_RED ||
        color == LED_ORANGE
    )
    {
        // Already blinking with same color
        if (
            led_blink_timer != NULL &&
            led_blink_color == color
        )
        {
            return;
        }

        // Delete previous blink timer
        if (led_blink_timer != NULL)
        {
            lv_timer_del(led_blink_timer);
            led_blink_timer = NULL;
        }

        led_blink_color = color;

        led_blink_state = true;

        lv_led_set_color(
            objects.obj0,
            lv_color_hex(led_blink_color)
        );

        lv_led_on(objects.obj0);

        led_blink_timer = lv_timer_create(
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
        return;

    led_blink_state = !led_blink_state;

    if (led_blink_state)
    {
        lv_led_set_color(
            objects.obj0,
            lv_color_hex(led_blink_color)
        );

        lv_led_on(objects.obj0);
    }
    else
    {
        lv_led_off(objects.obj0);
    }
}


// ==================================================
// TASK INITIALIZATION
// ==================================================

void tasks_init(void)
{
    last_lvgl = millis();

    last_uart_data = millis();

    data_received = false;

    uart_timeout = false;


    // ==================================================
    // INITIAL STATUS
    // ==================================================

    set_status_led(LED_BLUE);


    // ==================================================
    // HIDE ERROR BOX
    // ==================================================

    hide_status_error();
}


// ==================================================
// TASK RUNNER
// ==================================================

void tasks_run(void)
{
    uint32_t now = millis();


    // ==================================================
    // UART RECEIVE
    // ==================================================

    uart_receive();


    // ==================================================
    // GET NEW UART VALUES
    // ==================================================

    if (
        uart_get_values(
            &voltage,
            &current
        )
    )
    {
        // We received valid data
        data_received = true;

        last_uart_data = now;

        uart_timeout = false;


        // ==================================================
        // VOLTAGE TEXT
        // ==================================================

        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            voltage
        );


        // ==================================================
        // CURRENT TEXT
        // ==================================================

        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            current
        );


        // ==================================================
        // UPDATE VOLTAGE LABEL
        // ==================================================

        if (objects.voltage != NULL)
        {
            lv_label_set_text(
                objects.voltage,
                voltage_text
            );
        }


        // ==================================================
        // UPDATE CURRENT LABEL
        // ==================================================

        if (objects.current != NULL)
        {
            lv_label_set_text(
                objects.current,
                current_text
            );
        }


        // ==================================================
        // CHECK VOLTAGE
        // ==================================================

        bool voltage_ok =
            (
                voltage >= VOLTAGE_MIN &&
                voltage <= VOLTAGE_MAX
            );


        // ==================================================
        // CHECK CURRENT
        // ==================================================

        bool current_ok =
            (
                current >= CURRENT_MIN &&
                current <= CURRENT_MAX
            );


        // ==================================================
        // ERROR BOX
        // ==================================================

        if (voltage < VOLTAGE_MIN)
        {
            // Low voltage
            show_low_voltage_error();
        }
        else
        {
            // Voltage is not low
            // Hide previous error box
            hide_status_error();
        }


        // ==================================================
        // SYSTEM STATUS
        // ==================================================

        bool system_ok =
            voltage_ok &&
            current_ok;


        // ==================================================
        // NORMAL
        // ==================================================

        if (system_ok)
        {
            set_status_led(LED_GREEN);

            Serial.print("Voltage = ");
            Serial.print(voltage, 2);

            Serial.print(" V | Current = ");
            Serial.print(current, 2);

            Serial.println(
                " A | STATUS = NORMAL"
            );
        }


        // ==================================================
        // ERROR
        // ==================================================

        else
        {
            set_status_led(LED_RED);

            Serial.print("Voltage = ");
            Serial.print(voltage, 2);

            Serial.print(" V | Current = ");
            Serial.print(current, 2);

            Serial.println(
                " A | STATUS = ERROR"
            );
        }
    }


    // ==================================================
    // UART TIMEOUT
    // ==================================================

    if (
        data_received &&
        !uart_timeout
    )
    {
        if (
            now - last_uart_data >=
            UART_TIMEOUT
        )
        {
            uart_timeout = true;


            // ==================================================
            // ORANGE BLINKING LED
            // ==================================================

            set_status_led(LED_ORANGE);


            // ==================================================
            // ORANGE ERROR BOX
            // ==================================================

            show_connection_lost_error();


            Serial.println(
                "STATUS = UART TIMEOUT"
            );
        }
    }


    // ==================================================
    // LVGL
    // ==================================================

    if (
        now - last_lvgl >= 5
    )
    {
        last_lvgl = now;

        lv_timer_handler();

        ui_tick();
    }
}