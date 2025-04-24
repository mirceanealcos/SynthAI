cmake_minimum_required(VERSION 3.30)
project(SynthHost)

find_package(Boost 1.88.0 REQUIRED
        COMPONENTS system thread random chrono asio
)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(modules/JUCE)

set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules/vst3sdk" CACHE PATH "Path to VST3 SDK")
if(NOT EXISTS "${VST3_SDK_ROOT}/pluginterfaces/base/fplatform.h")
    message(FATAL_ERROR "VST3cmaSDK not found or incomplete at ${VST3_SDK_ROOT}")
endif()

set(JUCE_VST3_SDK_PATH "${VST3_SDK_ROOT}")

add_executable(SynthHost main.cpp
        vst_hosting/PluginManager.cpp
        vst_hosting/PluginManager.h
        utils/PluginEnum.h
        utils/PluginEnum.cpp
        audio_engine/SpeakerAudioEngine.cpp
        audio_engine/SpeakerAudioEngine.h
        midi/MidiInputCollector.cpp
        midi/MidiInputCollector.h
        midi/MidiDeviceManager.cpp
        midi/MidiDeviceManager.h
        utils/serum/Presets.cpp
        utils/serum/Presets.h
        utils/serum/SerumEditor.cpp
        utils/serum/SerumEditor.h
        audio_engine/HeadlessAudioEngine.cpp
        audio_engine/HeadlessAudioEngine.h
        audio_engine/utils/AudioRingBuffer.cpp
        audio_engine/utils/AudioRingBuffer.h)

target_compile_definitions(SynthHost
    PRIVATE
        JUCE_PLUGINHOST_VST3=1
)

target_link_libraries(SynthHost
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        Boost::system
        Boost::thread
        Boost::random
        Boost::chrono
        Boost::asio
)