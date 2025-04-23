#include "HeadlessAudioEngine.h"
#include "./utils/AudioRingBuffer.h"
#include "../utils/serum/SerumEditor.h"
#include <juce_audio_formats/juce_audio_formats.h> // needed for plugin formats

HeadlessAudioEngine::HeadlessAudioEngine(double sr, int bs)
    : sampleRate(sr), blockSize(bs)
{
    ringBuffer = std::make_shared<AudioRingBuffer>();
    midiQueue = std::make_shared<std::vector<juce::MidiMessage>>();

    // Prepare the buffer to store plugin output
    audioBuffer.setSize(2, blockSize); // stereo
}

HeadlessAudioEngine::~HeadlessAudioEngine()
{
    stop();
}

void HeadlessAudioEngine::setPlugin(std::unique_ptr<juce::AudioPluginInstance> plugin) {
    plugin->prepareToPlay(48000, 512);
    this->plugin = std::move(plugin);
}

void HeadlessAudioEngine::setPreset(Preset preset) {
    SerumEditor::loadSerumPreset(preset, this->plugin.get());
}

void HeadlessAudioEngine::start()
{
    if (running) return;
    running = true;
    renderThread = std::thread(&HeadlessAudioEngine::renderThreadFunc, this);
}

void HeadlessAudioEngine::stop()
{
    running = false;
    if (renderThread.joinable())
        renderThread.join();

    if (pluginInstance)
        pluginInstance->releaseResources();
}

void HeadlessAudioEngine::setMidiMessageQueue(std::shared_ptr<std::vector<juce::MidiMessage>> queue)
{
    midiQueue = queue;
}

void HeadlessAudioEngine::renderThreadFunc()
{
    while (running)
    {
        // Collect any MIDI messages
        midiBuffer.clear();
        {
            // e.g., copy from midiQueue
            // For (auto& msg : *midiQueue) midiBuffer.addEvent(msg, 0);
            // midiQueue->clear();
        }

        // Clear audioBuffer
        audioBuffer.clear();

        // Let the plugin process
        if (pluginInstance)
        {
            pluginInstance->processBlock(audioBuffer, midiBuffer);
        }

        // Now audioBuffer contains the plugin's output for 'blockSize' frames
        // at sampleRate (48k).
        // Push it into the ring buffer
        ringBuffer->write(audioBuffer);

        // Sleep for blockSize / sampleRate seconds => e.g. 512/48000 ~ 10.666 ms
        auto blockDurationMs = (int) std::round((blockSize / sampleRate) * 1000.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(blockDurationMs));
    }
}
