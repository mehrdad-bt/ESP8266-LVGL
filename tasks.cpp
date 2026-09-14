
#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

extern "C"
{
#include "ui/ui.h"
#include "ui/screens.h"
#include "ui/actions.h"
}

extern lv_obj_t *tick_value_change_obj;

#include "tasks.h"
#include "uart.h"
#include "screen_manager.h"
#include "ui/vars.h"

// ==================================================
// Hardware Configuration
// ==================================================

#define BTN_RIGHT   D1
#define BTN_SELECT  D4

#define BUZZER_PIN  D0
#define BUZZER_FREQ 2000

// ==================================================
// Timing Configuration
// ==================================================

#define BUTTON_DEBOUNCE_MS 50
#define UART_TIMEOUT_MS 3000
#define ERROR_TEXT_BLINK_MS 500

// ==================================================
// Menu Indices
// ==================================================

#define MAIN_OPTION_SETTINGS 0

#define SETTINGS_OPTION_BUZZER        0
#define SETTINGS_OPTION_CALIBRATION   1
#define SETTINGS_OPTION_VC_RANGE      2
#define SETTINGS_OPTION_BACK          3

// ==================================================
// Buzzer Modes
// ==================================================

#define BUZZER_MODE_1  0
#define BUZZER_MODE_2  1
#define BUZZER_MODE_3  2

// ==================================================
// LED Colors
// ==================================================

#define LED_BLUE    0x0000FF
#define LED_GREEN   0x00FF00
#define LED_RED     0xFF0000
#define LED_ORANGE  0xFFA500

#define FOCUS_COLOR 0xFF0000

// ==================================================
// Default Voltage and Current Limits
// ==================================================

#define DEFAULT_VOLTAGE_MIN 20.0f
#define DEFAULT_VOLTAGE_MAX 25.0f

#define DEFAULT_CURRENT_MIN 0.0f
#define DEFAULT_CURRENT_MAX 1.0f

// ==================================================
// Allowed Voltage and Current Limits
// ==================================================

#define VOLTAGE_LIMIT_MIN 0.0f
#define VOLTAGE_LIMIT_MAX 30.0f

#define CURRENT_LIMIT_MIN 0.0f
#define CURRENT_LIMIT_MAX 3.0f

// ==================================================
// Error Types
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
// System State
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
// Limit Values
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
// UART State
// ==================================================

static uint32_t last_valid_uart_time = 0;
static bool valid_uart_received_once = false;

// ==================================================
// Button State
// ==================================================

static bool right_last_state = HIGH;
static bool select_last_state = HIGH;

static uint32_t right_last_change = 0;
static uint32_t select_last_change = 0;

// ==================================================
// Menu State
// ==================================================

static int main_selection =
    MAIN_OPTION_SETTINGS;

static int settings_selection =
    SETTINGS_OPTION_BUZZER;

// ==================================================
// Buzzer Runtime State
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
// Buzzer UI State
// ==================================================

static lv_obj_t *buzzer_dropdown =
    NULL;

static bool buzzer_dropdown_open =
    false;

static bool buzzer_focus_back =
    false;

// ==================================================
// Voltage / Current UI State
// ==================================================

// 0 = Voltage minimum
// 1 = Voltage maximum
// 2 = Current minimum
// 3 = Current maximum
// 4 = Back button

static uint8_t vc_focus =
    0;

static bool vc_edit_mode =
    false;

// ==================================================
// LED State
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
// Main GUI Cache
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
// Voltage / Current GUI Cache
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
// Error Message Label
// ==================================================

static lv_obj_t *error_msg_label =
    NULL;

// ==================================================
// Error Text Blink State
// ==================================================

static uint32_t error_text_blink_timer =
    0;

static bool error_text_blink_state =
    true;

// ==================================================
// Error Box Appearance Cache
// ==================================================

static uint32_t error_box_last_color =
    0xFFFFFFFFUL;

// ==================================================
// Focus Screen Cache
// ==================================================

