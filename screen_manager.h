#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "ui/screens.h"

#ifdef __cplusplus
extern "C" {
#endif


void screen_manager_init(void);

void screen_manager_show(
    enum ScreensEnum screen
);

enum ScreensEnum screen_manager_get(void);

bool screen_manager_is(
    enum ScreensEnum screen
);


#ifdef __cplusplus
}
#endif

#endif