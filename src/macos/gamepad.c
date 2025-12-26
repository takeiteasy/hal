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
#include <IOKit/hid/IOHIDLib.h>
#include <limits.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#define HAL_GAMEPAD_RUN_LOOP_MODE CFSTR("HalGamepadRunLoopMode")

typedef struct {
    IOHIDElementCookie cookie;
    CFIndex logical_min;
    CFIndex logical_max;
    bool has_null_state;
    bool is_hat_switch;
    bool is_hat_switch_second_axis;
} hal_hid_axis_t;

typedef struct {
    IOHIDElementCookie cookie;
} hal_hid_button_t;

typedef struct {
    IOHIDDeviceRef device_ref;
    hal_hid_axis_t *axis_elements;
    hal_hid_button_t *button_elements;
} hal_gamepad_private_t;

static IOHIDManagerRef hid_manager = NULL;
static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;

static hal_gamepad_queued_event_t *input_event_queue = NULL;
static size_t input_event_queue_size = 0;
static size_t input_event_count = 0;

static hal_gamepad_queued_event_t *device_event_queue = NULL;
static size_t device_event_queue_size = 0;
static size_t device_event_count = 0;

static void hat_value_to_xy(CFIndex value, CFIndex range, int *out_x, int *out_y) {
    if (value == range) {
        *out_x = *out_y = 0;
    } else {
        if (value > 0 && value < range / 2)
            *out_x = 1;
        else if (value > range / 2)
            *out_x = -1;
        else
            *out_x = 0;

        if (value > range / 4 * 3 || value < range / 4)
            *out_y = -1;
        else if (value > range / 4 && value < range / 4 * 3)
            *out_y = 1;
        else
            *out_y = 0;
    }
}

static void queue_input_event(unsigned int device_id, hal_gamepad_event_type_t event_type, void *event_data) {
    hal_gamepad_queued_event_t event;
    event.device_id = device_id;
    event.event_type = event_type;
    event.event_data = event_data;

    if (input_event_count >= input_event_queue_size) {
        input_event_queue_size = input_event_queue_size == 0 ? 1 : input_event_queue_size * 2;
        input_event_queue = realloc(input_event_queue, sizeof(hal_gamepad_queued_event_t) * input_event_queue_size);
    }
    input_event_queue[input_event_count++] = event;
}

