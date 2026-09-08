#include "actions.h"
#include "screens.h"
#include "../screen_manager.h"


void action_go_to_settings_page(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_SETTINGS
    );
}


void action_go_to_main_screen(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_MAIN
    );
}


void action_go_to_buzzer_settings(
    lv_event_t *e
)
{
    (void)e;

    screen_manager_show(
        SCREEN_ID_BUZZER
    );
}