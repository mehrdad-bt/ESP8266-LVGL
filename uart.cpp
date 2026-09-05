#include "uart.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>


// ==================================================
// UART BUFFER
// ==================================================

#define UART_BUFFER_SIZE 128

static char uart_buffer[UART_BUFFER_SIZE];

static uint16_t uart_index = 0;


// ==================================================
// PARSED VALUES
// ==================================================

static float uart_voltage = 0.0f;

static float uart_current = 0.0f;

static bool uart_values_ready = false;


// ==================================================
// MESSAGE BUFFER
// ==================================================

static char uart_message[UART_BUFFER_SIZE];

static bool uart_message_ready = false;


// ==================================================
// SERIAL INIT
// ==================================================

extern "C"
{

void serial_init(void)
{
    Serial.begin(115200);

    Serial.println();

    Serial.println("================================");
    Serial.println("UART initialized");
    Serial.println("Baud rate: 115200");
    Serial.println("================================");
}


// ==================================================
// UART RECEIVE
// ==================================================

void uart_receive(void)
{
    while (Serial.available())
    {
        char c = Serial.read();


        // ==========================================
        // ENTER
        // ==========================================

        if (c == '\n' || c == '\r')
        {
            if (uart_index > 0)
            {
                uart_buffer[uart_index] = '\0';


                // ==================================
                // PRINT RECEIVED DATA
                // ==================================

                Serial.print("Received: ");

                Serial.println(
                    uart_buffer
                );


                // ==================================
                // PARSE
                //
                // Format:
                //
                // Voltage,Current
                //
                // Example:
                //
                // 23.75,0.82
                // ==================================

                float voltage;
                float current;


                int result = sscanf(
                    uart_buffer,
                    "%f,%f",
                    &voltage,
                    &current
                );


                // ==================================
                // VALID DATA
                // ==================================

                if (result == 2)
                {
                    uart_voltage = voltage;

                    uart_current = current;

                    uart_values_ready = true;


                    Serial.print(
                        "Voltage: "
                    );

                    Serial.print(
                        uart_voltage,
                        2
                    );

                    Serial.println(" V");


                    Serial.print(
                        "Current: "
                    );

                    Serial.print(
                        uart_current,
                        2
                    );

                    Serial.println(" A");
                }


                // ==================================
                // INVALID DATA
                // ==================================

                else
                {
                    Serial.println(
                        "ERROR: Invalid UART format"
                    );

                    Serial.println(
                        "Expected: Voltage,Current"
                    );

                    Serial.println(
                        "Example: 23.75,0.82"
                    );
                }


                // ==================================
                // SAVE ORIGINAL MESSAGE
                // ==================================

                if (!uart_message_ready)
                {
                    strncpy(
                        uart_message,
                        uart_buffer,
                        UART_BUFFER_SIZE
                    );

                    uart_message[
                        UART_BUFFER_SIZE - 1
                    ] = '\0';

                    uart_message_ready = true;
                }


                // ==================================
                // RESET BUFFER
                // ==================================

                uart_index = 0;
            }
        }


        // ==========================================
        // NORMAL CHARACTER
        // ==========================================

        else
        {
            if (
                uart_index <
                UART_BUFFER_SIZE - 1
            )
            {
                uart_buffer[
                    uart_index++
                ] = c;
            }
        }
    }
}


// ==================================================
// GET MESSAGE
// ==================================================

bool uart_get_message(
    char *buffer,
    uint16_t size
)
{
    if (!uart_message_ready)
    {
        return false;
    }


    if (
        buffer == NULL ||
        size == 0
    )
    {
        return false;
    }


    strncpy(
        buffer,
        uart_message,
        size
    );

    buffer[size - 1] = '\0';

    uart_message_ready = false;

    return true;
}


// ==================================================
// GET VOLTAGE / CURRENT
// ==================================================

bool uart_get_values(
    float *voltage,
    float *current
)
{
    if (!uart_values_ready)
    {
        return false;
    }


    if (
        voltage == NULL ||
        current == NULL
    )
    {
        return false;
    }


    *voltage = uart_voltage;

    *current = uart_current;

    uart_values_ready = false;

    return true;
}

}