static void queue_axis_event(hal_gamepad_device_t *device, double timestamp, unsigned int axis_id, float value, float last_value) {
    hal_gamepad_axis_event_t *event = malloc(sizeof(hal_gamepad_axis_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->axis_id = axis_id;
    event->value = value;
    event->last_value = last_value;
    queue_input_event(device->device_id, HAL_GAMEPAD_EVENT_AXIS_MOVED, event);
}

static void queue_button_event(hal_gamepad_device_t *device, double timestamp, unsigned int button_id, bool down) {
    hal_gamepad_button_event_t *event = malloc(sizeof(hal_gamepad_button_event_t));
    event->device = device;
    event->timestamp = timestamp;
    event->button_id = button_id;
    event->down = down;
    queue_input_event(device->device_id, down ? HAL_GAMEPAD_EVENT_BUTTON_DOWN : HAL_GAMEPAD_EVENT_BUTTON_UP, event);
}

static void on_device_value_changed(void *context, IOReturn result, void *sender, IOHIDValueRef value) {
    (void)result;
    (void)sender;
    
    hal_gamepad_device_t *device = context;
    hal_gamepad_private_t *priv = device->private_data;
    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDElementCookie cookie = IOHIDElementGetCookie(element);
    
    static mach_timebase_info_data_t timebase_info;
    if (timebase_info.denom == 0)
        mach_timebase_info(&timebase_info);

    for (unsigned int i = 0; i < device->num_axes; i++) {
        if (!priv->axis_elements[i].is_hat_switch_second_axis &&
            priv->axis_elements[i].cookie == cookie) {
            
            if (IOHIDValueGetLength(value) > 4)
                continue;
                
            CFIndex int_value = IOHIDValueGetIntegerValue(value);
            double timestamp = IOHIDValueGetTimeStamp(value) * timebase_info.numer / timebase_info.denom * 0.000000001;

            if (priv->axis_elements[i].is_hat_switch) {
                int x, y;
                
                if (!priv->axis_elements[i].has_null_state) {
                    if (int_value < priv->axis_elements[i].logical_min)
                        int_value = priv->axis_elements[i].logical_max - priv->axis_elements[i].logical_min + 1;
                    else
                        int_value--;
                }
                
                hat_value_to_xy(int_value, priv->axis_elements[i].logical_max - priv->axis_elements[i].logical_min + 1, &x, &y);
                
                if (x != (int)device->axis_states[i]) {
                    queue_axis_event(device, timestamp, i, (float)x, device->axis_states[i]);
                    device->axis_states[i] = (float)x;
                }
                
                if (y != (int)device->axis_states[i + 1]) {
                    queue_axis_event(device, timestamp, i + 1, (float)y, device->axis_states[i + 1]);
                    device->axis_states[i + 1] = (float)y;
                }
            } else {
                if (int_value < priv->axis_elements[i].logical_min)
                    priv->axis_elements[i].logical_min = int_value;
                if (int_value > priv->axis_elements[i].logical_max)
                    priv->axis_elements[i].logical_max = int_value;
                    
                float float_value = (int_value - priv->axis_elements[i].logical_min) / 
                    (float)(priv->axis_elements[i].logical_max - priv->axis_elements[i].logical_min) * 2.0f - 1.0f;
                
                queue_axis_event(device, timestamp, i, float_value, device->axis_states[i]);
                device->axis_states[i] = float_value;
            }
            return;
        }
    }

    for (unsigned int i = 0; i < device->num_buttons; i++) {
        if (priv->button_elements[i].cookie == cookie) {
            bool down = IOHIDValueGetIntegerValue(value) != 0;
            double timestamp = IOHIDValueGetTimeStamp(value) * timebase_info.numer / timebase_info.denom * 0.000000001;
            
            queue_button_event(device, timestamp, i, down);
            device->button_states[i] = down;
            return;
        }
    }
}

static int iohid_device_get_int_property(IOHIDDeviceRef device_ref, CFStringRef key) {
    CFTypeRef type_ref = IOHIDDeviceGetProperty(device_ref, key);
    if (type_ref == NULL || CFGetTypeID(type_ref) != CFNumberGetTypeID())
        return 0;
    int value;
    CFNumberGetValue((CFNumberRef)type_ref, kCFNumberSInt32Type, &value);
    return value;
}

static void on_device_matched(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    (void)context;
    (void)result;
    (void)sender;
    
    hal_gamepad_device_t *dev = malloc(sizeof(hal_gamepad_device_t));
    dev->device_id = next_device_id++;
    dev->vendor_id = iohid_device_get_int_property(device, CFSTR(kIOHIDVendorIDKey));
    dev->product_id = iohid_device_get_int_property(device, CFSTR(kIOHIDProductIDKey));
    dev->num_axes = 0;
    dev->num_buttons = 0;
    
    devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
    devices[num_devices++] = dev;

    hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
    priv->device_ref = device;
    priv->axis_elements = NULL;
    priv->button_elements = NULL;
    dev->private_data = priv;

    CFStringRef cf_product_name = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    if (cf_product_name == NULL || CFGetTypeID(cf_product_name) != CFStringGetTypeID()) {
        char *desc = malloc(strlen("[Unknown]") + 1);
        strcpy(desc, "[Unknown]");
        dev->description = desc;
    } else {
        CFIndex length;
        CFStringGetBytes(cf_product_name, CFRangeMake(0, CFStringGetLength(cf_product_name)),
                        kCFStringEncodingUTF8, '?', false, NULL, 100, &length);
        char *desc = malloc(length + 1);
        CFStringGetBytes(cf_product_name, CFRangeMake(0, CFStringGetLength(cf_product_name)),
                        kCFStringEncodingUTF8, '?', false, (UInt8 *)desc, length + 1, NULL);
        desc[length] = '\0';
        dev->description = desc;
    }

    CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
    for (CFIndex i = 0; i < CFArrayGetCount(elements); i++) {
        IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        IOHIDElementType type = IOHIDElementGetType(element);

        if (type == kIOHIDElementTypeInput_Misc || type == kIOHIDElementTypeInput_Axis) {
            priv->axis_elements = realloc(priv->axis_elements, sizeof(hal_hid_axis_t) * (dev->num_axes + 1));
            priv->axis_elements[dev->num_axes].cookie = IOHIDElementGetCookie(element);
            priv->axis_elements[dev->num_axes].logical_min = IOHIDElementGetLogicalMin(element);
            priv->axis_elements[dev->num_axes].logical_max = IOHIDElementGetLogicalMax(element);
            priv->axis_elements[dev->num_axes].has_null_state = IOHIDElementHasNullState(element);
            priv->axis_elements[dev->num_axes].is_hat_switch = (IOHIDElementGetUsage(element) == kHIDUsage_GD_Hatswitch);
            priv->axis_elements[dev->num_axes].is_hat_switch_second_axis = false;
            dev->num_axes++;

            if (priv->axis_elements[dev->num_axes - 1].is_hat_switch) {
                priv->axis_elements = realloc(priv->axis_elements, sizeof(hal_hid_axis_t) * (dev->num_axes + 1));
                priv->axis_elements[dev->num_axes].is_hat_switch_second_axis = true;
                dev->num_axes++;
            }
        } else if (type == kIOHIDElementTypeInput_Button) {
            priv->button_elements = realloc(priv->button_elements, sizeof(hal_hid_button_t) * (dev->num_buttons + 1));
            priv->button_elements[dev->num_buttons].cookie = IOHIDElementGetCookie(element);
            dev->num_buttons++;
        }
    }
    CFRelease(elements);

    dev->axis_states = calloc(sizeof(float), dev->num_axes);
    dev->button_states = calloc(sizeof(bool), dev->num_buttons);

    IOHIDDeviceRegisterInputValueCallback(device, on_device_value_changed, dev);

    hal_gamepad_queued_event_t event;
    event.device_id = dev->device_id;
    event.event_type = HAL_GAMEPAD_EVENT_ATTACHED;
    event.event_data = dev;

    if (device_event_count >= device_event_queue_size) {
        device_event_queue_size = device_event_queue_size == 0 ? 1 : device_event_queue_size * 2;
        device_event_queue = realloc(device_event_queue, sizeof(hal_gamepad_queued_event_t) * device_event_queue_size);
    }
    device_event_queue[device_event_count++] = event;
}

static void dispose_device(hal_gamepad_device_t *device) {
    hal_gamepad_private_t *priv = device->private_data;
    
    IOHIDDeviceRegisterInputValueCallback(priv->device_ref, NULL, NULL);

    for (size_t i = 0; i < input_event_count; i++) {
        if (input_event_queue[i].device_id == device->device_id) {
            free(input_event_queue[i].event_data);
            input_event_count--;
            for (size_t j = i; j < input_event_count; j++)
                input_event_queue[j] = input_event_queue[j + 1];
            i--;
        }
    }

    for (size_t i = 0; i < device_event_count; i++) {
        if (device_event_queue[i].device_id == device->device_id) {
            device_event_count--;
            for (size_t j = i; j < device_event_count; j++)
                device_event_queue[j] = device_event_queue[j + 1];
            i--;
        }
    }

    free(priv->axis_elements);
    free(priv->button_elements);
    free(priv);
    free((void *)device->description);
    free(device->axis_states);
    free(device->button_states);
    free(device);
}

static void on_device_removed(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    (void)context;
    (void)result;
    (void)sender;
    
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
        if (priv->device_ref == device) {
            if (hal_gamepad_remove_cb != NULL)
                hal_gamepad_remove_cb(devices[i], hal_gamepad_remove_ctx);
            
            dispose_device(devices[i]);
            num_devices--;
            for (unsigned int j = i; j < num_devices; j++)
                devices[j] = devices[j + 1];
            return;
        }
    }
}

bool hal_gamepad_available(void) {
    return true;
}

void hal_gamepad_init(void) {
    if (hid_manager != NULL)
        return;

    hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    CFStringRef keys[2];
    CFNumberRef values[2];
    CFDictionaryRef dictionaries[3];
    int value;

    keys[0] = CFSTR(kIOHIDDeviceUsagePageKey);
    keys[1] = CFSTR(kIOHIDDeviceUsageKey);

    value = kHIDPage_GenericDesktop;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    value = kHIDUsage_GD_Joystick;
    values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    dictionaries[0] = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2,
                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);

    value = kHIDPage_GenericDesktop;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    value = kHIDUsage_GD_GamePad;
    values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    dictionaries[1] = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2,
                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);

    value = kHIDPage_GenericDesktop;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    value = kHIDUsage_GD_MultiAxisController;
    values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    dictionaries[2] = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2,
                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);

    CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void **)dictionaries, 3, &kCFTypeArrayCallBacks);
    CFRelease(dictionaries[0]);
    CFRelease(dictionaries[1]);
    CFRelease(dictionaries[2]);
    IOHIDManagerSetDeviceMatchingMultiple(hid_manager, array);
    CFRelease(array);

    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, on_device_matched, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, on_device_removed, NULL);

    IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);

    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), HAL_GAMEPAD_RUN_LOOP_MODE);
    CFRunLoopRunInMode(HAL_GAMEPAD_RUN_LOOP_MODE, 0, true);
}

