#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus

extern "C" {

#endif


#include "ui/screens.h"


// ==================================================
// INITIALIZE
// ==================================================

void screen_manager_init(
    void
);


// ==================================================
// SHOW SCREEN
// ==================================================

void screen_manager_show(
    enum ScreensEnum screen
);


// ==================================================
// RELOAD CURRENT SCREEN
// ==================================================

void screen_manager_reload(
    void
);


// ==================================================
// GET CURRENT SCREEN
// ==================================================

enum ScreensEnum screen_manager_get(
    void
);


// ==================================================
// CHECK CURRENT SCREEN
// ==================================================

bool screen_manager_is(
    enum ScreensEnum screen
);


#ifdef __cplusplus

}

#endif

#endif