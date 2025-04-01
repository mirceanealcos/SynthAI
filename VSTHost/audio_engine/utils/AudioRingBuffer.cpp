#include "AudioRingBuffer.h"
#include <algorithm>

AudioRingBuffer::AudioRingBuffer(int channels, int capacitySamples)
    : channels_(channels)
{
    // capacity in frames * channels
    totalSize_ = capacitySamples * channels_;
    data_.resize(totalSize_, 0.0f);
}

void AudioRingBuffer::write(const juce::AudioBuffer<float>& buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const int numChannels = std::min(channels_, buffer.getNumChannels());
    const int numFrames   = buffer.getNumSamples();

    for (int frame = 0; frame < numFrames; frame++)
    {
        for (int ch = 0; ch < numChannels; ch++)
        {
            float sample = buffer.getSample(ch, frame);
            // convert (writePos_ is in samples, so multiply frames by channels if needed)
            data_[writePos_] = sample;
            writePos_ = (writePos_ + 1) % totalSize_;
            // Overwrite the readPos_ if we run out of space (not the best approach, but simple)
            if (writePos_ == readPos_)
                readPos_ = (readPos_ + channels_) % totalSize_;
        }
    }
}

int AudioRingBuffer::read(float* destination, int samplesToRead)
{
    std::lock_guard<std::mutex> lock(mutex_);

    int samplesRead = 0;
    for (int i = 0; i < samplesToRead; i++)
    {
        if (readPos_ == writePos_)
        {
            // buffer empty
            break;
        }
        destination[i] = data_[readPos_];
        readPos_ = (readPos_ + 1) % totalSize_;
        samplesRead++;
    }

    // If ring buffer is smaller than requested, fill remainder with zeros
    for (int i = samplesRead; i < samplesToRead; i++)
        destination[i] = 0.0f;

    return samplesRead;
}
