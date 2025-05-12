#include "HeadlessAudioEngine.h"
#include "./utils/AudioRingBuffer.h"
#include "../utils/serum/SerumEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

class InternalCallback : public juce::AudioIODeviceCallback
{
public:
    InternalCallback (HeadlessAudioEngine* owner) : owner (owner) {}

    void audioDeviceIOCallbackWithContext (const float* const*, int,
                                           float* const* outputs, int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext&) override
    {
        if (! owner->plugin)
            return;

        // 1) Prepare a local stereo buffer for the plugin
        juce::AudioBuffer<float> pluginBuffer (numOutputChannels, numSamples);
        pluginBuffer.clear();

        // 2) Fetch MIDI and process
        juce::MidiBuffer midi;
        owner->midiInputCollector.removeNextBlockOfMessages (midi, numSamples);
        owner->plugin->processBlock (pluginBuffer, midi);

        // 3) Write full stereo data into ring buffer
        owner->ringBuffer->write (pluginBuffer);

        // 4) Mute actual hardware output channels
        for (int ch = 0; ch < numOutputChannels; ++ch)
            juce::FloatVectorOperations::clear (outputs[ch], numSamples);
    }

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override
    {
        owner->plugin->prepareToPlay (device->getCurrentSampleRate(),
                                      device->getCurrentBufferSizeSamples());
        owner->midiInputCollector.getMidiMessageCollector().reset (device->getCurrentSampleRate());
    }

    void audioDeviceStopped() override
    {
        if (owner->plugin)
            owner->plugin->releaseResources();
    }

private:
    HeadlessAudioEngine* owner;
};

//==============================================================================

HeadlessAudioEngine::HeadlessAudioEngine (double sr, int bs)
    : sampleRate (sr), blockSize (bs)
{
    // Now stereo: 2 channels, capacity = 2 * blockSize frames
    ringBuffer = std::make_shared<AudioRingBuffer> (2, 2 * blockSize);
    callback   = std::make_unique<InternalCallback> (this);
}

HeadlessAudioEngine::~HeadlessAudioEngine()
{
    stop();
}

void HeadlessAudioEngine::setPlugin (std::unique_ptr<juce::AudioPluginInstance> p)
{
    plugin = std::move (p);
    plugin->prepareToPlay (sampleRate, blockSize);
}

void HeadlessAudioEngine::setPreset (Preset preset)
{
    SerumEditor::loadSerumPreset (preset, plugin.get());
}

void HeadlessAudioEngine::start()
{
    if (! plugin)
        return;

    deviceManager.initialise (0, 2, nullptr, true);

    for (auto& dev : juce::MidiInput::getAvailableDevices())
    {
        if (dev.name.containsIgnoreCase ("Minilab3 MIDI"))
        {
            deviceManager.setMidiInputDeviceEnabled (dev.identifier, true);
            deviceManager.addMidiInputDeviceCallback (dev.identifier,
                                                      &midiInputCollector);
            break;
        }
    }

    deviceManager.addAudioCallback (callback.get());

    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        std::cout << "Using device: " << device->getName()
                  << " | BufSize: " << device->getCurrentBufferSizeSamples()
                  << " | Rate: "    << device->getCurrentSampleRate()
                  << std::endl;
    }
}

void HeadlessAudioEngine::stop()
{
    deviceManager.removeAudioCallback (callback.get());
    deviceManager.closeAudioDevice();

    for (auto& dev : juce::MidiInput::getAvailableDevices())
        deviceManager.removeMidiInputDeviceCallback (dev.identifier,
                                                   &midiInputCollector);

    if (plugin)
        plugin->releaseResources();
}
