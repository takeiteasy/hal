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
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>

typedef struct {
    GCController *controller;
} hal_gamepad_private_t;

static hal_gamepad_device_t **devices = NULL;
static unsigned int num_devices = 0;
static unsigned int next_device_id = 0;
static bool inited = false;

static hal_gamepad_device_t *device_for_controller(GCController *controller) {
    for (unsigned int i = 0; i < num_devices; i++) {
        hal_gamepad_private_t *priv = devices[i]->private_data;
        if (priv->controller == controller)
            return devices[i];
    }
    return NULL;
}

static void setup_handlers(hal_gamepad_device_t *device) {
    hal_gamepad_private_t *priv = device->private_data;
    GCController *controller = priv->controller;
    
    if (controller.extendedGamepad != nil) {
        GCExtendedGamepad *gamepad = controller.extendedGamepad;
        
        /* Button handlers */
        gamepad.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            hal_gamepad_button_event_t *event = malloc(sizeof(hal_gamepad_button_event_t));
            event->device = device;
            event->timestamp = [[NSProcessInfo processInfo] systemUptime];
            event->button_id = 0;
            event->down = pressed;
            if (pressed && hal_gamepad_button_down_cb)
                hal_gamepad_button_down_cb(device, 0, event->timestamp, hal_gamepad_button_down_ctx);
            else if (!pressed && hal_gamepad_button_up_cb)
                hal_gamepad_button_up_cb(device, 0, event->timestamp, hal_gamepad_button_up_ctx);
            device->button_states[0] = pressed;
            free(event);
        };
        
        gamepad.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (pressed && hal_gamepad_button_down_cb)
                hal_gamepad_button_down_cb(device, 1, ts, hal_gamepad_button_down_ctx);
            else if (!pressed && hal_gamepad_button_up_cb)
                hal_gamepad_button_up_cb(device, 1, ts, hal_gamepad_button_up_ctx);
            device->button_states[1] = pressed;
        };
        
        gamepad.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (pressed && hal_gamepad_button_down_cb)
                hal_gamepad_button_down_cb(device, 2, ts, hal_gamepad_button_down_ctx);
            else if (!pressed && hal_gamepad_button_up_cb)
                hal_gamepad_button_up_cb(device, 2, ts, hal_gamepad_button_up_ctx);
            device->button_states[2] = pressed;
        };
        
        gamepad.buttonY.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (pressed && hal_gamepad_button_down_cb)
                hal_gamepad_button_down_cb(device, 3, ts, hal_gamepad_button_down_ctx);
            else if (!pressed && hal_gamepad_button_up_cb)
                hal_gamepad_button_up_cb(device, 3, ts, hal_gamepad_button_up_ctx);
            device->button_states[3] = pressed;
        };
        
        /* Axis handlers */
        gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (hal_gamepad_axis_cb) {
                if (xValue != device->axis_states[0])
                    hal_gamepad_axis_cb(device, 0, xValue, device->axis_states[0], ts, hal_gamepad_axis_ctx);
                if (yValue != device->axis_states[1])
                    hal_gamepad_axis_cb(device, 1, yValue, device->axis_states[1], ts, hal_gamepad_axis_ctx);
            }
            device->axis_states[0] = xValue;
            device->axis_states[1] = yValue;
        };
        
        gamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (hal_gamepad_axis_cb) {
                if (xValue != device->axis_states[2])
                    hal_gamepad_axis_cb(device, 2, xValue, device->axis_states[2], ts, hal_gamepad_axis_ctx);
                if (yValue != device->axis_states[3])
                    hal_gamepad_axis_cb(device, 3, yValue, device->axis_states[3], ts, hal_gamepad_axis_ctx);
            }
            device->axis_states[2] = xValue;
            device->axis_states[3] = yValue;
        };
        
        gamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (hal_gamepad_axis_cb && value != device->axis_states[4])
                hal_gamepad_axis_cb(device, 4, value, device->axis_states[4], ts, hal_gamepad_axis_ctx);
            device->axis_states[4] = value;
        };
        
        gamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            double ts = [[NSProcessInfo processInfo] systemUptime];
            if (hal_gamepad_axis_cb && value != device->axis_states[5])
                hal_gamepad_axis_cb(device, 5, value, device->axis_states[5], ts, hal_gamepad_axis_ctx);
            device->axis_states[5] = value;
        };
    }
}

static void on_controller_connected(NSNotification *notification) {
    GCController *controller = notification.object;
    
    if (device_for_controller(controller) != nil)
        return;
    
    hal_gamepad_device_t *device = malloc(sizeof(hal_gamepad_device_t));
    device->device_id = next_device_id++;
    device->description = controller.vendorName ? [controller.vendorName UTF8String] : "Unknown Controller";
    device->vendor_id = 0;
    device->product_id = 0;
    device->num_axes = 6;     /* 2 sticks + 2 triggers */
    device->num_buttons = 16;
    device->axis_states = calloc(sizeof(float), device->num_axes);
    device->button_states = calloc(sizeof(bool), device->num_buttons);
    
    hal_gamepad_private_t *priv = malloc(sizeof(hal_gamepad_private_t));
    priv->controller = controller;
    device->private_data = priv;
    
    devices = realloc(devices, sizeof(hal_gamepad_device_t *) * (num_devices + 1));
    devices[num_devices++] = device;
    
    setup_handlers(device);
    
    if (hal_gamepad_attach_cb)
        hal_gamepad_attach_cb(device, hal_gamepad_attach_ctx);
}

static void on_controller_disconnected(NSNotification *notification) {
    GCController *controller = notification.object;
    hal_gamepad_device_t *device = device_for_controller(controller);
    
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
    
    [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidConnectNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification *note) {
                                                      on_controller_connected(note);
                                                  }];
    
    [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidDisconnectNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification *note) {
                                                      on_controller_disconnected(note);
                                                  }];
    
    inited = true;
    hal_gamepad_detect_devices();
}

void hal_gamepad_shutdown(void) {
    if (!inited)
        return;
    
    [[NSNotificationCenter defaultCenter] removeObserver:nil name:GCControllerDidConnectNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:nil name:GCControllerDidDisconnectNotification object:nil];
    
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
    if (!inited)
        return;
    
    NSArray<GCController *> *controllers = [GCController controllers];
    for (GCController *controller in controllers) {
        if (device_for_controller(controller) == NULL) {
            NSNotification *note = [NSNotification notificationWithName:GCControllerDidConnectNotification object:controller];
            on_controller_connected(note);
        }
    }
}

void hal_gamepad_process_events(void) {
    /* iOS Game Controller framework uses callbacks, no explicit polling needed */
    /* Events are delivered via notification handlers set up in init */
}

#endif /* HAL_NO_GAMEPAD */
