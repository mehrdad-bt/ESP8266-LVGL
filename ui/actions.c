#include "actions.h"
#include "screens.h"
#include "ui.h"

void action_next_page(lv_event_t * e) {
    // Switch to page 2 when the button is pressed
    loadScreen(SCREEN_ID_PAGE_2);
}

void action_prev_page(lv_event_t * e) {
    // Switch to page 2 when the button is pressed
    loadScreen(SCREEN_ID_MAIN);
}