/* https://github.com/takeiteasy/hal

 hal Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

/* Shared callback storage and registration - included by platform implementations */

#ifndef HAL_GAMEPAD_COMMON_H
#define HAL_GAMEPAD_COMMON_H

#include "hal/gamepad.h"
#include <stdlib.h>

/* Callback storage */
static hal_gamepad_attach_callback_t hal_gamepad_attach_cb = NULL;
static hal_gamepad_remove_callback_t hal_gamepad_remove_cb = NULL;
static hal_gamepad_button_callback_t hal_gamepad_button_down_cb = NULL;
static hal_gamepad_button_callback_t hal_gamepad_button_up_cb = NULL;
static hal_gamepad_axis_callback_t hal_gamepad_axis_cb = NULL;

static void *hal_gamepad_attach_ctx = NULL;
static void *hal_gamepad_remove_ctx = NULL;
static void *hal_gamepad_button_down_ctx = NULL;
static void *hal_gamepad_button_up_ctx = NULL;
static void *hal_gamepad_axis_ctx = NULL;

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

/* Event types for queued events */
typedef enum {
    HAL_GAMEPAD_EVENT_ATTACHED,
    HAL_GAMEPAD_EVENT_REMOVED,
    HAL_GAMEPAD_EVENT_BUTTON_DOWN,
    HAL_GAMEPAD_EVENT_BUTTON_UP,
    HAL_GAMEPAD_EVENT_AXIS_MOVED
} hal_gamepad_event_type_t;

/* Button event data */
typedef struct {
    hal_gamepad_device_t *device;
    double timestamp;
    unsigned int button_id;
    bool down;
} hal_gamepad_button_event_t;

/* Axis event data */
typedef struct {
    hal_gamepad_device_t *device;
    double timestamp;
    unsigned int axis_id;
    float value;
    float last_value;
} hal_gamepad_axis_event_t;

/* Queued event */
typedef struct {
    unsigned int device_id;
    hal_gamepad_event_type_t event_type;
    void *event_data;
} hal_gamepad_queued_event_t;

#endif /* HAL_GAMEPAD_COMMON_H */
