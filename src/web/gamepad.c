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

#ifndef HAL_NO_GAMEPAD
#include "gamepad_common.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string.h>

typedef struct {
    int browser_index;
    char *id;
} hal_gamepad_private_t;

static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;
static bool inited = false;

static hal_gamepad_device_t *find_device_by_browser_index(int browser_index) {
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
        if (priv->browser_index == browser_index)
            return devices[i];
    }
    return NULL;
}

static void add_device(int browser_index, const char *id, int num_buttons, int num_axes) {
    if (find_device_by_browser_index(browser_index) != NULL)
        return;
    
    hal_gamepad_device_t *device = malloc(sizeof(hal_gamepad_device_t));
    device->device_id = next_device_id++;
    device->vendor_id = 0;
    device->product_id = 0;
    device->num_axes = num_axes;
    device->num_buttons = num_buttons;
    device->axis_states = calloc(sizeof(float), device->num_axes);
    device->button_states = calloc(sizeof(bool), device->num_buttons);
    
    hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
    priv->browser_index = browser_index;
    priv->id = strdup(id);
    device->description = priv->id;
    device->private_data = priv;
    
    devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
    devices[num_devices++] = device;
    
    if (hal_gamepad_attach_cb)
        hal_gamepad_attach_cb(device, hal_gamepad_attach_ctx);
}

static void remove_device(int browser_index) {
    hal_gamepad_device_t *device = find_device_by_browser_index(browser_index);
    if (device == NULL)
        return;
    
    if (hal_gamepad_remove_cb)
        hal_gamepad_remove_cb(device, hal_gamepad_remove_ctx);
    
    for (unsigned int i = 0; i < num_devices; i++) {
        if (devices[i] == device) {
            num_devices--;
            for (unsigned int j = i; j < num_devices; j++)
                devices[j] = devices[j + 1];
            break;
        }
    }
    
    hal_gamepad_private_t *priv = device->private_data;
    free(priv->id);
    free(priv);
    free(device->axis_states);
    free(device->button_states);
    free(device);
}

EM_BOOL gamepad_connected_callback(int eventType, const EmscriptenGamepadEvent *e, void *userData) {
    (void)eventType;
    (void)userData;
    add_device(e->index, e->id, e->numButtons, e->numAxes);
    return EM_TRUE;
}

EM_BOOL gamepad_disconnected_callback(int eventType, const EmscriptenGamepadEvent *e, void *userData) {
    (void)eventType;
    (void)userData;
    remove_device(e->index);
    return EM_TRUE;
}

bool hal_gamepad_available(void) {
    return emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS;
}

void hal_gamepad_init(void) {
    if (inited)
        return;
    
    emscripten_set_gamepadconnected_callback(NULL, EM_TRUE, gamepad_connected_callback);
    emscripten_set_gamepaddisconnected_callback(NULL, EM_TRUE, gamepad_disconnected_callback);
    
    inited = true;
    hal_gamepad_detect_devices();
}

void hal_gamepad_shutdown(void) {
    if (!inited)
        return;
    
    emscripten_set_gamepadconnected_callback(NULL, EM_FALSE, NULL);
    emscripten_set_gamepaddisconnected_callback(NULL, EM_FALSE, NULL);
    
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
        free(priv->id);
        free(priv);
        free(devices[i]->axis_states);
        free(devices[i]->button_states);
        free(devices[i]);
    }
    free(devices);
    devices = NULL;
    num_devices = 0;
    
    inited = false;
}

unsigned int hal_gamepad_num_devices(void) {
    return num_devices;
}

hal_gamepad_device_t *hal_gamepad_device_at(unsigned int index) {
    if (index >= num_devices)
        return NULL;
    return devices[index];
}

void hal_gamepad_detect_devices(void) {
    if (!inited)
        return;
    
    if (emscripten_sample_gamepad_data() != EMSCRIPTEN_RESULT_SUCCESS)
        return;
    
    int count = emscripten_get_num_gamepads();
    for (int i = 0; i < count; i++) {
        EmscriptenGamepadEvent state;
        if (emscripten_get_gamepad_status(i, &state) == EMSCRIPTEN_RESULT_SUCCESS) {
            if (state.connected && find_device_by_browser_index(i) == NULL)
                add_device(i, state.id, state.numButtons, state.numAxes);
        }
    }
}

void hal_gamepad_process_events(void) {
    if (!inited)
        return;
    
    if (emscripten_sample_gamepad_data() != EMSCRIPTEN_RESULT_SUCCESS)
        return;
    
    double timestamp = emscripten_get_now() / 1000.0;
    
    for (unsigned int d = 0; d < num_devices; d++) {
        hal_gamepad_device_t *device = devices[d];
        hal_gamepad_private_t *priv = device->private_data;
        
        EmscriptenGamepadEvent state;
        if (emscripten_get_gamepad_status(priv->browser_index, &state) != EMSCRIPTEN_RESULT_SUCCESS)
            continue;
        
        if (!state.connected) {
            remove_device(priv->browser_index);
            d--;
            continue;
        }
        
        /* Process axes */
        for (int a = 0; a < state.numAxes && a < (int)device->num_axes; a++) {
            float value = (float)state.axis[a];
            if (value != device->axis_states[a]) {
                if (hal_gamepad_axis_cb)
                    hal_gamepad_axis_cb(device, a, value, device->axis_states[a], timestamp, hal_gamepad_axis_ctx);
                device->axis_states[a] = value;
            }
        }
        
        /* Process buttons */
        for (int b = 0; b < state.numButtons && b < (int)device->num_buttons; b++) {
            bool pressed = state.digitalButton[b];
            if (pressed != device->button_states[b]) {
                if (pressed && hal_gamepad_button_down_cb)
                    hal_gamepad_button_down_cb(device, b, timestamp, hal_gamepad_button_down_ctx);
                else if (!pressed && hal_gamepad_button_up_cb)
                    hal_gamepad_button_up_cb(device, b, timestamp, hal_gamepad_button_up_ctx);
                device->button_states[b] = pressed;
            }
        }
    }
}

#endif /* HAL_NO_GAMEPAD */
