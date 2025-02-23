#include "vst_hosting/PluginManager.h"
#include "utils/PluginEnum.h"
#include "audio_engine/SpeakerAudioEngine.h"
#include "midi/MidiDeviceManager.h"

int main() {

    PluginManager manager;
    juce::String errorMsg;
    auto instance = manager.loadPlugin(PluginEnum::SERUM, 48000, 1024, errorMsg);
    SpeakerAudioEngine speakerAudioEngine;
    MidiDeviceManager midiDeviceManager;
    speakerAudioEngine.start();
    speakerAudioEngine.setPlugin(std::move(instance));
    while (true)
    {
        juce::Thread::sleep(100);
    }
}
