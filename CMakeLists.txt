cmake_minimum_required(VERSION 3.15)

# Project definition
# Note: C language is always enabled, OBJC is added conditionally for Apple platforms
project(HAL VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include platform detection and helper functions
include(cmake/HalPlatform.cmake)

# Enable Objective-C for Apple platforms
if(HAL_PLATFORM_IOS OR HAL_PLATFORM_MACOS)
  enable_language(OBJC)
  # Disable ARC (Automatic Reference Counting) to match project settings
  set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fno-objc-arc")

  # Set deployment targets
  if(HAL_PLATFORM_MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
  elseif(HAL_PLATFORM_IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS deployment version")
  endif()
endif()

# Include generated module definitions
include(cmake/HalModules.cmake)

# Include module dependency management
include(cmake/HalDependencies.cmake)

# Build options
option(HAL_BUILD_SHARED "Build HAL as a shared library" OFF)
option(HAL_BUILD_EXAMPLES "Build example programs" OFF)

# Initialize source, header, and library lists
set(HAL_SOURCES)
set(HAL_PRIVATE_HEADERS)
set(HAL_PUBLIC_HEADERS)
set(HAL_COMPILE_DEFINITIONS)
set(HAL_LINK_LIBRARIES)

# Add main public header
list(APPEND HAL_PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/hal/hal.h")

# Process each module based on user options
foreach(MODULE IN LISTS HAL_ALL_MODULES)
  string(TOUPPER ${MODULE} MODULE_UPPER)

  if(HAL_ENABLE_${MODULE_UPPER})
    # Module is enabled - add its sources and dependencies
    hal_add_module_sources(${MODULE})
  else()
    # Module is disabled - add preprocessor define
    list(APPEND HAL_COMPILE_DEFINITIONS "HAL_NO_${MODULE_UPPER}")
  endif()
endforeach()

# Create library target (static or shared)
if(HAL_BUILD_SHARED)
  add_library(hal SHARED ${HAL_SOURCES})
  message(STATUS "Building HAL as shared library")
else()
  add_library(hal STATIC ${HAL_SOURCES})
  message(STATUS "Building HAL as static library")
endif()

# Set include directories
target_include_directories(hal
  PUBLIC
    # Allow users to include as: #include <hal/hal.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    # Private headers in src/ directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Apply compile definitions (HAL_NO_* for disabled modules)
if(HAL_COMPILE_DEFINITIONS)
  target_compile_definitions(hal PRIVATE ${HAL_COMPILE_DEFINITIONS})
endif()

# Link required libraries
if(HAL_LINK_LIBRARIES)
  target_link_libraries(hal PRIVATE ${HAL_LINK_LIBRARIES})
endif()

# Print configuration summary
message(STATUS "HAL Configuration Summary:")
message(STATUS "  Platform: ${HAL_PLATFORM}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Library type: ${HAL_BUILD_SHARED}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
list(LENGTH HAL_SOURCES SOURCE_COUNT)
message(STATUS "  Source files: ${SOURCE_COUNT}")

# Installation configuration
install(TARGETS hal
  EXPORT HalTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Install public headers
install(DIRECTORY hal/
  DESTINATION include/hal
  FILES_MATCHING PATTERN "*.h"
)

# Install paul_os.h if it exists (dependency)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/paul_os.h")
  install(FILES paul_os.h DESTINATION include)
endif()

# Export targets for use by other CMake projects
install(EXPORT HalTargets
  FILE HalTargets.cmake
  NAMESPACE HAL::
  DESTINATION lib/cmake/HAL
)

# Create and install CMake package config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/HALConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Create simple config file
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/HALConfig.cmake"
"include(CMakeFindDependencyMacro)
include(\"\${CMAKE_CURRENT_LIST_DIR}/HalTargets.cmake\")
"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/HALConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/HALConfigVersion.cmake"
  DESTINATION lib/cmake/HAL
)

# Optional: Build examples
if(HAL_BUILD_EXAMPLES)
  # Examples directory would go here when created
  message(STATUS "Examples will be built (when examples/ directory exists)")
endif()
