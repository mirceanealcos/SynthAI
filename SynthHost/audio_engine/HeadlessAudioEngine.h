#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

#include "../utils/serum/Presets.h"
#include "../midi/MidiInputCollector.h"
#include "utils/AudioRingBuffer.h"

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 1024

class HeadlessAudioEngine
{
public:
    explicit HeadlessAudioEngine(double sampleRate = 48000.0, int blockSize = 1024);
    ~HeadlessAudioEngine();

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin);
    void setPreset(Preset preset);

    void start();
    void stop();

    std::shared_ptr<AudioRingBuffer> getRingBuffer() const { return ringBuffer; }

    friend class InternalCallback;

private:
    double sampleRate;
    int blockSize;

    std::shared_ptr<AudioRingBuffer> ringBuffer;
    juce::AudioBuffer<float> audioBuffer;

    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::AudioDeviceManager deviceManager;
    MidiInputCollector midiInputCollector;
    std::unique_ptr<juce::AudioIODeviceCallback> callback;
};
