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

// iOS camera using UIImagePickerController

#ifndef HAL_NO_CAMERA
#include "hal/camera.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

static hal_camera_cb g_camera_cb = NULL;
static void *g_camera_ctx = NULL;
static NSString *g_filename = nil;
static UIViewController *g_view_controller = nil;

void hal_camera_ios_set_view_controller(void *vc) {
    g_view_controller = (__bridge UIViewController *)vc;
}

@interface HALCameraDelegate : NSObject <UIImagePickerControllerDelegate, UINavigationControllerDelegate>
@property (nonatomic) BOOL isVideo;
@end

@implementation HALCameraDelegate

- (void)imagePickerController:(UIImagePickerController *)picker 
        didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey,id> *)info {
    [picker dismissViewControllerAnimated:YES completion:nil];
    
    if (self.isVideo) {
        NSURL *videoURL = info[UIImagePickerControllerMediaURL];
        if (videoURL && g_camera_cb) {
            g_camera_cb(videoURL.path.UTF8String, g_camera_ctx);
        }
    } else {
        UIImage *image = info[UIImagePickerControllerOriginalImage];
        if (image && g_filename) {
            NSData *data = UIImageJPEGRepresentation(image, 0.9);
            NSString *path = [NSTemporaryDirectory() stringByAppendingPathComponent:g_filename];
            [data writeToFile:path atomically:YES];
            if (g_camera_cb) {
                g_camera_cb(path.UTF8String, g_camera_ctx);
            }
        }
    }
    
    g_camera_cb = NULL;
    g_camera_ctx = NULL;
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [picker dismissViewControllerAnimated:YES completion:nil];
    if (g_camera_cb) {
        g_camera_cb(NULL, g_camera_ctx);
    }
    g_camera_cb = NULL;
    g_camera_ctx = NULL;
}

@end

static HALCameraDelegate *g_delegate = nil;

bool hal_camera_available(void) {
    return [UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera];
}

static bool start_capture(const char *filename, hal_camera_cb on_complete, void *ctx, BOOL video) {
    if (!hal_camera_available() || !g_view_controller) return false;
    
    g_camera_cb = on_complete;
    g_camera_ctx = ctx;
    g_filename = filename ? [NSString stringWithUTF8String:filename] : nil;
    
    if (!g_delegate) {
        g_delegate = [[HALCameraDelegate alloc] init];
    }
    g_delegate.isVideo = video;
    
    UIImagePickerController *picker = [[UIImagePickerController alloc] init];
    picker.sourceType = UIImagePickerControllerSourceTypeCamera;
    picker.delegate = g_delegate;
    
    if (video) {
        picker.mediaTypes = @[(NSString *)kUTTypeMovie];
        picker.videoQuality = UIImagePickerControllerQualityTypeHigh;
    }
    
    [g_view_controller presentViewController:picker animated:YES completion:nil];
    return true;
}

bool hal_camera_take_picture(const char *filename, hal_camera_cb on_complete, void *ctx) {
    return start_capture(filename, on_complete, ctx, NO);
}

bool hal_camera_take_video(const char *filename, hal_camera_cb on_complete, void *ctx) {
    return start_capture(filename, on_complete, ctx, YES);
}

#endif // HAL_NO_CAMERA
