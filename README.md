# JUCE + CMake: Common Issues & Best Practices

Some of the frequent troubleshooting experience accumulated building cross-platform audio plugins with JUCE and CMake just for myself.

---

## 🚨 Two Major Recurring Issues

### 1. The `JuceHeader.h` Problem

**What changed:**
- **Projucer era (JUCE 5-6)**: Used monolithic `#include <JuceHeader.h>`
- **CMake era (JUCE 7+)**: Must include specific module headers

**Old Way (No longer works with CMake):**
```cpp
#include <JuceHeader.h>  // Don't do this!
```

**New Way (CMake-compatible):**
```cpp
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
// Include only the modules you actually use
```

**Why this matters:**
- CMake builds don't generate `JuceHeader.h` by default
- Explicit includes improve compile times
- Better dependency management
- Forces you to be explicit about what you're using

---

### 2. CMakeLists.txt Structure Issues

The most common mistakes that cause cryptic linker errors:

#### ❌ **Problem: Inconsistent Target Names**
```cmake
juce_add_plugin(DynamicEQ    # Creates target "DynamicEQ"
    # ...
)

juce_generate_juce_header(PurrSynth)  # ❌ Wrong target name!
target_sources(PurrSynth PRIVATE ...)   # ❌ Target doesn't exist!
```

**Error:** `get_target_property() called with non-existent target`

#### ✅ **Solution: Consistent Target Names**
```cmake
juce_add_plugin(PurrSynth    # Creates target "PurrSynth"
    # ...
)

juce_generate_juce_header(PurrSynth)  # ✅ Same name
target_sources(PurrSynth PRIVATE ...)   # ✅ Same name
```

---

#### ❌ **Problem: Missing `createPluginFilter()` Function**
```cpp
// PurrSynthProcessor.cpp - missing the entry point!
```

**Error:** `undefined reference to 'createPluginFilter()'`

#### ✅ **Solution: Add Plugin Entry Point**
```cpp
// At the bottom of your processor .cpp file:
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PurrSynthAudioProcessor();
}
```

Also need these in your processor class:
```cpp
// In .h file:
juce::AudioProcessorEditor* createEditor() override;
bool hasEditor() const override;

// In .cpp file:
juce::AudioProcessorEditor* PurrSynthAudioProcessor::createEditor()
{
    return new PurrSynthAudioProcessorEditor(*this);
}

bool PurrSynthAudioProcessor::hasEditor() const
{
    return true;
}
```

---

## 📋 Correct CMakeLists.txt Structure

Here's the proper order and structure for a cross-platform JUCE plugin:

```cmake
cmake_minimum_required(VERSION 3.15)
project(YourPlugin VERSION 1.0.0)

# 1. BUILD CONFIGURATION
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# 2. PLATFORM-SPECIFIC SETTINGS
if (APPLE)
    option(MAC_UNIVERSAL "Build universal binary (arm64+x86_64)" ON)
    
    if (MAC_UNIVERSAL)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
    endif()
endif()

# 3. LOCATE JUCE
if (NOT DEFINED JUCE_DIR OR NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    if (WIN32)
        set(JUCE_DIR "D:/AudioDev/JUCE")
    else()
        set(JUCE_DIR "$ENV{HOME}/JUCE")
    endif()
    
    if (NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}")
    endif()
endif()

add_subdirectory(${JUCE_DIR} JUCE)

# 4. PLUGIN FORMATS
set(PLUGIN_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

# 5. COPY DESTINATIONS
if (WIN32)
    file(TO_CMAKE_PATH "$ENV{APPDATA}/VST3" VST3_COPY_DIR)
elseif(APPLE)
    set(VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(AU_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
else() # Linux
    set(VST3_COPY_DIR "$ENV{HOME}/.vst3")
endif()

# 6. SOURCE FILES
set(SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# 7. CREATE PLUGIN TARGET (Must come BEFORE configuration!)
juce_add_plugin(YourPlugin
    COMPANY_NAME "YourCompany"
    BUNDLE_ID com.yourcompany.yourplugin
    IS_SYNTH TRUE                    # Set appropriately
    NEEDS_MIDI_INPUT TRUE            # Set appropriately
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Plgn
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "Your Plugin"
)

# 8. CONFIGURE TARGET (Must use SAME name as above!)
juce_generate_juce_header(YourPlugin)

target_sources(YourPlugin PRIVATE ${SOURCES})

target_include_directories(YourPlugin 
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/Source
)

target_compile_features(YourPlugin PRIVATE cxx_std_17)

target_compile_definitions(YourPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# 9. LINK JUCE MODULES
target_link_libraries(YourPlugin
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

# 10. COPY SETTINGS
if (DEFINED VST3_COPY_DIR)
    set_target_properties(YourPlugin PROPERTIES 
        VST3_COPY_DIR "${VST3_COPY_DIR}"
    )
endif()

if (APPLE AND DEFINED AU_COPY_DIR)
    set_target_properties(YourPlugin PROPERTIES 
        AU_COPY_DIR "${AU_COPY_DIR}"
    )
endif()
```

