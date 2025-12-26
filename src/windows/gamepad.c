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

#define _WIN32_WINNT 0x0501
#define DIRECTINPUT_VERSION 0x0800

#ifdef _MSC_VER
#define strdup _strdup
#undef UNICODE
#else
#define __in
#define __out
#define __reserved
#endif

#include <windows.h>
#include <dinput.h>
#include <XInput.h>
#include <Dbt.h>
#include <stdio.h>

#define XINPUT_GAMEPAD_GUIDE 0x400
#define INPUT_QUEUE_SIZE 32
#define AXIS_MIN -32768
#define AXIS_MAX 32767

typedef DWORD (WINAPI *XInputGetState_t)(DWORD, XINPUT_STATE *);
typedef DWORD (WINAPI *XInputGetCapabilities_t)(DWORD, DWORD, XINPUT_CAPABILITIES *);

typedef struct {
    DWORD offset;
    bool is_pov;
    bool is_pov_second_axis;
} hal_di_axis_info_t;

typedef struct {
    bool is_xinput;
    
    /* DirectInput specific */
    GUID guid_instance;
    IDirectInputDevice8 *device_interface;
    bool buffered;
    unsigned int slider_count;
    unsigned int pov_count;
    hal_di_axis_info_t *axis_info;
    DWORD *button_offsets;
    
    /* XInput specific */
    unsigned int player_index;
    XINPUT_STATE last_state;
} hal_gamepad_private_t;

static LPDIRECTINPUT direct_input_interface;
static bool inited = false;
static bool xinput_available = false;

static XInputGetState_t XInputGetState_proc = NULL;
static XInputGetCapabilities_t XInputGetCapabilities_proc = NULL;

static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;

static hal_gamepad_device_t *registered_xinput_devices[4] = {NULL, NULL, NULL, NULL};
static const char *xinput_device_names[4] = {
    "XInput Controller 1",
    "XInput Controller 2",
    "XInput Controller 3",
    "XInput Controller 4"
};

static hal_gamepad_queued_event_t *event_queue = NULL;
static size_t event_queue_size = 0;
static size_t event_count = 0;

static double current_time_seconds(void) {
    static LARGE_INTEGER frequency;
    LARGE_INTEGER current;
    if (frequency.QuadPart == 0)
        QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&current);
    return (double)current.QuadPart / frequency.QuadPart;
}

static void queue_event(unsigned int device_id, hal_gamepad_event_type_t event_type, void *event_data) {
    hal_gamepad_queued_event_t event;
    event.device_id = device_id;
    event.event_type = event_type;
    event.event_data = event_data;

    if (event_count >= event_queue_size) {
        event_queue_size = event_queue_size == 0 ? 1 : event_queue_size * 2;
        event_queue = realloc(event_queue, sizeof(hal_gamepad_queued_event_t) * event_queue_size);
    }
    event_queue[event_count++] = event;
}