static enum ScreensEnum last_focus_screen =
    SCREEN_ID_MAIN;

static int last_buzzer_focus_state =
    -1;

static int last_vc_focus_state =
    -1;

static bool last_vc_edit_state =
    false;

// ==================================================
// Forward Declarations
// ==================================================

static void update_led_state(void);
static void update_error_box(void);
static void update_error_text_blink(void);
static void update_vc_range_gui(void);
static void update_buzzer_gui(void);
static void update_input_focus_gui(void);

static ErrorType get_error_type(void);

static bool main_screen_active(void);
static bool vc_range_screen_active(void);
static bool buzzer_screen_active(void);

static void init_error_msgbox(void);
static void set_error_text(const char *text);

static void buttons_task(void);

static void buzzer_dropdown_find(void);
static void buzzer_dropdown_change(int direction);

static void vc_change_value(void);

static void clear_buzzer_focus(void);
static void apply_buzzer_focus(void);

static void clear_vc_focus(void);
static void apply_vc_focus(void);

static void handle_right_release(void);
static void handle_select_release(void);

// ==================================================
// Screen State Helpers
// ==================================================

static bool main_screen_active(void)
{
    if (objects.main == NULL)
    {
        return false;
    }

    return lv_scr_act() == objects.main;
}

static bool vc_range_screen_active(void)
{
    return screen_manager_get() ==
           SCREEN_ID_V_C_RANGE_SETTINGS;
}

static bool buzzer_screen_active(void)
{
    return screen_manager_get() ==
           SCREEN_ID_BUZZER_SETTINGS;
}

// ==================================================
// Buzzer Dropdown Helper
// ==================================================

static void buzzer_dropdown_find(void)
{
    if (objects.buzzer_settings == NULL)
    {
        buzzer_dropdown = NULL;
        return;
    }

    buzzer_dropdown =
        lv_obj_get_child(
            objects.buzzer_settings,
            0
        );
}

// ==================================================
// Error Message Box Initialization
// ==================================================

