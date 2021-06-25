cmake_minimum_required(VERSION 3.12)
project(SNRD303 VERSION 0.0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(JUCE)
juce_add_gui_app(SNRD303
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "SNRD303")     # The name of the final executable, which can differ from the target name

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

# juce_generate_juce_header(GuiAppExample)

target_sources(SNRD303
    PRIVATE
	src/Main.cpp
        src/MainComponent.cpp
	src/WavetableGenerator.cpp
	src/Sequencer.cpp)

target_include_directories(SNRD303
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	)

target_compile_definitions(SNRD303
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SNRD303,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SNRD303,JUCE_VERSION>")

target_link_libraries(SNRD303
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
	juce::juce_audio_utils
	juce::juce_events
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
