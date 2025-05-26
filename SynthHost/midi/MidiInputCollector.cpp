//
// Created by Mircea Nealcos on 2/19/2025.
//

#include "MidiInputCollector.h"


void MidiInputCollector::logMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        auto note  = message.getNoteNumber();
        auto vel   = (int) message.getVelocity();
        auto ch    = message.getChannel();
        std::cout << "[MIDI] Note On - Note#: " << note
                  << ", Velocity: " << vel
                  << ", Channel: " << ch << std::endl;
    }
    else if (message.isNoteOff())
    {
        auto note = message.getNoteNumber();
        auto ch   = message.getChannel();
        std::cout << "[MIDI] Note Off - Note#: " << note
                  << ", Channel: " << ch << std::endl;
    }
    else if (message.isController())
    {
        auto ccNumber = message.getControllerNumber();
        auto ccValue  = message.getControllerValue();
        auto ch       = message.getChannel();
        std::cout << "[MIDI] CC - Controller#: " << ccNumber
                  << ", Value: " << ccValue
                  << ", Channel: " << ch << std::endl;
    }
    else
    {
        // If you want to see *all* MIDI events:
        // std::cout << "[MIDI] Unhandled: " << message.getDescription().toStdString() << std::endl;
    }
}


void MidiInputCollector::handleIncomingMidiMessage(juce::MidiInput *source, const juce::MidiMessage &message) {
    logMidiMessage(message);
    midiCollector.addMessageToQueue(message);
    if (midiSenderClient != nullptr) {
        json j;
        auto tsSeconds = juce::Time::currentTimeMillis();
        j["timestamp"] = tsSeconds;
        if (message.isNoteOn()) {
            j["type"] = "note_on";
            j["note"] = message.getNoteNumber();
            j["velocity"] = static_cast<int>(message.getVelocity() * 127.0f);
        }
        else if (message.isNoteOff())
        {
            j["type"] = "note_off";
            j["note"] = message.getNoteNumber();
            j["velocity"] = 0;
        }
        else
        {
            return;
        }
        midiSenderClient->sendJson(j);
    }
}

void MidiInputCollector::removeNextBlockOfMessages(juce::MidiBuffer &destBuffer, int numSamples) {
    midiCollector.removeNextBlockOfMessages(destBuffer, numSamples);
}

juce::MidiMessageCollector &MidiInputCollector::getMidiMessageCollector() {
    return midiCollector;
}

void MidiInputCollector::setMidiSenderClient(std::shared_ptr<WebSocketClient> sender) {
    this->midiSenderClient = sender;
}

void MidiInputCollector::setUserRole(std::string userRole) {
    this->userRole = userRole;
}


