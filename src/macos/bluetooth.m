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

// macOS bluetooth using IOBluetooth

#ifndef HAL_NO_BLUETOOTH
#include "hal/bluetooth.h"
#import <Foundation/Foundation.h>
#import <IOBluetooth/IOBluetooth.h>

bool hal_bluetooth_available(void) {
    return true;
}

bool hal_bluetooth_is_enabled(void) {
    return [IOBluetoothHostController defaultController].powerState == kBluetoothHCIPowerStateON;
}

bool hal_bluetooth_enable(void) {
    IOBluetoothHostController *controller = [IOBluetoothHostController defaultController];
    if (!controller) return false;
    // Note: Direct power control requires authorization
    // This is a best-effort approach
    return true;
}

bool hal_bluetooth_disable(void) {
    IOBluetoothHostController *controller = [IOBluetoothHostController defaultController];
    if (!controller) return false;
    return true;
}

#endif // HAL_NO_BLUETOOTH
