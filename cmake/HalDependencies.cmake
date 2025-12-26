# Module-specific dependencies per platform

# Function to add platform-specific dependencies for each module
function(hal_add_module_dependencies MODULE_NAME)
  string(TOUPPER ${MODULE_NAME} MODULE_UPPER)

  # Android platform dependencies
  if(HAL_PLATFORM_ANDROID)
    if(MODULE_NAME STREQUAL "accelerometer")
      # Android accelerometer uses ASensorManager and ALooper from android library
      list(APPEND HAL_LINK_LIBRARIES android log)
    elseif(MODULE_NAME STREQUAL "barometer")
      # Android barometer uses ASensorManager with ASENSOR_TYPE_PRESSURE
      list(APPEND HAL_LINK_LIBRARIES android log)
    elseif(MODULE_NAME STREQUAL "threads")
      # Android uses POSIX threads (already in libc)
    endif()
    # Add more Android-specific dependencies as modules are implemented
  endif()

  # iOS platform dependencies
  if(HAL_PLATFORM_IOS)
    if(MODULE_NAME STREQUAL "accelerometer")
      # iOS accelerometer uses CoreMotion framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(COREMOTION_FRAMEWORK CoreMotion REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${COREMOTION_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "barometer")
      # iOS barometer uses CMAltimeter from CoreMotion framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(COREMOTION_FRAMEWORK CoreMotion REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${COREMOTION_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "battery")
      # iOS battery uses UIDevice from UIKit framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${UIKIT_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "brightness")
      # iOS brightness uses UIScreen from UIKit framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${UIKIT_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "vibrator")
      # iOS vibrator uses AudioToolbox framework
      find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${AUDIOTOOLBOX_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "flash")
      # iOS flash uses AVFoundation framework
      find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${AVFOUNDATION_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "unique_id")
      # iOS unique ID uses UIKit framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${UIKIT_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "threads")
      # iOS uses POSIX threads (already in libc)
    elseif(MODULE_NAME STREQUAL "gamepad")
      # iOS gamepad uses GameController framework
      find_library(GAMECONTROLLER_FRAMEWORK GameController REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${GAMECONTROLLER_FRAMEWORK})
    endif()
    # Add more iOS-specific dependencies as modules are implemented
  endif()

  # macOS platform dependencies
  if(HAL_PLATFORM_MACOS)
    if(MODULE_NAME STREQUAL "accelerometer")
      # macOS accelerometer uses CoreMotion framework
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(COREMOTION_FRAMEWORK CoreMotion REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${COREMOTION_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "battery")
      # macOS battery uses IOKit power sources
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${IOKIT_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "brightness")
      # macOS brightness uses DisplayServices (private framework in CoreGraphics)
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "unique_id")
      # macOS unique ID uses IOKit
      find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
      find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${FOUNDATION_FRAMEWORK} ${IOKIT_FRAMEWORK})
    elseif(MODULE_NAME STREQUAL "threads")
      # macOS uses POSIX threads (already in libc)
    elseif(MODULE_NAME STREQUAL "gamepad")
      # macOS gamepad uses IOKit HID
      find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES ${IOKIT_FRAMEWORK})
    endif()
    # Add more macOS-specific dependencies as modules are implemented
  endif()

  # Linux platform dependencies
  if(HAL_PLATFORM_LINUX)
    if(MODULE_NAME STREQUAL "threads")
      # Linux uses POSIX threads
      find_package(Threads REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES Threads::Threads)
    elseif(MODULE_NAME STREQUAL "gamepad")
      # Linux gamepad uses POSIX threads for device polling
      find_package(Threads REQUIRED)
      list(APPEND HAL_LINK_LIBRARIES Threads::Threads)
    endif()
    # Add more Linux-specific dependencies as modules are implemented
    # Example: clipboard might need X11, GTK, or Wayland libraries
  endif()

  # Windows platform dependencies
  if(HAL_PLATFORM_WINDOWS)
    if(MODULE_NAME STREQUAL "threads")
      # Windows thread API is in kernel32.lib (linked by default)
    elseif(MODULE_NAME STREQUAL "gamepad")
      # Windows gamepad uses XInput and DirectInput
      list(APPEND HAL_LINK_LIBRARIES dinput8 xinput)
    endif()
    # Add more Windows-specific dependencies as modules are implemented
  endif()

  # Web/Emscripten dependencies
  if(HAL_PLATFORM_WEB)
    # Add Emscripten-specific flags and dependencies as needed
    # Example: -s USE_PTHREADS=1 for threading support
    if(MODULE_NAME STREQUAL "threads")
      # Emscripten threading might need special flags
      list(APPEND HAL_COMPILE_DEFINITIONS "USE_PTHREADS=1")
    endif()
    # Add more Web-specific dependencies as modules are implemented
  endif()

  # Propagate the updated libraries list back to parent scope
  set(HAL_LINK_LIBRARIES ${HAL_LINK_LIBRARIES} PARENT_SCOPE)
  set(HAL_COMPILE_DEFINITIONS ${HAL_COMPILE_DEFINITIONS} PARENT_SCOPE)
endfunction()
