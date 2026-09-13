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

static uint32_t last_valid_uart_time = 0;

static bool valid_uart_received_once = false;

// ==================================================
// BUTTON STATE
// ==================================================

static bool right_last_state = HIGH;
static bool select_last_state = HIGH;

static uint32_t right_last_change = 0;
static uint32_t select_last_change = 0;

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

static ErrorType led_last_state =
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

static ErrorType gui_last_error =
    ERROR_NONE;

// ==================================================
// V/C GUI CACHE
// ==================================================

static float gui_last_voltage_min =
    -1000.0f;

static float gui_last_voltage_max =
    -1000.0f;

static float gui_last_current_min =
    -1000.0f;

static float gui_last_current_max =
    -1000.0f;

// ==================================================
// INTERNAL ERROR MESSAGE LABEL
// ==================================================
//
// This label is created INSIDE the real EEZ-generated
// LV_MSGBOX content object.
//
// We are NOT creating a manual error box.
// ==================================================

static lv_obj_t *error_msg_label =
    NULL;

// ==================================================
// ERROR BOX COLOR CACHE
// ==================================================

static uint32_t error_box_last_color =
    0xFFFFFFFFUL;

// ==================================================
// FORWARD DECLARATIONS
// ==================================================

static void update_led_state(void);
static void update_error_box(void);
static void update_vc_range_gui(void);

static ErrorType get_error_type(void);
static bool main_screen_active(void);
static bool vc_range_screen_active(void);

static void init_error_msgbox(void);

// ==================================================
// CHECK MAIN SCREEN
// ==================================================

static bool main_screen_active(void)
{
    if (objects.main == NULL)
    {
        return false;
    }

    return lv_scr_act() == objects.main;
}

// ==================================================
// CHECK V/C RANGE SCREEN
// ==================================================

static bool vc_range_screen_active(void)
{
    return screen_manager_get() ==
           SCREEN_ID_V_C_RANGE_SETTINGS;
}

// ==================================================
// INIT ERROR MSGBOX
// ==================================================

