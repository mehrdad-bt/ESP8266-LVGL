#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void tasks_init(void);
void tasks_run(void);

// ==================================================
// BUZZER API
// ==================================================

void buzzer_set_mode(
    uint8_t mode
);

uint8_t buzzer_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif