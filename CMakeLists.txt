cmake_minimum_required(VERSION 3.22)

project(Bowie VERSION 0.4.5 LANGUAGES C CXX)

set(BOWIE_JUCE_SOURCE_DIR "" CACHE PATH "Path to a JUCE 9.0.1 checkout")

if(NOT BOWIE_JUCE_SOURCE_DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
        set(BOWIE_JUCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    elseif(WIN32 AND EXISTS "F:/JUCE/CMakeLists.txt")
        set(BOWIE_JUCE_SOURCE_DIR "F:/JUCE")
    else()
        message(FATAL_ERROR
            "JUCE was not found. Pass -DBOWIE_JUCE_SOURCE_DIR=/path/to/JUCE-9.0.1")
    endif()
endif()

add_subdirectory("${BOWIE_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(Bowie
    VERSION 0.4.5
    COMPANY_NAME "MonkeMountainStudios"
    COMPANY_WEBSITE "https://monkemountainstudios.neocities.org"
    BUNDLE_ID "com.monkemountainstudios.bowie"
    PLUGIN_MANUFACTURER_CODE Mnke
    PLUGIN_CODE Bwie
    FORMATS VST3 Standalone
    PRODUCT_NAME "Bowie"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/BowieIcon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Assets/BowieIcon.png"
)

juce_generate_juce_header(Bowie)

juce_add_binary_data(BowieAssets
    SOURCES
        Samples/scraping_bow_hp90.wav
        Samples/long_soft_bow_hp250.wav
        "Samples/short bow noise.wav"
        Assets/BowieIcon.png
        Assets/Fonts/BrunoAce-Official.ttf
        Assets/Fonts/Fancytext.ttf
        "Samples/Tones/Bassoon/Bassoon C2.wav"
        "Samples/Tones/Bassoon/Bassoon C3.wav"
        "Samples/Tones/Bassoon/Bassoon C4.wav"
        "Samples/Tones/Church/Church C2.wav"
        "Samples/Tones/Church/Church C3.wav"
        "Samples/Tones/Church/Church C4.wav"
        "Samples/Tones/English Horm/English C3.wav"
        "Samples/Tones/English Horm/English C4.wav"
        "Samples/Tones/Flute/Flute C3.wav"
        "Samples/Tones/Flute/Flute C4.wav"
        "Samples/Tones/Flute/Flute C5.wav"
        "Samples/Tones/Horn/Horn C2.wav"
        "Samples/Tones/Horn/Horn C3.wav"
        "Samples/Tones/Horn/Horn C4.wav"
        "Samples/Tones/Piano/Piano C2.wav"
        "Samples/Tones/Piano/Piano C3.wav"
        "Samples/Tones/Piano/PIano C4.wav"
        "Samples/Tones/Rhodes/Rhodes C2.wav"
        "Samples/Tones/Rhodes/Rhodes C3.wav"
        "Samples/Tones/Rhodes/Rhodes C4.wav"
        "Samples/Tones/Trumpet/Trumpet C2.wav"
        "Samples/Tones/Trumpet/Trumpet C3.wav"
        "Samples/Tones/Trumpet/Trumpet C4.wav"
        "Samples/Tones/Tuba/Tuba C2.wav"
        "Samples/Tones/Tuba/Tuba C3.wav"
        "Samples/Tones/Tuba/Tuba C4.wav"
        "Samples/Tones/Vibra/Vibra C2.wav"
        "Samples/Tones/Vibra/Vibra C3.wav"
        "Samples/Tones/Vibra/Vibra C4.wav"
)

target_sources(Bowie PRIVATE
    Source/BowieVoice.cpp
    Source/BowieVoice.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_compile_features(Bowie PRIVATE cxx_std_17)

target_compile_definitions(Bowie PUBLIC
    JUCE_ASIO=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Bowie PRIVATE
    BowieAssets
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
)

target_sources(Bowie_Standalone PRIVATE Source/StandaloneApp.cpp)
target_compile_definitions(Bowie_Standalone PRIVATE JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1)

juce_add_console_app(BowieSmokeTest PRODUCT_NAME "Bowie Smoke Test")
juce_generate_juce_header(BowieSmokeTest)
target_sources(BowieSmokeTest PRIVATE Tests/SmokeTest.cpp)
target_include_directories(BowieSmokeTest PRIVATE Source)
target_compile_features(BowieSmokeTest PRIVATE cxx_std_17)
target_link_libraries(BowieSmokeTest PRIVATE
    Bowie
    BowieAssets
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)
