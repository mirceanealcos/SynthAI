#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

#include "../utils/serum/Presets.h"
#include "../midi/MidiInputCollector.h"
#include "utils/AudioRingBuffer.h"

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512


class HeadlessAudioEngine
{
public:
    explicit HeadlessAudioEngine(double sampleRate = 48000.0, int blockSize = 512);
    ~HeadlessAudioEngine();

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin);
    void setPreset(Preset preset);

    // Start/stop the render thread
    void start();
    void stop();

    // Provide access to the ring buffer (for the WebRTC module)
    std::shared_ptr<AudioRingBuffer> getRingBuffer() const { return ringBuffer; }

private:
    void renderThreadFunc();

    double sampleRate;
    int blockSize;

    std::shared_ptr<AudioRingBuffer> ringBuffer;
    std::thread renderThread;
    std::atomic<bool> running { false };

    juce::AudioBuffer<float> audioBuffer;
    juce::MidiBuffer midiBuffer;

    std::unique_ptr<juce::AudioPluginInstance> plugin;

    MidiInputCollector midiInputCollector;
    std::vector<std::unique_ptr<juce::MidiInput>> midiInputs;


};