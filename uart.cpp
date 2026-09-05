#include "uart.h"

#include <Arduino.h>
#include <lvgl.h>

extern "C" {
    #include "ui/ui.h"
}


// ==================================================
// UART BUFFER
// ==================================================

#define UART_BUFFER_SIZE 128

static char uart_buffer[UART_BUFFER_SIZE];

static uint16_t uart_index = 0;


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


        // ------------------------------------------
        // ENTER
        // ------------------------------------------

        if (c == '\n' || c == '\r')
        {
            if (uart_index > 0)
            {
                uart_buffer[uart_index] = '\0';


                // ----------------------------------
                // نمایش در Serial Monitor
                // ----------------------------------

                Serial.print("Received: ");
                Serial.println(uart_buffer);


                // ----------------------------------
                // نمایش روی LCD
                // ----------------------------------

                if (objects.main_page_label != NULL)
                {
                    lv_label_set_text(
                        objects.main_page_label,
                        uart_buffer
                    );
                }


                // ----------------------------------
                // آماده دریافت پیام بعدی
                // ----------------------------------

                uart_index = 0;
            }
        }


        // ------------------------------------------
        // NORMAL CHARACTER
        // ------------------------------------------

        else
        {
            if (uart_index < UART_BUFFER_SIZE - 1)
            {
                uart_buffer[uart_index++] = c;
            }
        }
    }
}

}