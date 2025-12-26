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

// iOS GPS using CLLocationManager

#ifndef HAL_NO_GPS
#include "hal/gps.h"
#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

static hal_gps_location_cb g_location_cb = NULL;
static hal_gps_status_cb g_status_cb = NULL;
static void *g_ctx = NULL;
static CLLocationManager *g_manager = nil;

@interface HALLocationDelegate : NSObject <CLLocationManagerDelegate>
@end

@implementation HALLocationDelegate

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations {
    CLLocation *loc = locations.lastObject;
    if (g_location_cb && loc) {
        g_location_cb(loc.coordinate.latitude, loc.coordinate.longitude,
                      loc.altitude, loc.speed >= 0 ? loc.speed : 0,
                      loc.course >= 0 ? loc.course : 0, g_ctx);
    }
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {
    if (g_status_cb) {
        g_status_cb(error.localizedDescription.UTF8String, g_ctx);
    }
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager {
    if (g_status_cb) {
        switch (manager.authorizationStatus) {
            case kCLAuthorizationStatusNotDetermined:
                g_status_cb("authorization_not_determined", g_ctx); break;
            case kCLAuthorizationStatusRestricted:
                g_status_cb("authorization_restricted", g_ctx); break;
            case kCLAuthorizationStatusDenied:
                g_status_cb("authorization_denied", g_ctx); break;
            case kCLAuthorizationStatusAuthorizedAlways:
            case kCLAuthorizationStatusAuthorizedWhenInUse:
                g_status_cb("authorization_granted", g_ctx); break;
        }
    }
}

@end

static HALLocationDelegate *g_delegate = nil;

bool hal_gps_available(void) {
    return [CLLocationManager locationServicesEnabled];
}

void hal_gps_configure(hal_gps_location_cb on_location, hal_gps_status_cb on_status, void *ctx) {
    g_location_cb = on_location;
    g_status_cb = on_status;
    g_ctx = ctx;
}

bool hal_gps_start(int min_time_ms, float min_distance_m) {
    (void)min_time_ms;
    
    if (!hal_gps_available()) return false;
    
    if (g_manager == nil) {
        g_manager = [[CLLocationManager alloc] init];
        g_delegate = [[HALLocationDelegate alloc] init];
        g_manager.delegate = g_delegate;
    }
    
    g_manager.distanceFilter = min_distance_m > 0 ? min_distance_m : kCLDistanceFilterNone;
    g_manager.desiredAccuracy = kCLLocationAccuracyBest;
    
    [g_manager requestWhenInUseAuthorization];
    [g_manager startUpdatingLocation];
    
    return true;
}

void hal_gps_stop(void) {
    if (g_manager) {
        [g_manager stopUpdatingLocation];
    }
}

#endif // HAL_NO_GPS
