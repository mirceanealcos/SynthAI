//
// Created by Mircea Nealcos on 2/19/2025.
//

#ifndef SPEAKERAUDIOENGINE_H
#define SPEAKERAUDIOENGINE_H
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../utils/serum/Presets.h"

#include "../midi/MidiInputCollector.h"
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512

// LEGACY AUDIO ENGINE FOR PLAYING SOUND THROUGH SPEAKERS
class SpeakerAudioEngine : public juce::AudioIODeviceCallback {
public:
    SpeakerAudioEngine();
    ~SpeakerAudioEngine() override;
    void start();

    void stop();

    //========================================================
    // Called by the device manager on every audio block
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext &context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice *device) override;

    void audioDeviceStopped() override;

    void setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin);

    MidiInputCollector& getMidiInputCollector();

    void setPreset(Preset preset);

private:
    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    MidiInputCollector midiInputCollector;
};


#endif //SPEAKERAUDIOENGINE_H
