//
// Created by Mircea Nealcos on 2/19/2025.
//

#include "SpeakerAudioEngine.h"
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 256

SpeakerAudioEngine::SpeakerAudioEngine()
{
}

SpeakerAudioEngine::~SpeakerAudioEngine() {
    stop();
}


void SpeakerAudioEngine::start() {
    // Set up device
    deviceManager.initialise(1, 2, nullptr, true);

    auto allDevices = juce::MidiInput::getAvailableDevices();
    for (auto& dev : allDevices)
    {
        if (dev.name.containsIgnoreCase("Minilab3 MIDI"))  // or something matching your device
        {
            // Enable that specific device
            deviceManager.setMidiInputDeviceEnabled (dev.identifier, true);

            // Now add our midiInputCollector as a callback for that device only
            deviceManager.addMidiInputDeviceCallback (dev.identifier, &midiInputCollector);
            std::cout << "Opened specific MIDI device: " << dev.name << std::endl;
            break;
        }
    }

    deviceManager.addAudioCallback(this);

    // Prepare the plugin
    if (plugin)
        plugin->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE); // Or your actual sampleRate, blockSize
}

void SpeakerAudioEngine::stop() {
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
    deviceManager.removeMidiInputDeviceCallback({}, &midiInputCollector);

    if (plugin)
        plugin->releaseResources();
}

//========================================================
// Called by the device manager on every audio block
void SpeakerAudioEngine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                      int numInputChannels,
                                      float *const*outputChannelData,
                                      int numOutputChannels,
                                      int numSamples,
                                      const juce::AudioIODeviceCallbackContext &context) {
    if (plugin) {
        // Wrap the output channels into a Juce AudioBuffer
        juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
        buffer.clear();

        // Build or retrieve a MidiBuffer with note events
        juce::MidiBuffer midi;
        midiInputCollector.removeNextBlockOfMessages(midi, numSamples);

        // Let the plugin process
        plugin->processBlock(buffer, midi);

        // Done! The data in 'buffer' is automatically passed to the output device
    } else {
        // If no plugin, clear outputs so we don't make noise
        for (int ch = 0; ch < numOutputChannels; ++ch)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
    }
}

void SpeakerAudioEngine::audioDeviceAboutToStart(juce::AudioIODevice *device) {
    const double sampleRate = device->getCurrentSampleRate();
    const int blockSize = device->getCurrentBufferSizeSamples();
    if (plugin)
        plugin->prepareToPlay(sampleRate, blockSize);
    midiInputCollector.getMidiMessageCollector().reset(SAMPLE_RATE);
}

void SpeakerAudioEngine::audioDeviceStopped() {
    if (plugin)
        plugin->releaseResources();
}

void SpeakerAudioEngine::setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin) {
    plugin->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
    this->plugin = std::move(plugin);
}

MidiInputCollector &SpeakerAudioEngine::getMidiInputCollector() {
    return midiInputCollector;
}
