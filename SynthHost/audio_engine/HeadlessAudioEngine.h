#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include "../utils/serum/Presets.h"
#include "../midi/MidiInputCollector.h"
#include "utils/AudioRingBuffer.h"

// Forward declare the callback class
class InternalCallback;

class HeadlessAudioEngine {
public:
    explicit HeadlessAudioEngine(double sampleRate, int blockSize);

    ~HeadlessAudioEngine();

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> p);

    void setPreset(Preset preset);

    void setMidiSenderClient(std::shared_ptr<WebSocketClient> sender);

    void setMidiRole(std::string role);

    void start();

    void stop();

    void enableAIMidiInjection(bool e);

    void enqueueMidi(const juce::MidiMessage& m, int delaySamples);

    std::shared_ptr<AudioRingBuffer> getRingBuffer() const { return ringBuffer; }

    friend class InternalCallback;

private:
    double sampleRate;
    int blockSize;

    juce::AudioDeviceManager deviceManager;
    MidiInputCollector midiInputCollector;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    std::unique_ptr<juce::AudioIODeviceCallback> callback;
    std::shared_ptr<AudioRingBuffer> ringBuffer;

    std::mutex pendingMutex;
    std::vector<std::pair<int, juce::MidiMessage> > pendingMidi;
    bool shouldInjectAI = false;
};
