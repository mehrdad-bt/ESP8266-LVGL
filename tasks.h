#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void tasks_init(void);
void tasks_run(void);

void buzzer_set_mode(uint8_t mode);
uint8_t buzzer_get_mode(void);

/* ==========================================
 * VOLTAGE LIMITS
 * ========================================== */

void set_voltage_min_limit(float value);
void set_voltage_max_limit(float value);

float get_voltage_min_limit(void);
float get_voltage_max_limit(void);

/* ==========================================
 * CURRENT LIMITS
 * ========================================== */

void set_current_min_limit(float value);
void set_current_max_limit(float value);

float get_current_min_limit(void);
float get_current_max_limit(void);

#ifdef __cplusplus
}
#endif

#endif