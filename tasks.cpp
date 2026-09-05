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
// DISPLAY TEXT
// ==================================================

static char voltage_text[32];
static char current_text[32];


// ==================================================
// TASK INITIALIZATION
// ==================================================

void tasks_init(void)
{
    last_lvgl = millis();
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
        // ------------------------------------------
        // Voltage
        // ------------------------------------------

        snprintf(
            voltage_text,
            sizeof(voltage_text),
            "%.2f V",
            voltage
        );

        // ------------------------------------------
        // Current
        // ------------------------------------------

        snprintf(
            current_text,
            sizeof(current_text),
            "%.2f A",
            current
        );

        // ------------------------------------------
        // Update LVGL labels
        // ------------------------------------------

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

        // ------------------------------------------
        // Serial debug
        // ------------------------------------------

        Serial.print("Voltage = ");
        Serial.print(voltage, 2);

        Serial.print(" V | Current = ");
        Serial.print(current, 2);

        Serial.println(" A");
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