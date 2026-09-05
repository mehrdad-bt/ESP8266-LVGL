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

static bool data_received = false;


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
// TASK INITIALIZATION
// ==================================================

void tasks_init(void)
{
    last_lvgl = millis();

    // ----------------------------------------------
    // Initial LED state
    // ----------------------------------------------

    if (objects.obj0 != NULL)
    {
        lv_led_set_color(
            objects.obj0,
            lv_color_hex(0x0000FF)
        );

        lv_led_on(objects.obj0);
    }
}


// ==================================================
// TASK RUNNER
// ==================================================

void tasks_run(void)
{
    uint32_t now = millis();

    // ==============================================
    // UART
    // ==============================================

    uart_receive();

    // ==============================================
    // دریافت Voltage / Current
    // ==============================================

    if (uart_get_values(&voltage, &current))
    {
        data_received = true;

        // ------------------------------------------
        // Voltage text
        // ------------------------------------------

        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            voltage
        );

        // ------------------------------------------
        // Current text
        // ------------------------------------------

        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            current
        );

        // ------------------------------------------
        // Update Voltage label
        // ------------------------------------------

        if (objects.voltage != NULL)
        {
            lv_label_set_text(
                objects.voltage,
                voltage_text
            );
        }

        // ------------------------------------------
        // Update Current label
        // ------------------------------------------

        if (objects.current != NULL)
        {
            lv_label_set_text(
                objects.current,
                current_text
            );
        }

        // ==========================================
        // STATUS
        // ==========================================

        bool voltage_ok =
            (voltage >= VOLTAGE_MIN &&
             voltage <= VOLTAGE_MAX);

        bool current_ok =
            (current >= CURRENT_MIN &&
             current <= CURRENT_MAX);

        bool system_ok =
            voltage_ok && current_ok;

        // ------------------------------------------
        // LED
        // ------------------------------------------

        if (objects.obj0 != NULL)
        {
            if (system_ok)
            {
                // GREEN = NORMAL

                lv_led_set_color(
                    objects.obj0,
                    lv_color_hex(0x00FF00)
                );

                lv_led_on(objects.obj0);
            }
            else
            {
                // RED = ERROR

                lv_led_set_color(
                    objects.obj0,
                    lv_color_hex(0xFF0000)
                );

                lv_led_on(objects.obj0);
            }
        }

        // ==========================================
        // DEBUG
        // ==========================================

        Serial.print("Voltage = ");
        Serial.print(voltage, 2);

        Serial.print(" V | Current = ");
        Serial.print(current, 2);

        if (system_ok)
        {
            Serial.println(" A | STATUS = NORMAL");
        }
        else
        {
            Serial.println(" A | STATUS = ERROR");
        }
    }

    // ==============================================
    // LVGL
    // ==============================================

    if (now - last_lvgl >= 5)
    {
        last_lvgl = now;

        lv_timer_handler();

        ui_tick();
    }
}