static void init_error_msgbox(void)
{
    // --------------------------------------------------
    // Reset pointer
    // --------------------------------------------------

    error_msg_label =
        NULL;

    // --------------------------------------------------
    // Check real EEZ MsgBox
    // --------------------------------------------------

    if (objects.error_box == NULL)
    {
        Serial.println(
            "ERROR MSGBOX INIT: NULL"
        );

        return;
    }

    // --------------------------------------------------
    // Get MsgBox content area
    //
    // This is a child object of the real LV_MSGBOX.
    // --------------------------------------------------

    lv_obj_t *content =
        lv_msgbox_get_content(
            objects.error_box
        );

    if (content == NULL)
    {
        Serial.println(
            "ERROR MSGBOX INIT: CONTENT NULL"
        );

        return;
    }

    // --------------------------------------------------
    // Create text label INSIDE MsgBox content
    // --------------------------------------------------

    error_msg_label =
        lv_label_create(
            content
        );

    if (error_msg_label == NULL)
    {
        Serial.println(
            "ERROR MSGBOX INIT: LABEL NULL"
        );

        return;
    }

    // --------------------------------------------------
    // Initial text
    // --------------------------------------------------

    lv_label_set_text(
        error_msg_label,
        ""
    );

    // --------------------------------------------------
    // Long text mode
    // --------------------------------------------------

    lv_label_set_long_mode(
        error_msg_label,
        LV_LABEL_LONG_WRAP
    );

    // --------------------------------------------------
    // Width
    // --------------------------------------------------

    lv_obj_set_width(
        error_msg_label,
        LV_PCT(100)
    );

    // --------------------------------------------------
    // Center alignment
    // --------------------------------------------------

    lv_obj_set_style_text_align(
        error_msg_label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // ==================================================
    // Move text slightly down
    // ==================================================

    lv_obj_set_y(
    error_msg_label,
    25
    );

    // --------------------------------------------------
    // White text
    // --------------------------------------------------

    lv_obj_set_style_text_color(
        error_msg_label,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // --------------------------------------------------
    // Make text visible
    // --------------------------------------------------

    lv_obj_clear_flag(
        error_msg_label,
        LV_OBJ_FLAG_HIDDEN
    );

    // --------------------------------------------------
    // Debug
    // --------------------------------------------------

    Serial.println(
        "ERROR MSGBOX CONTENT = FOUND"
    );

    Serial.println(
        "ERROR MSGBOX TEXT LABEL = CREATED"
    );
}

// ==================================================
// BUZZER SET MODE
// ==================================================

void buzzer_set_mode(uint8_t mode)
{
    if (mode > BUZZER_MODE_3)
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

void set_voltage_min_limit(float value)
{
    if (value < VOLTAGE_LIMIT_MIN)
    {
        value =
            VOLTAGE_LIMIT_MIN;
    }

    if (value > VOLTAGE_LIMIT_MAX)
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

void set_voltage_max_limit(float value)
{
    if (value < VOLTAGE_LIMIT_MIN)
    {
        value =
            VOLTAGE_LIMIT_MIN;
    }

    if (value > VOLTAGE_LIMIT_MAX)
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

void set_current_min_limit(float value)
{
    if (value < CURRENT_LIMIT_MIN)
    {
        value =
            CURRENT_LIMIT_MIN;
    }

    if (value > CURRENT_LIMIT_MAX)
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

void set_current_max_limit(float value)
{
    if (value < CURRENT_LIMIT_MIN)
    {
        value =
            CURRENT_LIMIT_MIN;
    }

    if (value > CURRENT_LIMIT_MAX)
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
// UPDATE V/C RANGE GUI
// ==================================================

static void update_vc_range_gui(void)
{
    if (!vc_range_screen_active())
    {
        return;
    }

    // ==================================================
    // VOLTAGE MIN
    // ==================================================

    if (
        objects.voltage_min_value != NULL &&
        voltage_min_limit !=
        gui_last_voltage_min
    )
    {
        char text[16];

        snprintf(
            text,
            sizeof(text),
            "%.1f",
            voltage_min_limit
        );

        lv_label_set_text(
            objects.voltage_min_value,
            text
        );

        gui_last_voltage_min =
            voltage_min_limit;
    }

    // ==================================================
    // VOLTAGE MAX
    // ==================================================

    if (
        objects.voltage_max_value != NULL &&
        voltage_max_limit !=
        gui_last_voltage_max
    )
    {
        char text[16];

        snprintf(
            text,
            sizeof(text),
            "%.1f",
            voltage_max_limit
        );

        lv_label_set_text(
            objects.voltage_max_value,
            text
        );

        gui_last_voltage_max =
            voltage_max_limit;
    }

    // ==================================================
    // CURRENT MIN
    // ==================================================

    if (
        objects.current_min_value != NULL &&
        current_min_limit !=
        gui_last_current_min
    )
    {
        char text[16];

        snprintf(
            text,
            sizeof(text),
            "%.1f",
            current_min_limit
        );

        lv_label_set_text(
            objects.current_min_value,
            text
        );

        gui_last_current_min =
            current_min_limit;
    }

    // ==================================================
    // CURRENT MAX
    // ==================================================

    if (
        objects.current_max_value != NULL &&
        current_max_limit !=
        gui_last_current_max
    )
    {
        char text[16];

        snprintf(
            text,
            sizeof(text),
            "%.1f",
            current_max_limit
        );

        lv_label_set_text(
            objects.current_max_value,
            text
        );

        gui_last_current_max =
            current_max_limit;
    }
}

// ==================================================
// LED APPLY COLOR
// ==================================================

static void led_apply_color(uint32_t color)
{
    if (!main_screen_active())
    {
        return;
    }

    if (objects.obj0 == NULL)
    {
        return;
    }

    lv_led_set_color(
        objects.obj0,
        lv_color_hex(color)
    );

    lv_led_set_brightness(
        objects.obj0,
        255
    );
}

// ==================================================
// SET SOLID LED
// ==================================================

static void set_status_led(uint32_t color)
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

static void set_status_led_blink(uint32_t color)
{
    if (
        led_blink_enable &&
        led_blink_color == color
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
    if (!main_screen_active())
    {
        return;
    }

    if (objects.obj0 == NULL)
    {
        return;
    }

    if (!led_blink_enable)
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

    if (led_blink_state)
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
    if (objects.buzzer != NULL)
    {
        lv_obj_set_style_border_width(
            objects.buzzer,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.touch_calibration != NULL)
    {
        lv_obj_set_style_border_width(
            objects.touch_calibration,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.voltage_range != NULL)
    {
        lv_obj_set_style_border_width(
            objects.voltage_range,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.exit_settings != NULL)
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

    switch (settings_selection)
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

    if (selected != NULL)
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
        digitalRead(BTN_RIGHT);

    bool select_state =
        digitalRead(BTN_SELECT);

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
            now - right_last_change >=
            BUTTON_DEBOUNCE_MS
        )
        {
            right_last_change =
                now;

            right_last_state =
                right_state;

            if (right_state == LOW)
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
            now - select_last_change >=
            BUTTON_DEBOUNCE_MS
        )
        {
            select_last_change =
                now;

            select_last_state =
                select_state;

            if (select_state == LOW)
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

                    action_exit_from_v_c_menu_to_settings(
                        NULL
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
    // UART CONNECTION
    // =================================================

    if (!valid_uart_received_once)
    {
        system_state.data_received =
            false;

        system_state.uart_timeout =
            true;

        system_state.connection_lost =
            true;
    }
    else if (
        now - last_valid_uart_time >
        UART_TIMEOUT_MS
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
        system_state.data_received =
            true;

        system_state.uart_timeout =
            false;

        system_state.connection_lost =
            false;
    }

    // =================================================
    // VOLTAGE
    // =================================================

    system_state.voltage_ok =
        (
            system_state.voltage >=
            voltage_min_limit
        )
        &&
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
        )
        &&
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

    if (!system_state.low_voltage)
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
        now - buzzer_timer <
        interval
    )
    {
        return;
    }

    buzzer_timer =
        now;

    buzzer_output_state =
        !buzzer_output_state;

    if (buzzer_output_state)
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
    if (!main_screen_active())
    {
        return;
    }

    if (objects.error_box == NULL)
    {
        return;
    }

    if (error_msg_label == NULL)
    {
        return;
    }

    ErrorType error =
        get_error_type();

    // ==================================================
    // NO ERROR
    // ==================================================

    if (
        error ==
        ERROR_NONE
    )
    {
        if (
            gui_last_error !=
            ERROR_NONE
        )
        {
            Serial.println(
                "ERROR MSGBOX: HIDE"
            );

            lv_obj_add_flag(
                objects.error_box,
                LV_OBJ_FLAG_HIDDEN
            );

            lv_label_set_text(
                error_msg_label,
                ""
            );

            gui_last_error =
                ERROR_NONE;
        }

        return;
    }

    // ==================================================
    // SAME ERROR
    // ==================================================

    if (
        error ==
        gui_last_error
    )
    {
        return;
    }

    // ==================================================
    // ERROR MESSAGE
    // ==================================================

    const char *message =
        "";

    uint32_t color =
        LED_RED;

    switch (error)
    {
        case ERROR_CONNECTION:

            message =
                "CONNECTION LOST";

            color =
                LED_ORANGE;

            break;

        case ERROR_VOLTAGE_LOW:

            message =
                "VOLTAGE TOO LOW";

            color =
                LED_RED;

            break;

        case ERROR_VOLTAGE_HIGH:

            message =
                "VOLTAGE TOO HIGH";

            color =
                LED_RED;

            break;

        case ERROR_CURRENT_LOW:

            message =
                "CURRENT TOO LOW";

            color =
                LED_RED;

            break;

        case ERROR_CURRENT_HIGH:

            message =
                "CURRENT TOO HIGH";

            color =
                LED_RED;

            break;

        default:

            message =
                "";

            break;
    }

    // ==================================================
    // LOG
    // ==================================================

    Serial.print(
        "ERROR MSGBOX UPDATE: "
    );

    Serial.println(
        message
    );

    // ==================================================
    // UPDATE INTERNAL LABEL
    // ==================================================

    lv_label_set_text(
        error_msg_label,
        message
    );

    // ==================================================
    // ERROR TEXT COLOR
    // ==================================================

    lv_obj_set_style_text_color(
        error_msg_label,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // ==================================================
    // CENTER TEXT
    // ==================================================

    lv_obj_set_style_text_align(
        error_msg_label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // ==================================================
    // ERROR BOX COLOR
    //
    // The actual box is the EEZ-generated LV_MSGBOX.
    // ==================================================

    if (
        error_box_last_color !=
        color
    )
    {
        lv_obj_set_style_bg_color(
            objects.error_box,
            lv_color_hex(color),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_bg_opa(
            objects.error_box,
            LV_OPA_COVER,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        error_box_last_color =
            color;
    }

    // ==================================================
    // SHOW MSGBOX
    // ==================================================

    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );

    // ==================================================
    // MAKE INTERNAL LABEL VISIBLE
    // ==================================================

    lv_obj_clear_flag(
        error_msg_label,
        LV_OBJ_FLAG_HIDDEN
    );

    // ==================================================
    // SEPARATE EEZ LABEL MUST STAY HIDDEN
    // ==================================================

    if (
        objects.error_label != NULL
    )
    {
        lv_obj_add_flag(
            objects.error_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    // ==================================================
    // SAVE ERROR
    // ==================================================

    gui_last_error =
        error;
}

// ==================================================
// GUI UPDATE
// ==================================================

static void gui_update(void)
{
    // ==================================================
    // MAIN SCREEN
    // ==================================================

    if (main_screen_active())
    {
        // ==================================================
        // VOLTAGE
        // ==================================================

        if (
            objects.voltage != NULL &&
            system_state.voltage !=
            gui_last_voltage
        )
        {
            char voltage_text[16];

            snprintf(
                voltage_text,
                sizeof(voltage_text),
                "%.2f",
                system_state.voltage
            );

            lv_label_set_text(
                objects.voltage,
                voltage_text
            );

            gui_last_voltage =
                system_state.voltage;
        }

        // ==================================================
        // CURRENT
        // ==================================================

        if (
            objects.current != NULL &&
            system_state.current !=
            gui_last_current
        )
        {
            char current_text[16];

            snprintf(
                current_text,
                sizeof(current_text),
                "%.2f",
                system_state.current
            );

            lv_label_set_text(
                objects.current,
                current_text
            );

            gui_last_current =
                system_state.current;
        }

        // ==================================================
        // ERROR MSGBOX
        // ==================================================

        update_error_box();
    }

    // ==================================================
    // V/C RANGE
    // ==================================================

    update_vc_range_gui();
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

    // ==================================================
    // GUI PERIOD = 20 ms
    // ==================================================

    if (
        now - last_gui_update <
        20
    )
    {
        return;
    }

    last_gui_update =
        now;

    // ==================================================
    // CUSTOM GUI
    // ==================================================

    gui_update();

    // ==================================================
    // LED
    // ==================================================

    update_led_state();

    // ==================================================
    // EEZ GENERATED TICK
    //
    // Use current screen from screen_manager.
    // Do NOT use ui_tick(), because ui.cpp has its own
    // private currentScreen.
    //
    // V/C Range is handled directly by tasks.cpp.
    // ==================================================

    if (!vc_range_screen_active())
    {
        int16_t screen_index =
            (int16_t)screen_manager_get() -
            1;

        if (
            screen_index >= 0 &&
            screen_index < 4
        )
        {
            tick_screen(
                screen_index
            );
        }
    }
}

// ==================================================
// UPDATE LED STATE
// ==================================================

static void update_led_state(void)
{
    if (!main_screen_active())
    {
        return;
    }

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

    // ==================================================
    // CONNECTION LOST
    // ==================================================

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

    // ==================================================
    // NO DATA
    // ==================================================

    if (
        !system_state.data_received
    )
    {
        set_status_led(
            LED_BLUE
        );

        return;
    }

    // ==================================================
    // OK
    // ==================================================

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

    // ==================================================
    // ERROR
    // ==================================================

    set_status_led_blink(
        LED_RED
    );
}

// ==================================================
// TASK INIT
// ==================================================

void tasks_init(void)
{
    // ==================================================
    // BUTTONS
    // ==================================================

    pinMode(
        BTN_RIGHT,
        INPUT_PULLUP
    );

    pinMode(
        BTN_SELECT,
        INPUT_PULLUP
    );

    // ==================================================
    // BUZZER
    // ==================================================

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    noTone(
        BUZZER_PIN
    );

    // ==================================================
    // INITIAL BUTTON STATE
    // ==================================================

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

    // ==================================================
    // TIMERS
    // ==================================================

    buzzer_timer =
        millis();

    led_blink_timer =
        millis();

    // ==================================================
    // LED
    // ==================================================

    if (
        objects.obj0 != NULL
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

    // ==================================================
    // ERROR MSGBOX
    // ==================================================

    if (
        objects.error_box != NULL
    )
    {
        // ----------------------------------------------
        // Hide initially
        // ----------------------------------------------

        lv_obj_add_flag(
            objects.error_box,
            LV_OBJ_FLAG_HIDDEN
        );

        // ----------------------------------------------
        // Default color = RED
        // ----------------------------------------------

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

        error_box_last_color =
            LED_RED;

        // ----------------------------------------------
        // Create actual text label inside the MsgBox
        // ----------------------------------------------

        init_error_msgbox();

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

    // ==================================================
    // SEPARATE EEZ ERROR LABEL
    // ==================================================
    //
    // This object is NOT used for error display.
    // ==================================================

    if (
        objects.error_label != NULL
    )
    {
        lv_obj_add_flag(
            objects.error_label,
            LV_OBJ_FLAG_HIDDEN
        );

        Serial.println(
            "ERROR LABEL = UNUSED"
        );
    }
    else
    {
        Serial.println(
            "ERROR LABEL = NULL"
        );
    }

    // ==================================================
    // SETTINGS HIGHLIGHT
    // ==================================================

    apply_settings_highlight();

    // ==================================================
    // V/C CACHE
    // ==================================================

    gui_last_voltage_min =
        -1000.0f;

    gui_last_voltage_max =
        -1000.0f;

    gui_last_current_min =
        -1000.0f;

    gui_last_current_max =
        -1000.0f;

    // ==================================================
    // MAIN CACHE
    // ==================================================

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
        ERROR_NONE;

    // ==================================================
    // LED CACHE
    // ==================================================

    led_last_state =
        ERROR_NONE;

    // ==================================================
    // LOG
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
        "LED = EEZ GENERATED LV_LED"
    );

    Serial.println(
        "ERROR BOX = EEZ GENERATED LV_MSGBOX"
    );

    Serial.println(
        "ERROR MSG = CHILD OF MSGBOX CONTENT"
    );

    Serial.println(
        "ERROR LABEL = UNUSED"
    );

    Serial.println(
        "ERROR CONNECTION = ORANGE"
    );

    Serial.println(
        "ERROR VOLTAGE/CURRENT = RED"
    );

    Serial.println(
        "V/C RANGE GUI UPDATE = TASKS"
    );

    Serial.println(
        "GUI LABEL UPDATE = CHANGE ONLY"
    );

    Serial.println(
        "EEZ TICK = DIRECT CURRENT SCREEN"
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
    // ==================================================
    // 1. BUTTONS
    // ==================================================

    buttons_task();

    // ==================================================
    // 2. UART
    // ==================================================

    uart_task();

    // ==================================================
    // 3. SAFETY
    // ==================================================

    safety_task();

    // ==================================================
    // 4. BUZZER
    // ==================================================

    buzzer_task();

    // ==================================================
    // 5. SCREEN MANAGER
    // ==================================================
    //
    // Screen must be applied before GUI accesses
    // screen-dependent objects.
    // ==================================================

    screen_manager_process();

    // ==================================================
    // 6. GUI
    // ==================================================

    gui_task();

    // ==================================================
    // 7. LED
    // ==================================================

    led_task();
}