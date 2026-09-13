
#include <stdio.h>
#include <string.h>

#include "vars.h"
#include "tasks.h"

// ==================================================
// TEXT BUFFERS
// ==================================================

static char voltage_min_text_value[16] = "20.0";
static char voltage_max_text_value[16] = "25.0";
static char current_min_text_value[16] = "0.0";
static char current_max_text_value[16] = "1.0";


// ==================================================
// NUMERIC VARIABLES
// ==================================================

float get_var_voltage_min(void)
{
    return get_voltage_min_limit();
}

void set_var_voltage_min(float value)
{
    set_voltage_min_limit(value);
}


float get_var_voltage_max(void)
{
    return get_voltage_max_limit();
}

void set_var_voltage_max(float value)
{
    set_voltage_max_limit(value);
}


float get_var_current_min(void)
{
    return get_current_min_limit();
}

void set_var_current_min(float value)
{
    set_current_min_limit(value);
}


float get_var_current_max(void)
{
    return get_current_max_limit();
}

void set_var_current_max(float value)
{
    set_current_max_limit(value);
}


// ==================================================
// TEXT VARIABLES
// ==================================================

// Voltage MIN
const char *get_var_voltage_min_text(void)
{
    snprintf(
        voltage_min_text_value,
        sizeof(voltage_min_text_value),
        "%.1f",
        get_voltage_min_limit()
    );

    return voltage_min_text_value;
}

void set_var_voltage_min_text(const char *value)
{
    if (value == NULL)
        return;

    strncpy(
        voltage_min_text_value,
        value,
        sizeof(voltage_min_text_value) - 1
    );

    voltage_min_text_value[
        sizeof(voltage_min_text_value) - 1
    ] = '\0';
}


// Voltage MAX
const char *get_var_voltage_max_text(void)
{
    snprintf(
        voltage_max_text_value,
        sizeof(voltage_max_text_value),
        "%.1f",
        get_voltage_max_limit()
    );

    return voltage_max_text_value;
}

void set_var_voltage_max_text(const char *value)
{
    if (value == NULL)
        return;

    strncpy(
        voltage_max_text_value,
        value,
        sizeof(voltage_max_text_value) - 1
    );

    voltage_max_text_value[
        sizeof(voltage_max_text_value) - 1
    ] = '\0';
}


// Current MIN
const char *get_var_current_min_text(void)
{
    snprintf(
        current_min_text_value,
        sizeof(current_min_text_value),
        "%.1f",
        get_current_min_limit()
    );

    return current_min_text_value;
}

void set_var_current_min_text(const char *value)
{
    if (value == NULL)
        return;

    strncpy(
        current_min_text_value,
        value,
        sizeof(current_min_text_value) - 1
    );

    current_min_text_value[
        sizeof(current_min_text_value) - 1
    ] = '\0';
}


// Current MAX
const char *get_var_current_max_text(void)
{
    snprintf(
        current_max_text_value,
        sizeof(current_max_text_value),
        "%.1f",
        get_current_max_limit()
    );

    return current_max_text_value;
}

void set_var_current_max_text(const char *value)
{
    if (value == NULL)
        return;

    strncpy(
        current_max_text_value,
        value,
        sizeof(current_max_text_value) - 1
    );

    current_max_text_value[
        sizeof(current_max_text_value) - 1
    ] = '\0';
}

