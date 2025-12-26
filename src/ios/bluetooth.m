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

// iOS bluetooth using CoreBluetooth

#ifndef HAL_NO_BLUETOOTH
#include "hal/bluetooth.h"
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

static CBCentralManager *g_central_manager = nil;

@interface HALBluetoothDelegate : NSObject <CBCentralManagerDelegate>
@property (nonatomic) CBManagerState state;
@end

@implementation HALBluetoothDelegate
- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    self.state = central.state;
}
@end

static HALBluetoothDelegate *g_delegate = nil;

static void ensure_manager(void) {
    if (!g_central_manager) {
        g_delegate = [[HALBluetoothDelegate alloc] init];
        g_central_manager = [[CBCentralManager alloc] initWithDelegate:g_delegate queue:nil];
    }
}

bool hal_bluetooth_available(void) {
    ensure_manager();
    return g_central_manager.state != CBManagerStateUnsupported;
}

bool hal_bluetooth_is_enabled(void) {
    ensure_manager();
    return g_central_manager.state == CBManagerStatePoweredOn;
}

bool hal_bluetooth_enable(void) {
    // iOS doesn't allow programmatic enable/disable
    // User must do it through Settings
    return false;
}

bool hal_bluetooth_disable(void) {
    return false;
}

#endif // HAL_NO_BLUETOOTH
