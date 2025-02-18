cmake_minimum_required(VERSION 3.30)
project(VSTHost)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(modules/JUCE)

set(WEBRTC_ROOT "D:/Software/WebRTC/webrtc-checkout/src")

include_directories(${WEBRTC_ROOT})
include_directories(${WEBRTC_ROOT}/out/Release)

set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules/vst3sdk" CACHE PATH "Path to VST3 SDK")
if(NOT EXISTS "${VST3_SDK_ROOT}/pluginterfaces/base/fplatform.h")
    message(FATAL_ERROR "VST3cmaSDK not found or incomplete at ${VST3_SDK_ROOT}")
endif()

set(JUCE_VST3_SDK_PATH "${VST3_SDK_ROOT}")

add_executable(VSTHost main.cpp
        vst_hosting/PluginManager.cpp
        vst_hosting/PluginManager.h
        utils/PluginEnum.h
        utils/PluginEnum.cpp
        audio_engine/SpeakerAudioEngine.cpp
        audio_engine/SpeakerAudioEngine.h)

target_compile_definitions(VSTHost
    PRIVATE
        JUCE_PLUGINHOST_VST3=1
)

target_link_libraries(VSTHost
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
)