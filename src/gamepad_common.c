/* https://github.com/takeiteasy/hal */

#include "gamepad_common.h"
#include <stddef.h>

/* Callback storage definitions */
hal_gamepad_attach_callback_t hal_gamepad_attach_cb = NULL;
hal_gamepad_remove_callback_t hal_gamepad_remove_cb = NULL;
hal_gamepad_button_callback_t hal_gamepad_button_down_cb = NULL;
hal_gamepad_button_callback_t hal_gamepad_button_up_cb = NULL;
hal_gamepad_axis_callback_t hal_gamepad_axis_cb = NULL;

void *hal_gamepad_attach_ctx = NULL;
void *hal_gamepad_remove_ctx = NULL;
void *hal_gamepad_button_down_ctx = NULL;
void *hal_gamepad_button_up_ctx = NULL;
void *hal_gamepad_axis_ctx = NULL;

/* Callback registration implementations */
void hal_gamepad_set_attach_callback(hal_gamepad_attach_callback_t callback, void *context) {
    hal_gamepad_attach_cb = callback;
    hal_gamepad_attach_ctx = context;
}

void hal_gamepad_set_remove_callback(hal_gamepad_remove_callback_t callback, void *context) {
    hal_gamepad_remove_cb = callback;
    hal_gamepad_remove_ctx = context;
}

void hal_gamepad_set_button_down_callback(hal_gamepad_button_callback_t callback, void *context) {
    hal_gamepad_button_down_cb = callback;
    hal_gamepad_button_down_ctx = context;
}

void hal_gamepad_set_button_up_callback(hal_gamepad_button_callback_t callback, void *context) {
    hal_gamepad_button_up_cb = callback;
    hal_gamepad_button_up_ctx = context;
}

void hal_gamepad_set_axis_callback(hal_gamepad_axis_callback_t callback, void *context) {
    hal_gamepad_axis_cb = callback;
    hal_gamepad_axis_ctx = context;
}