void hal_gamepad_shutdown(void) {
    if (hid_manager == NULL)
        return;

    IOHIDManagerUnscheduleFromRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    for (unsigned int i = 0; i < num_devices; i++)
        dispose_device(devices[i]);
    free(devices);
    devices = NULL;
    num_devices = 0;

    IOHIDManagerClose(hid_manager, 0);
    CFRelease(hid_manager);
    hid_manager = NULL;

    free(input_event_queue);
    input_event_queue = NULL;
    input_event_queue_size = 0;
    input_event_count = 0;

    free(device_event_queue);
    device_event_queue = NULL;
    device_event_queue_size = 0;
    device_event_count = 0;
}

unsigned int hal_gamepad_num_devices(void) {
    return num_devices;
}

hal_gamepad_device_t *hal_gamepad_device_at(unsigned int index) {
    if (index >= num_devices)
        return NULL;
    return devices[index];
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

void hal_gamepad_detect_devices(void) {
    if (hid_manager == NULL)
        return;

    CFRunLoopRunInMode(HAL_GAMEPAD_RUN_LOOP_MODE, 0, true);
    for (size_t i = 0; i < device_event_count; i++)
        process_queued_event(device_event_queue[i]);
    device_event_count = 0;
}

void hal_gamepad_process_events(void) {
    static bool in_process_events = false;
    
    if (hid_manager == NULL || in_process_events)
        return;

    in_process_events = true;
    CFRunLoopRunInMode(HAL_GAMEPAD_RUN_LOOP_MODE, 0, true);
    for (size_t i = 0; i < input_event_count; i++) {
        process_queued_event(input_event_queue[i]);
        free(input_event_queue[i].event_data);
    }
    input_event_count = 0;
    in_process_events = false;
}

#endif /* HAL_NO_GAMEPAD */