static void init_error_msgbox(void)
{
    error_msg_label = NULL;

    if (objects.error_box == NULL)
    {
        return;
    }

    lv_obj_t *content =
        lv_msgbox_get_content(
            objects.error_box
        );

    if (content == NULL)
    {
        return;
    }

    error_msg_label =
        lv_label_create(
            content
        );

    if (error_msg_label == NULL)
    {
        return;
    }

    lv_label_set_text(
        error_msg_label,
        ""
    );

    lv_label_set_long_mode(
        error_msg_label,
        LV_LABEL_LONG_WRAP
    );

    lv_obj_set_width(
        error_msg_label,
        LV_PCT(100)
    );

    lv_obj_set_style_text_align(
        error_msg_label,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    lv_obj_set_style_text_color(
        error_msg_label,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    lv_obj_set_y(
        error_msg_label,
        40
    );

    lv_obj_set_style_border_width(
        error_msg_label,
        0,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    lv_obj_set_style_bg_opa(
        error_msg_label,
        LV_OPA_TRANSP,
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    lv_obj_clear_flag(
        error_msg_label,
        LV_OBJ_FLAG_HIDDEN
    );

    error_text_blink_timer =
        millis();

    error_text_blink_state =
        true;
}

// ==================================================
// Set Error Message Text
// ==================================================

static void set_error_text(
    const char *text
)
{
    if (text == NULL)
    {
        text = "";
    }

    if (error_msg_label != NULL)
    {
        lv_label_set_text(
            error_msg_label,
            text
        );

        lv_obj_set_style_text_color(
            error_msg_label,
            lv_color_hex(0xFFFFFF),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    error_text_blink_timer =
        millis();

    error_text_blink_state =
        true;
}

// ==================================================
// Error Text Blink Task
// ==================================================

static void update_error_text_blink(void)
{
    if (!main_screen_active())
    {
        return;
    }

    if (error_msg_label == NULL)
    {
        return;
    }

    if (
        gui_last_error ==
        ERROR_NONE
    )
    {
        return;
    }

    uint32_t now =
        millis();

    if (
        now - error_text_blink_timer <
        ERROR_TEXT_BLINK_MS
    )
    {
        return;
    }

    error_text_blink_timer =
        now;

    error_text_blink_state =
        !error_text_blink_state;

    uint32_t color;

    if (error_text_blink_state)
    {
        color =
            0x000000;
    }
    else
    {
        color =
            0xFFFFFF;
    }

    lv_obj_set_style_text_color(
        error_msg_label,
        lv_color_hex(color),
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );
}

// ==================================================
// Buzzer Mode Control
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
}

uint8_t buzzer_get_mode(void)
{
    return buzzer_mode;
}

// ==================================================
// Voltage Limit Setters
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
}

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
}

float get_voltage_min_limit(void)
{
    return voltage_min_limit;
}

float get_voltage_max_limit(void)
{
    return voltage_max_limit;
}

// ==================================================
// Current Limit Setters
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
}

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
}

float get_current_min_limit(void)
{
    return current_min_limit;
}

float get_current_max_limit(void)
{
    return current_max_limit;
}

// ==================================================
// Update V/C Screen
// ==================================================

static void update_vc_range_gui(void)
{
    if (!vc_range_screen_active())
    {
        return;
    }

    // --------------------------------------------------
    // Voltage minimum display
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Voltage maximum display
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Current minimum display
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Current maximum display
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Synchronize voltage minimum slider
    // --------------------------------------------------

    if (objects.voltage_minimum != NULL)
    {
        int32_t value =
            (int32_t)voltage_min_limit;

        int32_t current =
            lv_slider_get_value(
                objects.voltage_minimum
            );

        if (value != current)
        {
            tick_value_change_obj =
                objects.voltage_minimum;

            lv_slider_set_value(
                objects.voltage_minimum,
                value,
                LV_ANIM_OFF
            );

            tick_value_change_obj =
                NULL;
        }
    }

    // --------------------------------------------------
    // Synchronize voltage maximum slider
    // --------------------------------------------------

    if (objects.voltage_maximum != NULL)
    {
        int32_t value =
            (int32_t)voltage_max_limit;

        int32_t current =
            lv_slider_get_value(
                objects.voltage_maximum
            );

        if (value != current)
        {
            tick_value_change_obj =
                objects.voltage_maximum;

            lv_slider_set_value(
                objects.voltage_maximum,
                value,
                LV_ANIM_OFF
            );

            tick_value_change_obj =
                NULL;
        }
    }

    // --------------------------------------------------
    // Synchronize current minimum slider
    // --------------------------------------------------

    if (objects.current_minimum != NULL)
    {
        int32_t value =
            (int32_t)current_min_limit;

        int32_t current =
            lv_slider_get_value(
                objects.current_minimum
            );

        if (value != current)
        {
            tick_value_change_obj =
                objects.current_minimum;

            lv_slider_set_value(
                objects.current_minimum,
                value,
                LV_ANIM_OFF
            );

            tick_value_change_obj =
                NULL;
        }
    }

    // --------------------------------------------------
    // Synchronize current maximum slider
    // --------------------------------------------------

    if (objects.current_maximum != NULL)
    {
        int32_t value =
            (int32_t)current_max_limit;

        int32_t current =
            lv_slider_get_value(
                objects.current_maximum
            );

        if (value != current)
        {
            tick_value_change_obj =
                objects.current_maximum;

            lv_slider_set_value(
                objects.current_maximum,
                value,
                LV_ANIM_OFF
            );

            tick_value_change_obj =
                NULL;
        }
    }
}

// ==================================================
// Update Buzzer Screen
// ==================================================

static void update_buzzer_gui(void)
{
    if (!buzzer_screen_active())
    {
        return;
    }

    if (buzzer_dropdown == NULL)
    {
        buzzer_dropdown_find();
    }

    if (buzzer_dropdown == NULL)
    {
        return;
    }

    if (!buzzer_dropdown_open)
    {
        uint16_t selected =
            lv_dropdown_get_selected(
                buzzer_dropdown
            );

        uint8_t mode =
            buzzer_get_mode();

        if (selected != mode)
        {
            lv_dropdown_set_selected(
                buzzer_dropdown,
                mode
            );
        }
    }
}

// ==================================================
// LED Output
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
// Set Solid LED
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
// Set Blinking LED
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
// LED Task
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
// Settings Focus
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

        lv_obj_set_style_border_color(
            selected,
            lv_color_hex(
                FOCUS_COLOR
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

// ==================================================
// Buzzer Focus
// ==================================================

static void clear_buzzer_focus(void)
{
    buzzer_dropdown_find();

    if (buzzer_dropdown != NULL)
    {
        lv_obj_set_style_border_width(
            buzzer_dropdown,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (
        objects.buzzer_settings_page_back_button != NULL
    )
    {
        lv_obj_set_style_border_width(
            objects.buzzer_settings_page_back_button,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

static void apply_buzzer_focus(void)
{
    if (!buzzer_screen_active())
    {
        return;
    }

    clear_buzzer_focus();

    buzzer_dropdown_find();

    if (!buzzer_focus_back)
    {
        if (buzzer_dropdown != NULL)
        {
            lv_obj_set_style_border_width(
                buzzer_dropdown,
                buzzer_dropdown_open ? 3 : 2,
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            lv_obj_set_style_border_color(
                buzzer_dropdown,
                lv_color_hex(
                    FOCUS_COLOR
                ),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );
        }
    }
    else
    {
        if (
            objects.buzzer_settings_page_back_button != NULL
        )
        {
            lv_obj_set_style_border_width(
                objects.buzzer_settings_page_back_button,
                2,
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );

            lv_obj_set_style_border_color(
                objects.buzzer_settings_page_back_button,
                lv_color_hex(
                    FOCUS_COLOR
                ),
                LV_PART_MAIN |
                LV_STATE_DEFAULT
            );
        }
    }
}

// ==================================================
// V/C Focus
// ==================================================

static lv_obj_t *get_vc_back_button(void)
{
    if (
        objects.exit_from_v_c_menu_button == NULL
    )
    {
        return NULL;
    }

    return lv_obj_get_parent(
        objects.exit_from_v_c_menu_button
    );
}

static void clear_vc_focus(void)
{
    if (objects.voltage_minimum != NULL)
    {
        lv_obj_set_style_border_width(
            objects.voltage_minimum,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.voltage_maximum != NULL)
    {
        lv_obj_set_style_border_width(
            objects.voltage_maximum,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.current_minimum != NULL)
    {
        lv_obj_set_style_border_width(
            objects.current_minimum,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    if (objects.current_maximum != NULL)
    {
        lv_obj_set_style_border_width(
            objects.current_maximum,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }

    lv_obj_t *back =
        get_vc_back_button();

    if (back != NULL)
    {
        lv_obj_set_style_border_width(
            back,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

static void apply_vc_focus(void)
{
    if (!vc_range_screen_active())
    {
        return;
    }

    clear_vc_focus();

    lv_obj_t *selected =
        NULL;

    switch (vc_focus)
    {
        case 0:
            selected =
                objects.voltage_minimum;
            break;

        case 1:
            selected =
                objects.voltage_maximum;
            break;

        case 2:
            selected =
                objects.current_minimum;
            break;

        case 3:
            selected =
                objects.current_maximum;
            break;

        case 4:
            selected =
                get_vc_back_button();
            break;

        default:
            break;
    }

    if (selected != NULL)
    {
        lv_obj_set_style_border_width(
            selected,
            vc_edit_mode ? 3 : 2,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_border_color(
            selected,
            lv_color_hex(
                FOCUS_COLOR
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );
    }
}

// ==================================================
// Update Input Focus
// ==================================================

static void update_input_focus_gui(void)
{
    enum ScreensEnum current_screen =
        screen_manager_get();

    if (
        current_screen !=
        last_focus_screen
    )
    {
        clear_buzzer_focus();
        clear_vc_focus();

        if (
            current_screen ==
            SCREEN_ID_BUZZER_SETTINGS
        )
        {
            buzzer_focus_back =
                false;

            buzzer_dropdown_open =
                false;

            buzzer_dropdown_find();

            if (buzzer_dropdown != NULL)
            {
                lv_dropdown_close(
                    buzzer_dropdown
                );
            }

            apply_buzzer_focus();
        }

        if (
            current_screen ==
            SCREEN_ID_V_C_RANGE_SETTINGS
        )
        {
            vc_focus =
                0;

            vc_edit_mode =
                false;

            apply_vc_focus();
        }

        last_focus_screen =
            current_screen;

        last_buzzer_focus_state =
            -1;

        last_vc_focus_state =
            -1;

        last_vc_edit_state =
            false;
    }

    if (
        current_screen ==
        SCREEN_ID_BUZZER_SETTINGS
    )
    {
        int focus_state =
            buzzer_focus_back ? 1 : 0;

        if (
            focus_state !=
            last_buzzer_focus_state
        )
        {
            apply_buzzer_focus();

            last_buzzer_focus_state =
                focus_state;
        }

        if (buzzer_dropdown_open)
        {
            apply_buzzer_focus();
        }
    }

    if (
        current_screen ==
        SCREEN_ID_V_C_RANGE_SETTINGS
    )
    {
        if (
            vc_focus !=
            last_vc_focus_state ||
            vc_edit_mode !=
            last_vc_edit_state
        )
        {
            apply_vc_focus();

            last_vc_focus_state =
                vc_focus;

            last_vc_edit_state =
                vc_edit_mode;
        }
    }
}

// ==================================================
// Buzzer Dropdown Navigation
// ==================================================

static void buzzer_dropdown_change(
    int direction
)
{
    buzzer_dropdown_find();

    if (buzzer_dropdown == NULL)
    {
        return;
    }

    uint16_t selected =
        lv_dropdown_get_selected(
            buzzer_dropdown
        );

    if (direction > 0)
    {
        if (selected < 2)
        {
            selected++;
        }
        else
        {
            selected = 0;
        }
    }
    else
    {
        if (selected > 0)
        {
            selected--;
        }
        else
        {
            selected = 2;
        }
    }

    lv_dropdown_set_selected(
        buzzer_dropdown,
        selected
    );

    set_var_buzzer_mode(
        (int32_t)selected
    );
}

// ==================================================
// V/C Value Change
// ==================================================

static void vc_change_value(void)
{
    if (!vc_range_screen_active())
    {
        return;
    }

    // --------------------------------------------------
    // Voltage minimum
    // --------------------------------------------------

    if (vc_focus == 0)
    {
        float value =
            get_voltage_min_limit();

        value += 1.0f;

        if (
            value >
            VOLTAGE_LIMIT_MAX
        )
        {
            value =
                VOLTAGE_LIMIT_MIN;
        }

        set_voltage_min_limit(
            value
        );
    }

    // --------------------------------------------------
    // Voltage maximum
    // --------------------------------------------------

    else if (vc_focus == 1)
    {
        float value =
            get_voltage_max_limit();

        value += 1.0f;

        if (
            value >
            VOLTAGE_LIMIT_MAX
        )
        {
            value =
                VOLTAGE_LIMIT_MIN;
        }

        set_voltage_max_limit(
            value
        );
    }

    // --------------------------------------------------
    // Current minimum
    // --------------------------------------------------

    else if (vc_focus == 2)
    {
        float value =
            get_current_min_limit();

        value += 1.0f;

        if (
            value >
            CURRENT_LIMIT_MAX
        )
        {
            value =
                CURRENT_LIMIT_MIN;
        }

        set_current_min_limit(
            value
        );
    }

    // --------------------------------------------------
    // Current maximum
    // --------------------------------------------------

    else if (vc_focus == 3)
    {
        float value =
            get_current_max_limit();

        value += 1.0f;

        if (
            value >
            CURRENT_LIMIT_MAX
        )
        {
            value =
                CURRENT_LIMIT_MIN;
        }

        set_current_max_limit(
            value
        );
    }

    update_vc_range_gui();

    apply_vc_focus();
}

// ==================================================
// Handle RIGHT Button Release
// ==================================================

static void handle_right_release(void)
{
    enum ScreensEnum screen =
        screen_manager_get();

    // --------------------------------------------------
    // Main screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_MAIN
    )
    {
        main_selection =
            MAIN_OPTION_SETTINGS;

        return;
    }

    // --------------------------------------------------
    // Settings screen
    // --------------------------------------------------

    if (
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

        return;
    }

    // --------------------------------------------------
    // Buzzer screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_BUZZER_SETTINGS
    )
    {
        if (buzzer_dropdown_open)
        {
            buzzer_dropdown_change(
                1
            );

            return;
        }

        if (!buzzer_focus_back)
        {
            buzzer_focus_back =
                true;
        }
        else
        {
            buzzer_focus_back =
                false;
        }

        apply_buzzer_focus();

        return;
    }

    // --------------------------------------------------
    // V/C screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_V_C_RANGE_SETTINGS
    )
    {
        if (vc_edit_mode)
        {
            vc_change_value();

            return;
        }

        vc_focus++;

        if (vc_focus > 4)
        {
            vc_focus =
                0;
        }

        apply_vc_focus();

        return;
    }
}

// ==================================================
// Handle SELECT Button Release
// ==================================================

static void handle_select_release(void)
{
    enum ScreensEnum screen =
        screen_manager_get();

    // --------------------------------------------------
    // Main screen
    // --------------------------------------------------

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

        return;
    }

    // --------------------------------------------------
    // Settings screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_SETTINGS_PAGE
    )
    {
        switch (settings_selection)
        {
            case SETTINGS_OPTION_BUZZER:

                action_go_to_buzzer_settings(
                    NULL
                );

                break;

            case SETTINGS_OPTION_CALIBRATION:

                action_go_to_touch_calibration(
                    NULL
                );

                break;

            case SETTINGS_OPTION_VC_RANGE:

                action_go_to_v_c_range_settings(
                    NULL
                );

                break;

            case SETTINGS_OPTION_BACK:

                action_exit_to_main_page(
                    NULL
                );

                break;

            default:

                break;
        }

        return;
    }

    // --------------------------------------------------
    // Buzzer screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_BUZZER_SETTINGS
    )
    {
        buzzer_dropdown_find();

        if (!buzzer_focus_back)
        {
            if (buzzer_dropdown == NULL)
            {
                return;
            }

            if (!buzzer_dropdown_open)
            {
                buzzer_dropdown_open =
                    true;

                lv_dropdown_open(
                    buzzer_dropdown
                );

                apply_buzzer_focus();
            }
            else
            {
                buzzer_dropdown_open =
                    false;

                lv_dropdown_close(
                    buzzer_dropdown
                );

                set_var_buzzer_mode(
                    (int32_t)
                    lv_dropdown_get_selected(
                        buzzer_dropdown
                    )
                );

                apply_buzzer_focus();
            }

            return;
        }

        action_go_from_buzzer_settings_page_to_settings_page(
            NULL
        );

        return;
    }

    // --------------------------------------------------
    // V/C screen
    // --------------------------------------------------

    if (
        screen ==
        SCREEN_ID_V_C_RANGE_SETTINGS
    )
    {
        if (vc_focus == 4)
        {
            if (!vc_edit_mode)
            {
                action_exit_from_v_c_menu_to_settings(
                    NULL
                );
            }

            return;
        }

        vc_edit_mode =
            !vc_edit_mode;

        apply_vc_focus();

        return;
    }
}

// ==================================================
// Button Task
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

    // --------------------------------------------------
    // RIGHT button
    // --------------------------------------------------

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

            // Only act on release
            if (
                right_state == HIGH
            )
            {
                handle_right_release();
            }
        }
    }

    // --------------------------------------------------
    // SELECT button
    // --------------------------------------------------

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

            // Only act on release
            if (
                select_state == HIGH
            )
            {
                handle_select_release();
            }
        }
    }
}

// ==================================================
// UART Task
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
// Safety Task
// ==================================================

static void safety_task(void)
{
    uint32_t now =
        millis();

    // --------------------------------------------------
    // UART connection state
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Voltage validation
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Current validation
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Low voltage detection
    // --------------------------------------------------

    system_state.low_voltage =
        (
            system_state.voltage <
            voltage_min_limit
        );

    // --------------------------------------------------
    // Overall system state
    // --------------------------------------------------

    system_state.system_ok =
        system_state.data_received &&
        !system_state.connection_lost &&
        system_state.voltage_ok &&
        system_state.current_ok;
}

// ==================================================
// Buzzer Task
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

    // --------------------------------------------------
    // Mode 1
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Mode 2
    // --------------------------------------------------

    else if (
        buzzer_mode ==
        BUZZER_MODE_2
    )
    {
        interval =
            100;
    }

    // --------------------------------------------------
    // Mode 3
    // --------------------------------------------------

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
// Determine Current Error
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
// Error Message Box Update
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

    // --------------------------------------------------
    // No error
    // --------------------------------------------------

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
            lv_obj_add_flag(
                objects.error_box,
                LV_OBJ_FLAG_HIDDEN
            );

            set_error_text(
                ""
            );

            gui_last_error =
                ERROR_NONE;

            error_text_blink_state =
                true;

            error_text_blink_timer =
                millis();
        }

        return;
    }

    // --------------------------------------------------
    // Ignore unchanged error
    // --------------------------------------------------

    if (
        error ==
        gui_last_error
    )
    {
        return;
    }

    const char *message =
        "";

    uint32_t color =
        LED_RED;

    // --------------------------------------------------
    // Select error message and box color
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Update error text
    // --------------------------------------------------

    set_error_text(
        message
    );

    lv_obj_set_style_text_color(
        error_msg_label,
        lv_color_hex(
            0xFFFFFF
        ),
        LV_PART_MAIN |
        LV_STATE_DEFAULT
    );

    // --------------------------------------------------
    // Update box color only when necessary
    // --------------------------------------------------

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
            LV_OPA_60,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        error_box_last_color =
            color;
    }

    // --------------------------------------------------
    // Show error box
    // --------------------------------------------------

    lv_obj_clear_flag(
        objects.error_box,
        LV_OBJ_FLAG_HIDDEN
    );

    lv_obj_clear_flag(
        error_msg_label,
        LV_OBJ_FLAG_HIDDEN
    );

    if (
        objects.error_label != NULL
    )
    {
        lv_obj_add_flag(
            objects.error_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    // --------------------------------------------------
    // Restart text blinking
    // --------------------------------------------------

    error_text_blink_timer =
        millis();

    error_text_blink_state =
        true;

    gui_last_error =
        error;
}

// ==================================================
// GUI Update
// ==================================================

static void gui_update(void)
{
    // --------------------------------------------------
    // Main screen data
    // --------------------------------------------------

    if (main_screen_active())
    {
        // Voltage display
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

        // Current display
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

        update_error_box();
        update_error_text_blink();
    }

    // --------------------------------------------------
    // Settings screens
    // --------------------------------------------------

    update_vc_range_gui();
    update_buzzer_gui();
    update_input_focus_gui();
}

// ==================================================
// GUI Task
// ==================================================

static void gui_task(void)
{
    static uint32_t last_gui_update =
        0;

    uint32_t now =
        millis();

    if (
        now - last_gui_update <
        20
    )
    {
        return;
    }

    last_gui_update =
        now;

    gui_update();
    update_led_state();

    // Keep EEZ screen tick synchronized with
    // the screen manager.
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
// Update LED State
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

    // Connection lost
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

    // No valid data
    if (
        !system_state.data_received
    )
    {
        set_status_led(
            LED_BLUE
        );

        return;
    }

    // System is healthy
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

    // Voltage or current fault
    set_status_led_blink(
        LED_RED
    );
}

// ==================================================
// Task System Initialization
// ==================================================

void tasks_init(void)
{
    // --------------------------------------------------
    // Initialize hardware inputs
    // --------------------------------------------------

    pinMode(
        BTN_RIGHT,
        INPUT_PULLUP
    );

    pinMode(
        BTN_SELECT,
        INPUT_PULLUP
    );

    // --------------------------------------------------
    // Initialize buzzer
    // --------------------------------------------------

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    noTone(
        BUZZER_PIN
    );

    // --------------------------------------------------
    // Read initial button states
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Initialize timers
    // --------------------------------------------------

    buzzer_timer =
        millis();

    led_blink_timer =
        millis();

    error_text_blink_timer =
        millis();

    error_text_blink_state =
        true;

    // --------------------------------------------------
    // Initialize buzzer dropdown
    // --------------------------------------------------

    buzzer_dropdown_find();

    if (buzzer_dropdown != NULL)
    {
        lv_dropdown_set_selected(
            buzzer_dropdown,
            buzzer_get_mode()
        );
    }

    // --------------------------------------------------
    // Initialize status LED
    // --------------------------------------------------

    if (objects.obj0 != NULL)
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
    }

    // --------------------------------------------------
    // Initialize error message box
    // --------------------------------------------------

    if (objects.error_box != NULL)
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
            LV_OPA_60,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        // ------------------------------------------------
        // Small shadow under Error Box
        // ------------------------------------------------

        lv_obj_set_style_shadow_width(
            objects.error_box,
            6,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_shadow_color(
            objects.error_box,
            lv_color_hex(
                0x000000
            ),
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_shadow_opa(
            objects.error_box,
            LV_OPA_40,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_shadow_ofs_x(
            objects.error_box,
            0,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        lv_obj_set_style_shadow_ofs_y(
            objects.error_box,
            3,
            LV_PART_MAIN |
            LV_STATE_DEFAULT
        );

        error_box_last_color =
            LED_RED;

        init_error_msgbox();
    }

    // --------------------------------------------------
    // Hide unused EEZ error label
    // --------------------------------------------------

    if (objects.error_label != NULL)
    {
        lv_obj_add_flag(
            objects.error_label,
            LV_OBJ_FLAG_HIDDEN
        );
    }

    // --------------------------------------------------
    // Initialize menu focus
    // --------------------------------------------------

    apply_settings_highlight();

    buzzer_focus_back =
        false;

    buzzer_dropdown_open =
        false;

    vc_focus =
        0;

    vc_edit_mode =
        false;

    // --------------------------------------------------
    // Reset GUI caches
    // --------------------------------------------------

    gui_last_voltage_min =
        -1000.0f;

    gui_last_voltage_max =
        -1000.0f;

    gui_last_current_min =
        -1000.0f;

    gui_last_current_max =
        -1000.0f;

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

    last_focus_screen =
        SCREEN_ID_MAIN;

    last_buzzer_focus_state =
        -1;

    last_vc_focus_state =
        -1;

    last_vc_edit_state =
        false;

    led_last_state =
        ERROR_NONE;
}

// ==================================================
// Task Scheduler
// ==================================================

void tasks_run(void)
{
    // Read physical buttons
    buttons_task();

    // Receive UART data
    uart_task();

    // Validate system conditions
    safety_task();

    // Control buzzer output
    buzzer_task();

    // Apply pending screen changes
    screen_manager_process();

    // Update user interface
    gui_task();

    // Update status LED
    led_task();
}

