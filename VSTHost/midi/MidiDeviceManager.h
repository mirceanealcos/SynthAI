//
// Created by Mircea Nealcos on 2/19/2025.
//

#ifndef MIDIDEVICEMANAGER_H
#define MIDIDEVICEMANAGER_H
#include "MidiInputCollector.h"

class MidiDeviceManager {
public:
    void openMidiDevice(MidiInputCollector& midiCollector);
    std::unique_ptr<juce::MidiInput> midiInput;
};



#endif //MIDIDEVICEMANAGER_H
