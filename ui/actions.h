#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_go_to_settings_page(lv_event_t * e);
extern void action_exit_to_main_page(lv_event_t * e);
extern void action_go_to_buzzer_settings(lv_event_t * e);
extern void action_go_to_touch_calibration(lv_event_t * e);
extern void action_go_to_v_c_range_settings(lv_event_t * e);
extern void action_go_from_buzzer_settings_page_to_settings_page(lv_event_t * e);
extern void action_voltage_min_changed(lv_event_t * e);
extern void action_voltage_max_changed(lv_event_t * e);
extern void action_current_min_changed(lv_event_t * e);
extern void action_current_max_changed(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/