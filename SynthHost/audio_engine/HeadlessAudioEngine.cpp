#include "HeadlessAudioEngine.h"
#include "./utils/AudioRingBuffer.h"
#include "../utils/serum/SerumEditor.h"
#include <juce_audio_formats/juce_audio_formats.h> // needed for plugin formats

HeadlessAudioEngine::HeadlessAudioEngine(double sr, int bs)
    : sampleRate(sr), blockSize(bs)
{
    ringBuffer = std::make_shared<AudioRingBuffer>(2,  2 * blockSize);

    // Prepare the buffer to store plugin output
    audioBuffer.setSize(2, 2 * blockSize); // stereo
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
    if (!plugin)
        return;
    plugin->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
    std::cout << "Successfully prepared for audio: " << plugin->getName() << std::endl;
    midiInputCollector.getMidiMessageCollector().reset(sampleRate);
    auto devices = juce::MidiInput::getAvailableDevices();
    for (auto& device : devices) {
        if (device.name.containsIgnoreCase("Minilab3 MIDI")) {
            auto inputDevice = juce::MidiInput::openDevice(device.identifier, &midiInputCollector);
            inputDevice->start();
            midiInputs.push_back(std::move(inputDevice));
            std::cout << "Successfully set input device to : " << device.name << std::endl;
        }
    }
    running.store(true);
    renderThread = std::thread([this] { renderThreadFunc(); });
}

void HeadlessAudioEngine::stop()
{
    running = false;
    for (auto& in : midiInputs) {
        in->stop();
    }
    midiInputs.clear();
    if (renderThread.joinable())
        renderThread.join();

    if (plugin)
        plugin->releaseResources();
}


void HeadlessAudioEngine::renderThreadFunc()
{
    while (running)
    {
        midiBuffer.clear();
        midiInputCollector.removeNextBlockOfMessages(midiBuffer, blockSize);
        audioBuffer.clear();
        if (plugin)
        {
            plugin->processBlock(audioBuffer, midiBuffer);
        }
        ringBuffer->write(audioBuffer);
        auto blockDurationMs = (int) std::round((blockSize / sampleRate) * 1000.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(blockDurationMs));
    }
}