static void queue_axis_event(hal_gamepad_device_t *device, double timestamp, unsigned int axis_id, float value, float last_value) {
    hal_gamepad_axis_event_t *event = malloc(sizeof(hal_gamepad_axis_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->axis_id = axis_id;
    event->value = value;
    event->last_value = last_value;
    queue_event(device->device_id, HAL_GAMEPAD_EVENT_AXIS_MOVED, event);
}

static void queue_button_event(hal_gamepad_device_t *device, double timestamp, unsigned int button_id, bool down) {
    hal_gamepad_button_event_t *event = malloc(sizeof(hal_gamepad_button_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->button_id = button_id;
    event->down = down;
    queue_event(device->device_id, down ? HAL_GAMEPAD_EVENT_BUTTON_DOWN : HAL_GAMEPAD_EVENT_BUTTON_UP, event);
}

static void dispose_device(hal_gamepad_device_t *device) {
    hal_gamepad_private_t *priv = device->private_data;
    
    if (!priv->is_xinput) {
        IDirectInputDevice8_Release(priv->device_interface);
        free(priv->axis_info);
        free(priv->button_offsets);
        free((void *)device->description);
    }
    free(priv);
    free(device->axis_states);
    free(device->button_states);
    free(device);
}

static hal_gamepad_device_t *create_xinput_device(unsigned int player_index) {
    hal_gamepad_device_t *device = malloc(sizeof(hal_gamepad_device_t));
    device->device_id = next_device_id++;
    device->description = xinput_device_names[player_index];
    device->vendor_id = 0x045E;  /* Microsoft */
    device->product_id = 0x028E; /* Xbox 360 Controller */
    device->num_axes = 6;        /* 2 sticks + 2 triggers */
    device->num_buttons = 15;
    device->axis_states = calloc(sizeof(float), device->num_axes);
    device->button_states = calloc(sizeof(bool), device->num_buttons);
    
    hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
    memset(priv, 0, sizeof(hal_gamepad_private_t));
    priv->is_xinput = true;
    priv->player_index = player_index;
    device->private_data = priv;
    
    return device;
}

static void poll_xinput(void) {
    double timestamp = current_time_seconds();
    
    for (unsigned int i = 0; i < 4; i++) {
        XINPUT_STATE state;
        DWORD result = XInputGetState_proc(i, &state);
        
        if (result == ERROR_SUCCESS) {
            /* Controller connected */
            if (registered_xinput_devices[i] == NULL) {
                hal_gamepad_device_t *device = create_xinput_device(i);
                registered_xinput_devices[i] = device;
                devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
                devices[num_devices++] = device;
                
                if (hal_gamepad_attach_cb != NULL)
                    hal_gamepad_attach_cb(device, hal_gamepad_attach_ctx);
            }
            
            hal_gamepad_device_t *device = registered_xinput_devices[i];
            hal_gamepad_private_t *priv = device->private_data;
            
            if (state.dwPacketNumber != priv->last_state.dwPacketNumber) {
                /* Axes: Left stick X/Y, Right stick X/Y, Left trigger, Right trigger */
                float new_axes[6] = {
                    state.Gamepad.sThumbLX / 32768.0f,
                    state.Gamepad.sThumbLY / 32768.0f,
                    state.Gamepad.sThumbRX / 32768.0f,
                    state.Gamepad.sThumbRY / 32768.0f,
                    state.Gamepad.bLeftTrigger / 255.0f,
                    state.Gamepad.bRightTrigger / 255.0f
                };
                
                for (unsigned int a = 0; a < 6; a++) {
                    if (new_axes[a] != device->axis_states[a]) {
                        queue_axis_event(device, timestamp, a, new_axes[a], device->axis_states[a]);
                        device->axis_states[a] = new_axes[a];
                    }
                }
                
                /* Buttons */
                static const WORD button_masks[15] = {
                    XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
                    XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
                    XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK,
                    XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
                    XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                    XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B,
                    XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
                    XINPUT_GAMEPAD_GUIDE
                };
                
                for (unsigned int b = 0; b < 15; b++) {
                    bool pressed = (state.Gamepad.wButtons & button_masks[b]) != 0;
                    if (pressed != device->button_states[b]) {
                        queue_button_event(device, timestamp, b, pressed);
                        device->button_states[b] = pressed;
                    }
                }
                
                priv->last_state = state;
            }
        } else if (registered_xinput_devices[i] != NULL) {
            /* Controller disconnected */
            hal_gamepad_device_t *device = registered_xinput_devices[i];
            
            if (hal_gamepad_remove_cb != NULL)
                hal_gamepad_remove_cb(device, hal_gamepad_remove_ctx);
            
            for (unsigned int j = 0; j < num_devices; j++) {
                if (devices[j] == device) {
                    num_devices--;
                    for (unsigned int k = j; k < num_devices; k++)
                        devices[k] = devices[k + 1];
                    break;
                }
            }
            
            dispose_device(device);
            registered_xinput_devices[i] = NULL;
        }
    }
}

bool hal_gamepad_available(void) {
    return true;
}

void hal_gamepad_init(void) {
    if (inited)
        return;

    HMODULE module;
    HRESULT result;
    HRESULT (WINAPI *DirectInput8Create_proc)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

    /* Load XInput */
    module = LoadLibrary("XInput1_4.dll");
    if (module == NULL)
        module = LoadLibrary("XInput1_3.dll");
    
    if (module != NULL) {
        xinput_available = true;
        XInputGetState_proc = (XInputGetState_t)GetProcAddress(module, "XInputGetState");
        XInputGetCapabilities_proc = (XInputGetCapabilities_t)GetProcAddress(module, "XInputGetCapabilities");
    }

    /* Load DirectInput */
    module = LoadLibrary("DINPUT8.dll");
    if (module != NULL) {
        DirectInput8Create_proc = (HRESULT (WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN))
            GetProcAddress(module, "DirectInput8Create");
        result = DirectInput8Create_proc(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
            &IID_IDirectInput8, (void **)&direct_input_interface, NULL);
        if (result != DI_OK)
            direct_input_interface = NULL;
    }

    inited = true;
    hal_gamepad_detect_devices();
}

void hal_gamepad_shutdown(void) {
    if (!inited)
        return;

    for (unsigned int i = 0; i < num_devices; i++)
        dispose_device(devices[i]);
    free(devices);
    devices = NULL;
    num_devices = 0;

    for (unsigned int i = 0; i < 4; i++)
        registered_xinput_devices[i] = NULL;

    if (direct_input_interface != NULL) {
        IDirectInput8_Release(direct_input_interface);
        direct_input_interface = NULL;
    }

    free(event_queue);
    event_queue = NULL;
    event_queue_size = 0;
    event_count = 0;

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

    /* XInput devices are detected in poll_xinput() */
    
    /* TODO: DirectInput device enumeration for non-XInput controllers */
    /* This would use IDirectInput8_EnumDevices with DIEDFL_ATTACHEDONLY */
}

static void process_queued_event(hal_gamepad_queued_event_t event) {
    switch (event.event_type) {
        case HAL_GAMEPAD_EVENT_ATTACHED:
            if (hal_gamepad_attach_cb != NULL)
                hal_gamepad_attach_cb(event.event_data, hal_gamepad_attach_ctx);
            break;

        case HAL_GAMEPAD_EVENT_REMOVED:
            if (hal_gamepad_remove_cb != NULL)
                hal_gamepad_remove_cb(event.event_data, hal_gamepad_remove_ctx);
            break;

        case HAL_GAMEPAD_EVENT_BUTTON_DOWN:
            if (hal_gamepad_button_down_cb != NULL) {
                hal_gamepad_button_event_t *e = event.event_data;
                hal_gamepad_button_down_cb(e->device, e->button_id, e->timestamp, hal_gamepad_button_down_ctx);
            }
            break;

        case HAL_GAMEPAD_EVENT_BUTTON_UP:
            if (hal_gamepad_button_up_cb != NULL) {
                hal_gamepad_button_event_t *e = event.event_data;
                hal_gamepad_button_up_cb(e->device, e->button_id, e->timestamp, hal_gamepad_button_up_ctx);
            }
            break;

        case HAL_GAMEPAD_EVENT_AXIS_MOVED:
            if (hal_gamepad_axis_cb != NULL) {
                hal_gamepad_axis_event_t *e = event.event_data;
                hal_gamepad_axis_cb(e->device, e->axis_id, e->value, e->last_value, e->timestamp, hal_gamepad_axis_ctx);
            }
            break;
    }
}

void hal_gamepad_process_events(void) {
    static bool in_process_events = false;

    if (!inited || in_process_events)
        return;

    in_process_events = true;

    if (xinput_available)
        poll_xinput();

    for (size_t i = 0; i < event_count; i++) {
        process_queued_event(event_queue[i]);
        if (event_queue[i].event_type != HAL_GAMEPAD_EVENT_ATTACHED &&
            event_queue[i].event_type != HAL_GAMEPAD_EVENT_REMOVED)
            free(event_queue[i].event_data);
    }
    event_count = 0;

    in_process_events = false;
}

#endif /* HAL_NO_GAMEPAD */
