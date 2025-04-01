#pragma once
#include <vector>
#include <mutex>
#include <juce_audio_basics/juce_audio_basics.h>

class AudioRingBuffer
{
public:
    AudioRingBuffer(int channels = 2, int capacitySamples = 48000); // 1 second default
    ~AudioRingBuffer() = default;

    // Write from a JUCE AudioBuffer
    void write(const juce::AudioBuffer<float>& buffer);

    // Read into a float array. E.g., 10ms (480 frames) for stereo => 960 samples
    int read(float* destination, int samplesToRead);

private:
    std::mutex mutex_;
    int channels_;
    std::vector<float> data_;
    int writePos_ = 0;
    int readPos_ = 0;
    int totalSize_ = 0;
};
