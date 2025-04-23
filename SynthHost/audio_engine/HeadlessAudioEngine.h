#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include "../utils/serum/Presets.h"


class AudioRingBuffer;

class HeadlessAudioEngine
{
public:
    HeadlessAudioEngine(double sampleRate = 48000.0, int blockSize = 512);
    ~HeadlessAudioEngine();

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin);
    void setPreset(Preset preset);

    // Start/stop the render thread
    void start();
    void stop();

    // Provide access to the ring buffer (for the WebRTC module)
    std::shared_ptr<AudioRingBuffer> getRingBuffer() const { return ringBuffer; }

    // Optionally handle MIDI
    void setMidiMessageQueue(std::shared_ptr<std::vector<juce::MidiMessage>> midiQueue);

private:
    void renderThreadFunc();

    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    double sampleRate;
    int blockSize;

    std::thread renderThread;
    std::atomic<bool> running { false };

    std::shared_ptr<AudioRingBuffer> ringBuffer;
    std::shared_ptr<std::vector<juce::MidiMessage>> midiQueue;

    juce::AudioBuffer<float> audioBuffer;
    juce::MidiBuffer midiBuffer;

    std::unique_ptr<juce::AudioPluginInstance> plugin;

};