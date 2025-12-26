# Platform detection and source file discovery for HAL

# Detect target platform
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(HAL_PLATFORM "android")
  set(HAL_PLATFORM_ANDROID TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(HAL_PLATFORM "ios")
  set(HAL_PLATFORM_IOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(HAL_PLATFORM "linux")
  set(HAL_PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(HAL_PLATFORM "macos")
  set(HAL_PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(HAL_PLATFORM "windows")
  set(HAL_PLATFORM_WINDOWS TRUE)
elseif(EMSCRIPTEN)
  set(HAL_PLATFORM "web")
  set(HAL_PLATFORM_WEB TRUE)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "HAL Platform: ${HAL_PLATFORM}")

# Helper function to add module sources based on platform
# This function discovers platform-specific implementation files and adds them to HAL_SOURCES
function(hal_add_module_sources MODULE_NAME)
  string(TOUPPER ${MODULE_NAME} MODULE_UPPER)

  # Add public header if it exists
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/hal/${MODULE_NAME}.h")
    list(APPEND HAL_PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/hal/${MODULE_NAME}.h")
    set(HAL_PUBLIC_HEADERS ${HAL_PUBLIC_HEADERS} PARENT_SCOPE)
  endif()

  # Determine source file extension (.c or .m for Objective-C)
  set(SOURCE_FILE "")
  if(HAL_PLATFORM_IOS OR HAL_PLATFORM_MACOS)
    # Apple platforms may use .m (Objective-C) or .c files
    set(POSSIBLE_EXTENSIONS ".m" ".c")
  else()
    # Other platforms use .c files
    set(POSSIBLE_EXTENSIONS ".c")
  endif()

  # Look for platform-specific implementation
  foreach(EXT IN LISTS POSSIBLE_EXTENSIONS)
    set(PLATFORM_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/${HAL_PLATFORM}/${MODULE_NAME}${EXT}")
    if(EXISTS "${PLATFORM_SOURCE}")
      set(SOURCE_FILE "${PLATFORM_SOURCE}")
      break()
    endif()
  endforeach()

  # Fallback to shared implementations
  if(NOT SOURCE_FILE)
    # threads module has a shared POSIX implementation
    if(MODULE_NAME STREQUAL "threads")
      if(NOT HAL_PLATFORM_WINDOWS)
        set(POSIX_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/threads_posix.c")
        if(EXISTS "${POSIX_SOURCE}")
          set(SOURCE_FILE "${POSIX_SOURCE}")
        endif()
      endif()
    endif()
  endif()

  # Add additional sources for specific modules
  if(MODULE_NAME STREQUAL "gamepad")
    set(MAPPING_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/gamepad_mapping.c")
    if(EXISTS "${MAPPING_SOURCE}")
      list(APPEND HAL_SOURCES "${MAPPING_SOURCE}")
    endif()
  endif()

  # Fallback to dummy implementation
  if(NOT SOURCE_FILE)
    set(DUMMY_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/dummy_${MODULE_NAME}.c")
    if(EXISTS "${DUMMY_SOURCE}")
      set(SOURCE_FILE "${DUMMY_SOURCE}")
      message(WARNING "Using dummy implementation for ${MODULE_NAME} on ${HAL_PLATFORM}")
    endif()
  endif()

  # Add the source file if found
  if(SOURCE_FILE)
    list(APPEND HAL_SOURCES "${SOURCE_FILE}")
    set(HAL_SOURCES ${HAL_SOURCES} PARENT_SCOPE)
  else()
    message(WARNING "No implementation found for ${MODULE_NAME} on ${HAL_PLATFORM} - module will be skipped")
    # Automatically disable the module by adding HAL_NO_<MODULE> define
    list(APPEND HAL_COMPILE_DEFINITIONS "HAL_NO_${MODULE_UPPER}")
    set(HAL_COMPILE_DEFINITIONS ${HAL_COMPILE_DEFINITIONS} PARENT_SCOPE)
    return()
  endif()

  # Add module-specific dependencies
  hal_add_module_dependencies(${MODULE_NAME})

  # Propagate variables to parent scope
  set(HAL_COMPILE_DEFINITIONS ${HAL_COMPILE_DEFINITIONS} PARENT_SCOPE)
  set(HAL_LINK_LIBRARIES ${HAL_LINK_LIBRARIES} PARENT_SCOPE)
endfunction()
