#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// ==================================================
// INIT
// ==================================================

void tasks_init(void);


// ==================================================
// RUN
// ==================================================

void tasks_run(void);


// ==================================================
// BUZZER
// ==================================================

void buzzer_set_mode(
    uint8_t mode
);

uint8_t buzzer_get_mode(void);


#ifdef __cplusplus
}
#endif

#endif