cmake_minimum_required(VERSION 3.15)

# ============================================================================
# PROJECT CONFIGURATION
# ============================================================================
project(MyPlugin VERSION 1.0.0)

# Prevent in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# Set default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# C++ STANDARD
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# PLATFORM-SPECIFIC CONFIGURATION
# ============================================================================

# --- macOS ---
if(APPLE)
    option(UNIVERSAL_BINARY "Build universal binary for macOS (arm64 + x86_64)" ON)
    
    if(UNIVERSAL_BINARY)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)
    else()
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)
        endif()
    endif()
    
    message(STATUS "macOS architectures: ${CMAKE_OSX_ARCHITECTURES}")
    message(STATUS "macOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# --- Windows ---
if(WIN32)
    # Use static runtime for easier distribution
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# --- Linux ---
if(UNIX AND NOT APPLE)
    # Find required packages
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)
    
    # Add GTK include directories
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
endif()

# ============================================================================
# LOCATE JUCE
# ============================================================================
option(JUCE_DIR "Path to JUCE framework" "")

if(NOT JUCE_DIR OR NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    # Try common locations
    set(JUCE_SEARCH_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/JUCE"
        "$ENV{HOME}/JUCE"
        "C:/JUCE"
        "D:/JUCE"
    )
    
    foreach(SEARCH_PATH ${JUCE_SEARCH_PATHS})
        if(EXISTS "${SEARCH_PATH}/CMakeLists.txt")
            set(JUCE_DIR "${SEARCH_PATH}")
            break()
        endif()
    endforeach()
    
    if(NOT JUCE_DIR OR NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR 
            "JUCE not found. Please:\n"
            "  1. Clone JUCE: git clone https://github.com/juce-framework/JUCE.git\n"
            "  2. Set JUCE_DIR: cmake -DJUCE_DIR=/path/to/JUCE ..\n"
            "Searched locations: ${JUCE_SEARCH_PATHS}"
        )
    endif()
endif()

message(STATUS "Using JUCE from: ${JUCE_DIR}")

# Configure JUCE
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)
add_subdirectory(${JUCE_DIR} JUCE)

# ============================================================================
# PLUGIN CONFIGURATION
# ============================================================================

# Plugin formats to build
set(FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND FORMATS AU)
endif()

# Installation directories
if(WIN32)
    file(TO_CMAKE_PATH "$ENV{COMMONPROGRAMFILES}" COMMON_PROGRAM_FILES)
    set(VST3_INSTALL_DIR "${COMMON_PROGRAM_FILES}/VST3")
elseif(APPLE)
    set(VST3_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(AU_INSTALL_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
elseif(UNIX)
    set(VST3_INSTALL_DIR "$ENV{HOME}/.vst3")
endif()

# ============================================================================
# SOURCE FILES
# ============================================================================
set(SourceFiles
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# ============================================================================
# PLUGIN TARGET
# ============================================================================
juce_add_plugin(${PROJECT_NAME}
    # Basic Info
    COMPANY_NAME "YourCompany"
    BUNDLE_ID "com.yourcompany.myplugin"
    PLUGIN_MANUFACTURER_CODE "Ycmp"
    PLUGIN_CODE "Mypl"
    
    # Version
    VERSION "${PROJECT_VERSION}"
    
    # Formats
    FORMATS ${FORMATS}
    
    # Product Name (what users see)
    PRODUCT_NAME "My Plugin"
    
    # Plugin Characteristics
    IS_SYNTH FALSE                          # TRUE for instruments
    NEEDS_MIDI_INPUT FALSE                  # TRUE if you need MIDI
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    
    # Installation
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "Ycmp"
    PLUGIN_CODE "Mypl"
    
    # Compatibility
    VST3_CAN_REPLACE_VST2 FALSE
    
    # macOS specific
    $<$<PLATFORM_ID:Darwin>:
        AU_MAIN_TYPE "kAudioUnitType_Effect"
    >
)

# ============================================================================
# TARGET CONFIGURATION
# ============================================================================

# Generate JUCE header
juce_generate_juce_header(${PROJECT_NAME})

# Add source files
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE Settings
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        
        # Plugin Settings
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        
        # Performance
        JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=0
        
        # Platform-specific
        $<$<PLATFORM_ID:Windows>:
            _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        >
)

# Link JUCE modules
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ============================================================================
# COMPILER WARNINGS
# ============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                     # Warning level 4
        /WX-                    # Don't treat warnings as errors
        /permissive-            # Standards conformance
        /MP                     # Multi-processor compilation
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wformat=2
        
        # Disable specific warnings
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    # Extra flags for Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O3
            -ffast-math
        )
    endif()
endif()

# ============================================================================
# INSTALLATION
# ============================================================================
if(DEFINED VST3_INSTALL_DIR)
    install(TARGETS ${PROJECT_NAME}_VST3
        LIBRARY DESTINATION "${VST3_INSTALL_DIR}"
        RUNTIME DESTINATION "${VST3_INSTALL_DIR}"
    )
endif()

if(APPLE AND DEFINED AU_INSTALL_DIR)
    install(TARGETS ${PROJECT_NAME}_AU
        LIBRARY DESTINATION "${AU_INSTALL_DIR}"
    )
endif()

# ============================================================================
# IDE SUPPORT
# ============================================================================
# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_Standalone
)

# Organize files in IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source 
    PREFIX "Source" 
    FILES ${SourceFiles}
)

# ============================================================================
# BUILD INFORMATION
# ============================================================================
message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Plugin formats: ${FORMATS}")
if(APPLE)
    message(STATUS "macOS Universal Binary: ${UNIVERSAL_BINARY}")
endif()
message(STATUS "===================================")
message(STATUS "")
