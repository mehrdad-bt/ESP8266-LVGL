#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

extern "C"
{
    #include "ui/ui.h"
}

#include "uart.h"


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
// LED COLORS
// ==================================================

#define LED_BLUE   0x0000FF
#define LED_GREEN  0x00FF00
#define LED_RED    0xFF0000
#define LED_ORANGE 0xFFA500


// ==================================================
// SET LED COLOR
// ==================================================

static void set_status_led(
    uint32_t color
)
{
    if (objects.obj0 == NULL)
    {
        return;
    }


    lv_led_set_color(
        objects.obj0,
        lv_color_hex(color)
    );


    lv_led_on(
        objects.obj0
    );
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


    // ==============================================
    // Initial LED
    // ==============================================

    set_status_led(
        LED_BLUE
    );
}


// ==================================================
// TASK RUNNER
// ==================================================

void tasks_run(void)
{
    uint32_t now = millis();


    // ==============================================
    // UART RECEIVE
    // ==============================================

    uart_receive();


    // ==============================================
    // GET NEW VALUES
    // ==============================================

    if (
        uart_get_values(
            &voltage,
            &current
        )
    )
    {
        // ==========================================
        // New data received
        // ==========================================

        data_received = true;

        last_uart_data = now;


        // ==========================================
        // Clear timeout
        // ==========================================

        uart_timeout = false;


        // ==========================================
        // Voltage text
        // ==========================================

        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            voltage
        );


        // ==========================================
        // Current text
        // ==========================================

        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            current
        );


        // ==========================================
        // Update Voltage Label
        // ==========================================

        if (objects.voltage != NULL)
        {
            lv_label_set_text(
                objects.voltage,
                voltage_text
            );
        }


        // ==========================================
        // Update Current Label
        // ==========================================

        if (objects.current != NULL)
        {
            lv_label_set_text(
                objects.current,
                current_text
            );
        }


        // ==========================================
        // CHECK VOLTAGE
        // ==========================================

        bool voltage_ok =
            (
                voltage >= VOLTAGE_MIN &&
                voltage <= VOLTAGE_MAX
            );


        // ==========================================
        // CHECK CURRENT
        // ==========================================

        bool current_ok =
            (
                current >= CURRENT_MIN &&
                current <= CURRENT_MAX
            );


        // ==========================================
        // SYSTEM STATUS
        // ==========================================

        bool system_ok =
            voltage_ok &&
            current_ok;


        // ==========================================
        // LED STATUS
        // ==========================================

        if (system_ok)
        {
            // --------------------------------------
            // GREEN = NORMAL
            // --------------------------------------

            set_status_led(
                LED_GREEN
            );


            Serial.print(
                "Voltage = "
            );

            Serial.print(
                voltage,
                2
            );

            Serial.print(
                " V | Current = "
            );

            Serial.print(
                current,
                2
            );

            Serial.println(
                " A | STATUS = NORMAL"
            );
        }
        else
        {
            // --------------------------------------
            // RED = ERROR
            // --------------------------------------

            set_status_led(
                LED_RED
            );


            Serial.print(
                "Voltage = "
            );

            Serial.print(
                voltage,
                2
            );

            Serial.print(
                " V | Current = "
            );

            Serial.print(
                current,
                2
            );

            Serial.println(
                " A | STATUS = ERROR"
            );
        }
    }


    // ==============================================
    // UART TIMEOUT
    // ==============================================

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


            // --------------------------------------
            // ORANGE = COMMUNICATION LOST
            // --------------------------------------

            set_status_led(
                LED_ORANGE
            );


            Serial.println(
                "STATUS = UART TIMEOUT"
            );
        }
    }


    // ==============================================
    // LVGL
    // ==============================================

    if (
        now - last_lvgl >= 5
    )
    {
        last_lvgl = now;


        lv_timer_handler();


        ui_tick();
    }
}