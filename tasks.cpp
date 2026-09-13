
#include <Arduino.h>
#include <lvgl.h>

extern "C"
{
#include "ui/ui.h"
#include "ui/screens.h"
#include "ui/actions.h"
}

#include "tasks.h"
#include "uart.h"
#include "screen_manager.h"

// ==================================================
// PHYSICAL BUTTONS
// ==================================================

#define BTN_RIGHT   D1
#define BTN_SELECT  D4

// ==================================================
// BUZZER
// ==================================================

#define BUZZER_PIN  D0
#define BUZZER_FREQ 2000

// ==================================================
// BUTTON DEBOUNCE
// ==================================================

#define BUTTON_DEBOUNCE_MS 50

// ==================================================
// MAIN MENU
// ==================================================

#define MAIN_OPTION_SETTINGS 0

// ==================================================
// SETTINGS MENU
// ==================================================

#define SETTINGS_OPTION_BUZZER        0
#define SETTINGS_OPTION_CALIBRATION   1
#define SETTINGS_OPTION_VC_RANGE      2
#define SETTINGS_OPTION_BACK          3

// ==================================================
// BUZZER MODES
// ==================================================

#define BUZZER_MODE_1  0
#define BUZZER_MODE_2  1
#define BUZZER_MODE_3  2

// ==================================================
// LED COLORS
// ==================================================

#define LED_BLUE    0x0000FF
#define LED_GREEN   0x00FF00
#define LED_RED     0xFF0000
#define LED_ORANGE  0xFFA500

// ==================================================
// DEFAULT LIMITS
// ==================================================

#define DEFAULT_VOLTAGE_MIN 20.0f
#define DEFAULT_VOLTAGE_MAX 25.0f

#define DEFAULT_CURRENT_MIN 0.0f
#define DEFAULT_CURRENT_MAX 1.0f

// ==================================================
// LIMIT RANGES
// ==================================================

#define VOLTAGE_LIMIT_MIN 0.0f
#define VOLTAGE_LIMIT_MAX 30.0f

#define CURRENT_LIMIT_MIN 0.0f
#define CURRENT_LIMIT_MAX 3.0f

// ==================================================
// UART TIMEOUT
// ==================================================

#define UART_TIMEOUT_MS 3000

// ==================================================
// ERROR TYPES
// ==================================================

enum ErrorType
{
    ERROR_NONE = 0,
    ERROR_CONNECTION,
    ERROR_VOLTAGE_LOW,
    ERROR_VOLTAGE_HIGH,
    ERROR_CURRENT_LOW,
    ERROR_CURRENT_HIGH
};

// ==================================================
// SYSTEM STATE
// ==================================================

struct SystemState
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
};

static SystemState system_state =
{
    0.0f,
    0.0f,

    false,
    true,
    true,

    false,
    false,
    false,

    false
};

// ==================================================
// LIMIT VALUES
// ==================================================

static float voltage_min_limit =
    DEFAULT_VOLTAGE_MIN;

static float voltage_max_limit =
    DEFAULT_VOLTAGE_MAX;

static float current_min_limit =
    DEFAULT_CURRENT_MIN;

static float current_max_limit =
    DEFAULT_CURRENT_MAX;

// ==================================================
// UART TIMESTAMP
// ==================================================

static uint32_t last_valid_uart_time =
    0;

static bool valid_uart_received_once =
    false;

// ==================================================
// BUTTON STATE
// ==================================================

static bool right_last_state =
    HIGH;

static bool select_last_state =
    HIGH;

static uint32_t right_last_change =
    0;

static uint32_t select_last_change =
    0;

// ==================================================
// MENU SELECTION
// ==================================================

static int main_selection =
    MAIN_OPTION_SETTINGS;

static int settings_selection =
    SETTINGS_OPTION_BUZZER;

// ==================================================
// BUZZER STATE
// ==================================================

static uint8_t buzzer_mode =
    BUZZER_MODE_1;

static bool buzzer_output_state =
    false;

static uint32_t buzzer_timer =
    0;

static uint8_t buzzer_phase =
    0;

// ==================================================
// LED STATE
// ==================================================

