//
// Created by Mircea Nealcos on 2/19/2025.
//

#ifndef MIDIINPUTCOLLECTOR_H
#define MIDIINPUTCOLLECTOR_H
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../websocket/WebSocketClient.h"

class MidiInputCollector: public juce::MidiInputCallback {
public:
    MidiInputCollector() = default;
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void removeNextBlockOfMessages(juce::MidiBuffer& destBuffer, int numSamples);
    juce::MidiMessageCollector& getMidiMessageCollector();
    void setMidiSenderClient(std::shared_ptr<WebSocketClient> sender);
private:
    void logMidiMessage(const juce::MidiMessage& message);
    juce::MidiMessageCollector midiCollector;
    std::shared_ptr<WebSocketClient> midiSenderClient;
};



#endif //MIDIINPUTCOLLECTOR_H
