#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include "../utils/serum/Presets.h"
#include "../midi/MidiInputCollector.h"
#include "utils/AudioRingBuffer.h"

// Forward declare the callback class
class InternalCallback;

class HeadlessAudioEngine
{
public:
    explicit HeadlessAudioEngine(double sampleRate, int blockSize);
    ~HeadlessAudioEngine();

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> p);
    void setPreset(Preset preset);

    void start();
    void stop();

    /** Raw interleaved stereo floats: frames * 2 samples */
    std::shared_ptr<AudioRingBuffer> getRingBuffer() const { return ringBuffer; }

    // Grant InternalCallback access to private members
    friend class InternalCallback;

private:
    double sampleRate;
    int    blockSize;

    juce::AudioDeviceManager                   deviceManager;
    MidiInputCollector                         midiInputCollector;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    std::unique_ptr<juce::AudioIODeviceCallback> callback;
    std::shared_ptr<AudioRingBuffer>           ringBuffer;
};
