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
#include <android/input.h>
#include <android/looper.h>
#include <android/log.h>
#include <string.h>

#define LOG_TAG "hal_gamepad"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef struct {
    int32_t device_id;
} hal_gamepad_private_t;

static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;
static bool inited = false;

static hal_gamepad_device_t *find_device_by_android_id(int32_t android_device_id) {
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
        if (priv->device_id == android_device_id)
            return devices[i];
    }
    return NULL;
}

static void add_device(int32_t android_device_id) {
    if (find_device_by_android_id(android_device_id) != NULL)
        return;
    
    hal_gamepad_device_t *device = malloc(sizeof(hal_gamepad_device_t));
    device->device_id = next_device_id++;
    device->description = "Android Controller";
    device->vendor_id = 0;
    device->product_id = 0;
    device->num_axes = 6;     /* 2 sticks + 2 triggers */
    device->num_buttons = 16;
    device->axis_states = calloc(sizeof(float), device->num_axes);
    device->button_states = calloc(sizeof(bool), device->num_buttons);
    
    hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
    priv->device_id = android_device_id;
    device->private_data = priv;
    
    devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
    devices[num_devices++] = device;
    
    if (hal_gamepad_attach_cb)
        hal_gamepad_attach_cb(device, hal_gamepad_attach_ctx);
}

static void remove_device(hal_gamepad_device_t *device) {
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
    free(priv);
    free(device->axis_states);
    free(device->button_states);
    free(device);
}

bool hal_gamepad_available(void) {
    return true;
}

void hal_gamepad_init(void) {
    if (inited)
        return;
    
    inited = true;
    LOGI("Gamepad subsystem initialized");
}

void hal_gamepad_shutdown(void) {
    if (!inited)
        return;
    
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
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
    /* Device detection on Android is handled via the Java layer
       using InputManager.registerInputDeviceListener() */
}

/* Called from Java via JNI when a gamepad is connected */
void hal_gamepad_on_device_added(int32_t device_id) {
    if (inited)
        add_device(device_id);
}

/* Called from Java via JNI when a gamepad is disconnected */
void hal_gamepad_on_device_removed(int32_t device_id) {
    if (!inited)
        return;
    
    hal_gamepad_device_t *device = find_device_by_android_id(device_id);
    if (device)
        remove_device(device);
}

/* Called from Java via JNI when an axis event occurs */
void hal_gamepad_on_axis_event(int32_t device_id, int axis_id, float value) {
    if (!inited)
        return;
    
    hal_gamepad_device_t *device = find_device_by_android_id(device_id);
    if (device == NULL) {
        add_device(device_id);
        device = find_device_by_android_id(device_id);
    }
    
    if (device && axis_id < (int)device->num_axes) {
        float last_value = device->axis_states[axis_id];
        if (value != last_value) {
            device->axis_states[axis_id] = value;
            if (hal_gamepad_axis_cb)
                hal_gamepad_axis_cb(device, axis_id, value, last_value, 0.0, hal_gamepad_axis_ctx);
        }
    }
}

/* Called from Java via JNI when a button event occurs */
void hal_gamepad_on_button_event(int32_t device_id, int button_id, bool pressed) {
    if (!inited)
        return;
    
    hal_gamepad_device_t *device = find_device_by_android_id(device_id);
    if (device == NULL) {
        add_device(device_id);
        device = find_device_by_android_id(device_id);
    }
    
    if (device && button_id < (int)device->num_buttons) {
        device->button_states[button_id] = pressed;
        if (pressed && hal_gamepad_button_down_cb)
            hal_gamepad_button_down_cb(device, button_id, 0.0, hal_gamepad_button_down_ctx);
        else if (!pressed && hal_gamepad_button_up_cb)
            hal_gamepad_button_up_cb(device, button_id, 0.0, hal_gamepad_button_up_ctx);
    }
}

void hal_gamepad_process_events(void) {
    /* Android events are processed via JNI callbacks from Java */
}

#endif /* HAL_NO_GAMEPAD */
