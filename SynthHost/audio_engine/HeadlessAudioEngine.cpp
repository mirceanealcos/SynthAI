#include "HeadlessAudioEngine.h"
#include "./utils/AudioRingBuffer.h"
#include "../utils/serum/SerumEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

class InternalCallback : public juce::AudioIODeviceCallback {
public:
    InternalCallback(HeadlessAudioEngine *owner) : owner(owner) {
    }

    void audioDeviceIOCallbackWithContext(const float* const*, int,
                                      float* const* outputChannelData,
                                      int numOutputChannels,
                                      int numSamples,
                                      const juce::AudioIODeviceCallbackContext&) override
    {
        if (!owner->plugin) return;

        juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
        buffer.clear(); // mute actual output

        juce::MidiBuffer midi;
        owner->midiInputCollector.removeNextBlockOfMessages(midi, numSamples);
        owner->plugin->processBlock(buffer, midi);

        // Downmix to mono and write to ring buffer
        juce::AudioBuffer<float> mono(1, buffer.getNumSamples());
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            float l = buffer.getNumChannels() > 0 ? buffer.getSample(0, i) : 0.0f;
            float r = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : 0.0f;
            mono.setSample(0, i, 0.5f * (l + r));
        }

        owner->ringBuffer->write(mono);

        for (int ch = 0; ch < numOutputChannels; ++ch) {
            if (outputChannelData[ch] != nullptr)
                juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
        }
    }


    void audioDeviceAboutToStart(juce::AudioIODevice *device) override {
        owner->plugin->prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
        owner->midiInputCollector.getMidiMessageCollector().reset(device->getCurrentSampleRate());
    }

    void audioDeviceStopped() override {
        if (owner->plugin) owner->plugin->releaseResources();
    }

private:
    HeadlessAudioEngine *owner;
};

HeadlessAudioEngine::HeadlessAudioEngine(double sr, int bs)
    : sampleRate(sr), blockSize(bs) {
    ringBuffer = std::make_shared<AudioRingBuffer>(1, 2 * blockSize);
    audioBuffer.setSize(2, 2 * blockSize);
    callback = std::make_unique<InternalCallback>(this);
}

HeadlessAudioEngine::~HeadlessAudioEngine() {
    stop();
}

void HeadlessAudioEngine::setPlugin(std::unique_ptr<juce::AudioPluginInstance> p) {
    plugin = std::move(p);
    plugin->prepareToPlay(sampleRate, blockSize);
}

void HeadlessAudioEngine::setPreset(Preset preset) {
    SerumEditor::loadSerumPreset(preset, this->plugin.get());
}

void HeadlessAudioEngine::start()
{
    if (!plugin)
        return;

    // Allow mic access on macOS/iOS
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [](bool) {});

    // Initialize the real output device (2 out channels)
    deviceManager.initialise(
        0, // no input channels
        2, // stereo output (needed to trigger audio callbacks)
        nullptr,
        true
    );

    // Attach MIDI input if available
    auto midiDevices = juce::MidiInput::getAvailableDevices();
    for (auto& dev : midiDevices) {
        if (dev.name.containsIgnoreCase("Minilab3 MIDI")) {
            deviceManager.setMidiInputDeviceEnabled(dev.identifier, true);
            deviceManager.addMidiInputDeviceCallback(dev.identifier, &midiInputCollector);
            std::cout << "MIDI Input enabled: " << dev.name << std::endl;
            break;
        }
    }

    deviceManager.addAudioCallback(callback.get());

    // Log device info
    if (auto* device = deviceManager.getCurrentAudioDevice()) {
        std::cout << "Using device: " << device->getName() << std::endl;
        std::cout << "Buffer size: " << device->getCurrentBufferSizeSamples()
                  << ", Rate: " << device->getCurrentSampleRate() << std::endl;
    }
}

void HeadlessAudioEngine::stop() {
    deviceManager.removeAudioCallback(callback.get());
    deviceManager.closeAudioDevice();

    for (auto &dev: juce::MidiInput::getAvailableDevices()) {
        deviceManager.removeMidiInputDeviceCallback(dev.identifier, &midiInputCollector);
    }

    if (plugin)
        plugin->releaseResources();
}