static uint32_t led_blink_color =
    LED_BLUE;

static bool led_blink_state =
    true;

static uint32_t led_blink_timer =
    0;

static bool led_blink_enable =
    false;

static enum ErrorType led_last_state =
    ERROR_NONE;

// ==================================================
// GUI CACHE
// ==================================================

static float gui_last_voltage =
    -1000.0f;

static float gui_last_current =
    -1000.0f;

static bool gui_last_low_voltage =
    false;

static bool gui_last_connection_lost =
    false;

static bool gui_last_system_ok =
    false;

static int gui_last_error =
    -1;

// ==================================================
// FORWARD DECLARATIONS
// ==================================================

static void update_led_state(void);

static void update_error_box(void);

static ErrorType get_error_type(void);

// ==================================================
// BUZZER SET MODE
// ==================================================

void buzzer_set_mode(
    uint8_t mode
)
{
    if (
        mode >
        BUZZER_MODE_3
    )
    {
        mode =
            BUZZER_MODE_3;
    }

    buzzer_mode =
        mode;

    Serial.print(
        "BUZZER MODE = "
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
// VOLTAGE MIN
// ==================================================

void set_voltage_min_limit(
    float value
)
{
    if (
        value <
        VOLTAGE_LIMIT_MIN
    )
    {
        value =
            VOLTAGE_LIMIT_MIN;
    }

    if (
        value >
        VOLTAGE_LIMIT_MAX
    )
    {
        value =
            VOLTAGE_LIMIT_MAX;
    }

    voltage_min_limit =
        value;

    if (
        voltage_min_limit >
        voltage_max_limit
    )
    {
        voltage_max_limit =
            voltage_min_limit;
    }

    Serial.print(
        "VOLTAGE MIN LIMIT = "
    );

    Serial.println(
        voltage_min_limit,
        1
    );
}

// ==================================================
// VOLTAGE MAX
// ==================================================

void set_voltage_max_limit(
    float value
)
{
    if (
        value <
        VOLTAGE_LIMIT_MIN
    )
    {
        value =
            VOLTAGE_LIMIT_MIN;
    }

    if (
        value >
        VOLTAGE_LIMIT_MAX
    )
    {
        value =
            VOLTAGE_LIMIT_MAX;
    }

    voltage_max_limit =
        value;

    if (
        voltage_max_limit <
        voltage_min_limit
    )
    {
        voltage_min_limit =
            voltage_max_limit;
    }

    Serial.print(
        "VOLTAGE MAX LIMIT = "
    );

    Serial.println(
        voltage_max_limit,
        1
    );
}

// ==================================================
// GET VOLTAGE MIN
// ==================================================

float get_voltage_min_limit(void)
{
    return voltage_min_limit;
}

// ==================================================
// GET VOLTAGE MAX
// ==================================================

float get_voltage_max_limit(void)
{
    return voltage_max_limit;
}

// ==================================================
// CURRENT MIN
// ==================================================

void set_current_min_limit(
    float value
)
{
    if (
        value <
        CURRENT_LIMIT_MIN
    )
    {
        value =
            CURRENT_LIMIT_MIN;
    }

    if (
        value >
        CURRENT_LIMIT_MAX
    )
    {
        value =
            CURRENT_LIMIT_MAX;
    }

    current_min_limit =
        value;

    if (
        current_min_limit >
        current_max_limit
    )
    {
        current_max_limit =
            current_min_limit;
    }

    Serial.print(
        "CURRENT MIN LIMIT = "
    );

    Serial.println(
        current_min_limit,
        1
    );
}

// ==================================================
// CURRENT MAX
// ==================================================

void set_current_max_limit(
    float value
)
{
    if (
        value <
        CURRENT_LIMIT_MIN
    )
    {
        value =
            CURRENT_LIMIT_MIN;
    }

    if (
        value >
        CURRENT_LIMIT_MAX
    )
    {
        value =
            CURRENT_LIMIT_MAX;
    }

    current_max_limit =
        value;

    if (
        current_max_limit <
        current_min_limit
    )
    {
        current_min_limit =
            current_max_limit;
    }

    Serial.print(
        "CURRENT MAX LIMIT = "
    );

    Serial.println(
        current_max_limit,
        1
    );
}

// ==================================================
// GET CURRENT MIN
// ==================================================

float get_current_min_limit(void)
{
    return current_min_limit;
}

// ==================================================
// GET CURRENT MAX
// ==================================================

float get_current_max_limit(void)
{
    return current_max_limit;
}

// ==================================================
// LED APPLY COLOR
// ==================================================

static void led_apply_color(
    uint32_t color
)
{
    if (
        objects.obj0 ==
        NULL
    )
    {
        return;
    }

    lv_led_set_color(
        objects.obj0,
        lv_color_hex(
            color
        )
    );

    lv_led_set_brightness(
        objects.obj0,
        255
    );
}

// ==================================================
// SET SOLID LED
// ==================================================

static void set_status_led(
    uint32_t color
)
{
    led_blink_color =
        color;

    led_blink_enable =
        false;

    led_blink_state =
        true;

    led_blink_timer =
        millis();

    led_apply_color(
        color
    );
}

// ==================================================
// SET BLINK LED
// ==================================================

static void set_status_led_blink(
    uint32_t color
)
{
    if (
        led_blink_enable &&
        led_blink_color ==
        color
    )
    {
        return;
    }

    led_blink_color =
        color;

    led_blink_enable =
        true;

    led_blink_state =
        true;

    led_blink_timer =
        millis();

    led_apply_color(
        color
    );
}

// ==================================================
// LED TASK
// ==================================================

static void led_task(void)
{
    if (
        objects.obj0 ==
        NULL
    )
    {
        return;
    }

    if (
        !led_blink_enable
    )
    {
        return;
    }

    uint32_t now =
        millis();

    if (
        now - led_blink_timer <
        500
    )
    {
        return;
    }

    led_blink_timer =
        now;

    led_blink_state =
        !led_blink_state;

    if (
        led_blink_state
    )
    {
        led_apply_color(
            led_blink_color
        );
    }
    else
    {
        lv_led_set_brightness(
            objects.obj0,
            0
        );
    }
}

// ==================================================
// CLEAR SETTINGS HIGHLIGHT
// ==================================================

static void clear_settings_highlight(void)
{
    if (
        objects.buzzer !=
        NULL
    )
    {
        lv_obj_set_style_border_width(
            objects.buzzer,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (
        objects.touch_calibration !=
        NULL
    )
    {
        lv_obj_set_style_border_width(
            objects.touch_calibration,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (
        objects.voltage_range !=
        NULL
    )
    {
        lv_obj_set_style_border_width(
            objects.voltage_range,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (
        objects.exit_settings !=
        NULL
    )
    {
        lv_obj_set_style_border_width(
            objects.exit_settings,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

// ==================================================
// APPLY SETTINGS HIGHLIGHT
// ==================================================

static void apply_settings_highlight(void)
{
    clear_settings_highlight();

    lv_obj_t *selected =
        NULL;

    switch (
        settings_selection
    )
    {
        case SETTINGS_OPTION_BUZZER:

            selected =
                objects.buzzer;

            break;

        case SETTINGS_OPTION_CALIBRATION:

            selected =
                objects.touch_calibration;

            break;

        case SETTINGS_OPTION_VC_RANGE:

            selected =
                objects.voltage_range;

            break;

        case SETTINGS_OPTION_BACK:

            selected =
                objects.exit_settings;

            break;

        default:

            break;
    }

    if (
        selected !=
        NULL
    )
    {
        lv_obj_set_style_border_width(
            selected,
            2,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

// ==================================================
// BUTTONS TASK
// ==================================================

static void buttons_task(void)
{
    bool right_state =
        digitalRead(
            BTN_RIGHT
        );

    bool select_state =
        digitalRead(
            BTN_SELECT
        );

    uint32_t now =
        millis();

    // =================================================
    // RIGHT
    // =================================================

    if (
        right_state !=
        right_last_state
    )
    {
        if (
            now -
            right_last_change >=
            BUTTON_DEBOUNCE_MS
        )
        {
            right_last_change =
                now;

            right_last_state =
                right_state;

            if (
                right_state ==
                LOW
            )
            {
                Serial.println(
                    "BUTTON: RIGHT"
                );

                enum ScreensEnum screen =
                    screen_manager_get();

                if (
                    screen ==
                    SCREEN_ID_MAIN
                )
                {
                    main_selection =
                        MAIN_OPTION_SETTINGS;
                }

                else if (
                    screen ==
                    SCREEN_ID_SETTINGS_PAGE
                )
                {
                    settings_selection++;

                    if (
                        settings_selection >
                        SETTINGS_OPTION_BACK
                    )
                    {
                        settings_selection =
                            SETTINGS_OPTION_BUZZER;
                    }

                    apply_settings_highlight();
                }

                else if (
                    screen ==
                    SCREEN_ID_BUZZER_SETTINGS
                )
                {
                    Serial.println(
                        "BUTTON: RIGHT -> BUZZER"
                    );
                }

                else if (
                    screen ==
                    SCREEN_ID_V_C_RANGE_SETTINGS
                )
                {
                    Serial.println(
                        "BUTTON: RIGHT -> V/C RANGE"
                    );
                }
            }
        }
    }

    // =================================================
    // SELECT
    // =================================================

    if (
        select_state !=
        select_last_state
    )
    {
        if (
            now -
            select_last_change >=
            BUTTON_DEBOUNCE_MS
        )
        {
            select_last_change =
                now;

            select_last_state =
                select_state;

            if (
                select_state ==
                LOW
            )
            {
                Serial.println(
                    "BUTTON: SELECT"
                );

                enum ScreensEnum screen =
                    screen_manager_get();

                // =========================================
                // MAIN
                // =========================================

                if (
                    screen ==
                    SCREEN_ID_MAIN
                )
                {
                    if (
                        main_selection ==
                        MAIN_OPTION_SETTINGS
                    )
                    {
                        action_go_to_settings_page(
                            NULL
                        );
                    }
                }

                // =========================================
                // SETTINGS
                // =========================================

                else if (
                    screen ==
                    SCREEN_ID_SETTINGS_PAGE
                )
                {
                    switch (
                        settings_selection
                    )
                    {
                        case SETTINGS_OPTION_BUZZER:

                            Serial.println(
                                "ACTION: SETTINGS -> BUZZER"
                            );

                            action_go_to_buzzer_settings(
                                NULL
                            );

                            break;

                        case SETTINGS_OPTION_CALIBRATION:

                            Serial.println(
                                "ACTION: SETTINGS -> CALIBRATION"
                            );

                            action_go_to_touch_calibration(
                                NULL
                            );

                            break;

                        case SETTINGS_OPTION_VC_RANGE:

                            Serial.println(
                                "ACTION: SETTINGS -> V/C RANGE"
                            );

                            action_go_to_v_c_range_settings(
                                NULL
                            );

                            break;

                        case SETTINGS_OPTION_BACK:

                            Serial.println(
                                "ACTION: SETTINGS -> MAIN"
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
                    screen ==
                    SCREEN_ID_BUZZER_SETTINGS
                )
                {
                    action_go_from_buzzer_settings_page_to_settings_page(
                        NULL
                    );
                }

                // =========================================
                // V/C RANGE
                // =========================================

                else if (
                    screen ==
                    SCREEN_ID_V_C_RANGE_SETTINGS
                )
                {
                    Serial.println(
                        "ACTION: V/C RANGE -> SETTINGS"
                    );

                    /*
                     * این Action در actions.h تولید نشده.
                     * برای دکمه فیزیکی مستقیماً screen manager
                     * را صدا می‌زنیم.
                     */

                    screen_manager_show(
                        SCREEN_ID_SETTINGS_PAGE
                    );
                }
            }
        }
    }
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

        last_valid_uart_time =
            millis();

        valid_uart_received_once =
            true;
    }
}

// ==================================================
// SAFETY TASK
// ==================================================

static void safety_task(void)
{
    uint32_t now =
        millis();

    // =================================================
    // UART TIMEOUT
    // =================================================

    if (
        !valid_uart_received_once
    )
    {
        system_state.data_received =
            false;

        system_state.uart_timeout =
            true;

        system_state.connection_lost =
            true;
    }
    else
    {
        if (
            now -
            last_valid_uart_time >
            UART_TIMEOUT_MS
        )
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

    // =================================================
    // VOLTAGE
    // =================================================

    system_state.voltage_ok =
        (
            system_state.voltage >=
            voltage_min_limit
        ) &&
        (
            system_state.voltage <=
            voltage_max_limit
        );

    // =================================================
    // CURRENT
    // =================================================

    system_state.current_ok =
        (
            system_state.current >=
            current_min_limit
        ) &&
        (
            system_state.current <=
            current_max_limit
        );

    // =================================================
    // LOW VOLTAGE
    // =================================================

    system_state.low_voltage =
        (
            system_state.voltage <
            voltage_min_limit
        );

    // =================================================
    // SYSTEM OK
    // =================================================

    system_state.system_ok =
        system_state.data_received &&
        !system_state.connection_lost &&
        system_state.voltage_ok &&
        system_state.current_ok;
}

// ==================================================
// BUZZER TASK
// ==================================================

static void buzzer_task(void)
{
    if (
        !system_state.data_received ||
        system_state.connection_lost
    )
    {
        noTone(
            BUZZER_PIN
        );

        buzzer_output_state =
            false;

        buzzer_phase =
            0;

        return;
    }

    if (
        !system_state.low_voltage
    )
    {
        noTone(
            BUZZER_PIN
        );

        buzzer_output_state =
            false;

        buzzer_phase =
            0;

        return;
    }

    uint32_t now =
        millis();

    uint32_t interval =
        0;

    if (
        buzzer_mode ==
        BUZZER_MODE_1
    )
    {
        interval =
            buzzer_output_state ?
            200 :
            700;
    }
    else if (
        buzzer_mode ==
        BUZZER_MODE_2
    )
    {
        interval =
            100;
    }
    else
    {
        interval =
            500;
    }

    if (
        now -
        buzzer_timer <
        interval
    )
    {
        return;
    }

    buzzer_timer =
        now;

    buzzer_output_state =
        !buzzer_output_state;

    if (
        buzzer_output_state
    )
    {
        tone(
            BUZZER_PIN,
            BUZZER_FREQ
        );
    }
    else
    {
        noTone(
            BUZZER_PIN
        );

        if (
            buzzer_mode ==
            BUZZER_MODE_2
        )
        {
            buzzer_phase++;

            if (
                buzzer_phase >=
                2
            )
            {
                buzzer_phase =
                    0;
            }
        }
    }
}

// ==================================================
// GET ERROR TYPE
// ==================================================

static ErrorType get_error_type(void)
{
    if (
        system_state.connection_lost ||
        system_state.uart_timeout
    )
    {
        return ERROR_CONNECTION;
    }

    if (
        system_state.voltage <
        voltage_min_limit
    )
    {
        return ERROR_VOLTAGE_LOW;
    }

    if (
        system_state.voltage >
        voltage_max_limit
    )
    {
        return ERROR_VOLTAGE_HIGH;
    }

    if (
        system_state.current <
        current_min_limit
    )
    {
        return ERROR_CURRENT_LOW;
    }

    if (
        system_state.current >
        current_max_limit
    )
    {
        return ERROR_CURRENT_HIGH;
    }

    return ERROR_NONE;
}

// ==================================================
// UPDATE ERROR BOX
// ==================================================

static void update_error_box(void)
{
    if (
        objects.error_box ==
        NULL
    )
    {
        return;
    }

    ErrorType error =
        get_error_type();

    if (
        (int)error ==
        gui_last_error
    )
    {
        return;
    }

    gui_last_error =
        (int)error;

    lv_obj_t *text =
        lv_msgbox_get_text(
            objects.error_box
        );

    // =================================================
    // NO ERROR
    // =================================================

    if (
        error ==
        ERROR_NONE
    )
    {
        lv_obj_add_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );

        if (
            text !=
            NULL
        )
        {
            lv_label_set_text(
                text,
                ""
            );
        }

        return;
    }

    char message[96];

    // =================================================
    // CONNECTION
    // =================================================

    if (
        error ==
        ERROR_CONNECTION
    )
    {
        snprintf(
            message,
            sizeof(message),
            "CONNECTION LOST"
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_ORANGE
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // =================================================
    // VOLTAGE LOW
    // =================================================

    else if (
        error ==
        ERROR_VOLTAGE_LOW
    )
    {
        snprintf(
            message,
            sizeof(message),
            "VOLTAGE TOO LOW\n"
            "%.2f V < %.0f V",
            system_state.voltage,
            voltage_min_limit
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_RED
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // =================================================
    // VOLTAGE HIGH
    // =================================================

    else if (
        error ==
        ERROR_VOLTAGE_HIGH
    )
    {
        snprintf(
            message,
            sizeof(message),
            "VOLTAGE TOO HIGH\n"
            "%.2f V > %.0f V",
            system_state.voltage,
            voltage_max_limit
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_RED
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // =================================================
    // CURRENT LOW
    // =================================================

    else if (
        error ==
        ERROR_CURRENT_LOW
    )
    {
        snprintf(
            message,
            sizeof(message),
            "CURRENT TOO LOW\n"
            "%.2f A < %.0f A",
            system_state.current,
            current_min_limit
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_RED
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // =================================================
    // CURRENT HIGH
    // =================================================

    else
    {
        snprintf(
            message,
            sizeof(message),
            "CURRENT TOO HIGH\n"
            "%.2f A > %.0f A",
            system_state.current,
            current_max_limit
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_RED
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    // =================================================
    // BOX OPACITY
    // =================================================

    lv_obj_set_style_bg_opa(
        objects.error_box,
        LV_OPA_COVER,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // =================================================
    // TEXT
    // =================================================

    if (
        text !=
        NULL
    )
    {
        lv_label_set_text(
            text,
            message
        );

        lv_obj_set_style_text_color(
            text,
            lv_color_hex(
                0xFFFFFF
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_text_align(
            text,
            LV_TEXT_ALIGN_CENTER,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_label_set_long_mode(
            text,
            LV_LABEL_LONG_WRAP
        );
    }

    // =================================================
    // SHOW
    // =================================================

    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );
}

// ==================================================
// GUI UPDATE
// ==================================================

static void gui_update(void)
{
    // =================================================
    // VOLTAGE
    // =================================================

    if (
        objects.voltage != NULL &&
        system_state.voltage !=
        gui_last_voltage
    )
    {
        char text[24];

        snprintf(
            text,
            sizeof(text),
            "%.2f",
            system_state.voltage
        );

        lv_label_set_text(
            objects.voltage,
            text
        );

        gui_last_voltage =
            system_state.voltage;
    }

    // =================================================
    // CURRENT
    // =================================================

    if (
        objects.current != NULL &&
        system_state.current !=
        gui_last_current
    )
    {
        char text[24];

        snprintf(
            text,
            sizeof(text),
            "%.2f",
            system_state.current
        );

        lv_label_set_text(
            objects.current,
            text
        );

        gui_last_current =
            system_state.current;
    }

    // =================================================
    // LOW VOLTAGE LABEL
    // =================================================

    if (
        objects.low_voltage_label !=
        NULL
    )
    {
        lv_obj_add_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );

        gui_last_low_voltage =
            system_state.low_voltage;
    }

    // =================================================
    // ERROR BOX
    // =================================================

    update_error_box();

    // =================================================
    // CACHE
    // =================================================

    gui_last_connection_lost =
        system_state.connection_lost;

    gui_last_system_ok =
        system_state.system_ok;
}

// ==================================================
// GUI TASK
// ==================================================

static void gui_task(void)
{
    static uint32_t last_gui_update =
        0;

    uint32_t now =
        millis();

    if (
        now -
        last_gui_update >=
        5
    )
    {
        last_gui_update =
            now;

        gui_update();

        update_led_state();

        lv_timer_handler();

        ui_tick();
    }

    led_task();
}

// ==================================================
// UPDATE LED STATE
// ==================================================

static void update_led_state(void)
{
    ErrorType error =
        get_error_type();

    if (
        error ==
        led_last_state
    )
    {
        return;
    }

    led_last_state =
        error;

    // =================================================
    // CONNECTION
    // =================================================

    if (
        error ==
        ERROR_CONNECTION
    )
    {
        set_status_led_blink(
            LED_ORANGE
        );

        return;
    }

    // =================================================
    // NO DATA
    // =================================================

    if (
        !system_state.data_received
    )
    {
        set_status_led(
            LED_BLUE
        );

        return;
    }

    // =================================================
    // OK
    // =================================================

    if (
        error ==
        ERROR_NONE
    )
    {
        set_status_led(
            LED_GREEN
        );

        return;
    }

    // =================================================
    // FAULT
    // =================================================

    set_status_led_blink(
        LED_RED
    );
}

// ==================================================
// WATCHDOG
// ==================================================

static void watchdog_task(void)
{
    yield();
}

// ==================================================
// TASK INIT
// ==================================================

void tasks_init(void)
{
    pinMode(
        BTN_RIGHT,
        INPUT_PULLUP
    );

    pinMode(
        BTN_SELECT,
        INPUT_PULLUP
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    noTone(
        BUZZER_PIN
    );

    right_last_state =
        digitalRead(
            BTN_RIGHT
        );

    select_last_state =
        digitalRead(
            BTN_SELECT
        );

    right_last_change =
        millis();

    select_last_change =
        millis();

    buzzer_timer =
        millis();

    led_blink_timer =
        millis();

    // =================================================
    // LED
    // =================================================

    if (
        objects.obj0 !=
        NULL
    )
    {
        lv_led_set_color(
            objects.obj0,
            lv_color_hex(
                LED_BLUE
            )
        );

        lv_led_set_brightness(
            objects.obj0,
            255
        );

        Serial.println(
            "LED OBJECT = EEZ LV_LED"
        );
    }
    else
    {
        Serial.println(
            "LED OBJECT = NULL"
        );
    }

    // =================================================
    // ERROR BOX
    // =================================================

    if (
        objects.error_box !=
        NULL
    )
    {
        lv_obj_add_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );

        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(
                LED_RED
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_bg_opa(
            objects.error_box,
            LV_OPA_COVER,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_t *text =
            lv_msgbox_get_text(
                objects.error_box
            );

        if (
            text !=
            NULL
        )
        {
            lv_obj_set_style_text_color(
                text,
                lv_color_hex(
                    0xFFFFFF
                ),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            lv_obj_set_style_text_align(
                text,
                LV_TEXT_ALIGN_CENTER,
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            lv_label_set_long_mode(
                text,
                LV_LABEL_LONG_WRAP
            );
        }

        Serial.println(
            "ERROR BOX = EEZ LV_MSGBOX"
        );
    }
    else
    {
        Serial.println(
            "ERROR BOX = NULL"
        );
    }

    // =================================================
    // LOW VOLTAGE LABEL
    // =================================================

    if (
        objects.low_voltage_label !=
        NULL
    )
    {
        lv_obj_add_flag(
            objects.low_voltage_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    // =================================================
    // SETTINGS
    // =================================================

    apply_settings_highlight();

    // =================================================
    // CACHE
    // =================================================

    gui_last_voltage =
        -1000.0f;

    gui_last_current =
        -1000.0f;

    gui_last_low_voltage =
        false;

    gui_last_connection_lost =
        false;

    gui_last_system_ok =
        false;

    gui_last_error =
        -1;

    led_last_state =
        ERROR_NONE;

    // =================================================
    // LOG
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
        "LED = EEZ GENERATED LV_LED"
    );

    Serial.println(
        "ERROR BOX = EEZ GENERATED LV_MSGBOX"
    );

    Serial.println(
        "UART TIMEOUT = 3000 ms"
    );

    Serial.println(
        "DEFAULT V RANGE = 20..25 V"
    );

    Serial.println(
        "DEFAULT C RANGE = 0..1 A"
    );

    Serial.println(
        "=============================="
    );
}

// ==================================================
// TASK RUN
// ==================================================

void tasks_run()
{
    buttons_task();
    uart_task();
    safety_task();
    buzzer_task();
    gui_task();
    led_task();

    screen_manager_process();
}

