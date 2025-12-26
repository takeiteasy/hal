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

// iOS orientation using UIDevice

#ifndef HAL_NO_ORIENTATION
#include "hal/orientation.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

bool hal_orientation_available(void) {
    return true;
}

bool hal_orientation_set_landscape(bool reverse) {
    UIInterfaceOrientationMask mask = reverse ? 
        UIInterfaceOrientationMaskLandscapeRight : UIInterfaceOrientationMaskLandscapeLeft;
    
    if (@available(iOS 16.0, *)) {
        UIWindowScene *scene = (UIWindowScene *)[[UIApplication sharedApplication].connectedScenes anyObject];
        UIWindowSceneGeometryPreferencesIOS *prefs = 
            [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:mask];
        [scene requestGeometryUpdateWithPreferences:prefs errorHandler:nil];
    }
    
    return true;
}

bool hal_orientation_set_portrait(bool reverse) {
    UIInterfaceOrientationMask mask = reverse ?
        UIInterfaceOrientationMaskPortraitUpsideDown : UIInterfaceOrientationMaskPortrait;
    
    if (@available(iOS 16.0, *)) {
        UIWindowScene *scene = (UIWindowScene *)[[UIApplication sharedApplication].connectedScenes anyObject];
        UIWindowSceneGeometryPreferencesIOS *prefs =
            [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:mask];
        [scene requestGeometryUpdateWithPreferences:prefs errorHandler:nil];
    }
    
    return true;
}

bool hal_orientation_set_sensor(void) {
    if (@available(iOS 16.0, *)) {
        UIWindowScene *scene = (UIWindowScene *)[[UIApplication sharedApplication].connectedScenes anyObject];
        UIWindowSceneGeometryPreferencesIOS *prefs =
            [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:UIInterfaceOrientationMaskAll];
        [scene requestGeometryUpdateWithPreferences:prefs errorHandler:nil];
    }
    
    return true;
}

bool hal_orientation_lock(void) {
    // Get current orientation and lock to it
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    UIInterfaceOrientationMask mask;
    
    switch (orientation) {
        case UIDeviceOrientationLandscapeLeft:
            mask = UIInterfaceOrientationMaskLandscapeRight; break;
        case UIDeviceOrientationLandscapeRight:
            mask = UIInterfaceOrientationMaskLandscapeLeft; break;
        case UIDeviceOrientationPortraitUpsideDown:
            mask = UIInterfaceOrientationMaskPortraitUpsideDown; break;
        default:
            mask = UIInterfaceOrientationMaskPortrait; break;
    }
    
    if (@available(iOS 16.0, *)) {
        UIWindowScene *scene = (UIWindowScene *)[[UIApplication sharedApplication].connectedScenes anyObject];
        UIWindowSceneGeometryPreferencesIOS *prefs =
            [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:mask];
        [scene requestGeometryUpdateWithPreferences:prefs errorHandler:nil];
    }
    
    return true;
}

bool hal_orientation_unlock(void) {
    return hal_orientation_set_sensor();
}

#endif // HAL_NO_ORIENTATION
