//
// Created by Mircea Nealcos on 2/19/2025.
//

#include "MidiDeviceManager.h"


void MidiDeviceManager::openMidiDevice(MidiInputCollector &midiCollector) {
    auto devices = juce::MidiInput::getAvailableDevices();
    for (auto& device : devices) {
        if (device.name.containsIgnoreCase("Minilab3 MIDI")) {
            if (auto input = juce::MidiInput::openDevice(device.identifier, &midiCollector.getMidiMessageCollector())) {
                if (input == nullptr) {
                    std::cout << "Error opening MIDI device: " << device.identifier << std::endl;
                    return;
                }
                midiInput = std::move(input);
                midiInput->start();
                std::cout << "Listening to MIDI input from " << device.name << std::endl;
            }
        }
    }
}