---

## 🎯 Key Principles

### 1. **Order Matters**
```cmake
# ✅ CORRECT ORDER:
juce_add_plugin(MyPlugin ...)           # Create target FIRST
juce_generate_juce_header(MyPlugin)     # Then configure it
target_sources(MyPlugin ...)            # Then add sources
target_link_libraries(MyPlugin ...)     # Then link libraries

# ❌ WRONG - will fail:
juce_generate_juce_header(MyPlugin)     # Target doesn't exist yet!
juce_add_plugin(MyPlugin ...)           # Too late!
```

### 2. **Consistency is Critical**
- Target name in `juce_add_plugin()` must match everywhere
- One typo = cryptic linker errors
- Use a variable if you want to rename easily:
  ```cmake
  set(TARGET_NAME "MyPlugin")
  juce_add_plugin(${TARGET_NAME} ...)
  target_sources(${TARGET_NAME} ...)
  ```

### 3. **Required Plugin Components**
Every JUCE plugin needs:
1. **Processor class** (inherits from `juce::AudioProcessor`)
2. **Editor class** (inherits from `juce::AudioProcessorEditor`)
3. **Entry point function** (`createPluginFilter()`)
4. **CMakeLists.txt** with correct target configuration

---

## 🛠️ Building Commands

### Modern CMake Way (Recommended)
```bash
# Configure and build in one go
cmake -B build && cmake --build build -j$(nproc)

# Or step by step
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### Clean Rebuild
```bash
rm -rf build
cmake -B build && cmake --build build -j$(nproc)
```

---

## 🐛 Debugging Checklist

When you get build errors, check:

1. ✅ **Target name is consistent** throughout CMakeLists.txt
2. ✅ **`juce_add_plugin()` comes before** `juce_generate_juce_header()`
3. ✅ **No `#include <JuceHeader.h>`** in any source files
4. ✅ **`createPluginFilter()` exists** in processor .cpp file
5. ✅ **`createEditor()` and `hasEditor()` implemented**
6. ✅ **All source files listed** in `target_sources()`
7. ✅ **Required JUCE modules linked** in `target_link_libraries()`

---

## 📚 Common JUCE Modules

```cmake
# Audio processing
juce::juce_audio_processors     # Required for plugins
juce::juce_audio_utils          # AudioDeviceManager, etc.
juce::juce_dsp                  # Filters, reverb, etc.

# GUI
juce::juce_gui_basics           # Basic GUI components
juce::juce_gui_extra            # WebBrowser, etc.

# Core
juce::juce_core                 # String, Array, File, etc.
juce::juce_data_structures      # ValueTree, etc.
juce::juce_events               # Timers, messages
```

---

## 💡 Pro Tips

1. **Use variables for paths**:
   ```cmake
   set(JUCE_DIR "$ENV{HOME}/JUCE" CACHE PATH "JUCE directory")
   ```

2. **Enable parallel builds**:
   ```bash
   make -j$(nproc)  # Linux/Mac
   cmake --build build -j8  # Cross-platform
   ```

3. **Test on all platforms early** - don't wait until the end

4. **Use `COPY_PLUGIN_AFTER_BUILD TRUE`** to auto-install plugins for testing

5. **Keep CMakeLists.txt simple** - don't over-complicate it

---

## 📖 Additional Resources

- [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- [JUCE Tutorials](https://juce.com/learn/tutorials)
- [JUCE Forum](https://forum.juce.com/)

---

**Remember:** The two most common issues are:
1. Using `JuceHeader.h` instead of specific module headers
2. Inconsistent target names in CMakeLists.txt

Fix these,  90% of build problems disappear! 
