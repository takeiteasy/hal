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

#ifndef HAL_NO_ACCELEROMETER
#include "../accelerometer.h"
#import <IOKit/IOKitLib.h>

typedef struct {
    int16_t x, y, z;
    uint8_t pad[34];
} in_struct;

@interface Accelerometer : NSObject
@property io_connect_t dataPort;
-(id)init;
- (BOOL)getCoordsX:(float*)x Y:(float*)y Z:(float*)z;
@end

@implementation Accelerometer
@synthesize dataPort;

- (id)init {
#ifndef ARCH_64BIT
    return nil;
#endif
    if ((self = [super init])) {
        dataPort = IO_OBJECT_NULL;
        mach_port_t master;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        kern_return_t result = IOMasterPort(MACH_PORT_NULL, &master);
#pragma clang diagnostic pop
        CFMutableDictionaryRef match = IOServiceMatching("SMCMotionSensor");
        io_iterator_t iter;
        if ((result = IOServiceGetMatchingServices(master, match, &iter)) != KERN_SUCCESS)
            return nil;
        io_object_t sms = IOIteratorNext(iter);
        if (sms == IO_OBJECT_NULL)
            return nil;
        dataPort = IO_OBJECT_NULL;
        if ((result = IOServiceOpen(sms, mach_task_self(), 0, &dataPort)))
            return nil;

    }
    return self;
}

- (void)dealloc {
    if (dataPort != IO_OBJECT_NULL)
        IOServiceClose(dataPort);
    [super dealloc];
}

- (BOOL)getCoordsX:(float*)x Y:(float*)y Z:(float*)z {
    if (dataPort == IO_OBJECT_NULL)
        goto BAIL;
    in_struct _in = {0}, _out = {0};
    size_t _out_sz;
    kern_return_t result = IOConnectCallStructMethod(dataPort, 5, &_in, sizeof(in_struct), &_out, &_out_sz);
    if (result != KERN_SUCCESS) {
        IOServiceClose(dataPort);
        goto BAIL;
    }
    if (x)
        *x = _in.x;
    if (y)
        *y = _in.y;
    if (z)
        *z = _in.z;
    return YES;
BAIL:
    if (x)
        *x = -1;
    if (y)
        *y = -1;
    if (z)
        *z = -1;
    return NO;
}
@end

static Accelerometer* _accelerometer = NULL;

bool hal_accelerometer_available(void) {
    Accelerometer *tmp = [Accelerometer new];
    if (!tmp)
        return false;
    bool result = [tmp getCoordsX:NULL Y:NULL Z:NULL];
    [tmp dealloc];
    return result;
}

void hal_accelerometer_enable(void) {
    if (!_accelerometer)
        _accelerometer = [Accelerometer new];
}

void hal_accelerometer_disable(void) {
    if (_accelerometer) {
        [_accelerometer dealloc];
        _accelerometer = NULL;
    }
}

bool hal_accelerometer_enabled(void) {
    return _accelerometer != NULL;
}

bool hal_accelerometer_disabled(void) {
    return _accelerometer == NULL;
}

bool hal_accelerometer_toggle(void) {
    bool state = hal_accelerometer_enabled();
    if (state)
        hal_accelerometer_disable();
    else
        hal_accelerometer_enable();
    return !state;
}

bool hal_accelerometer_acceleration(float *x, float *y, float *z)  {
    if (hal_accelerometer_enabled()) {
        [_accelerometer getCoordsX:x Y:y Z:z];
        return true;
    } else {
        if (x)
            *x = -1;
        if (y)
            *y = -1;
        if (z)
            *z = -1;
        return false;
    }
}
#endif // HAL_NO_ACCELEROMETER
