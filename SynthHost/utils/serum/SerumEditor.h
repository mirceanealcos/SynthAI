//
// Created by Mircea Nealcos on 2/22/2025.
//

#ifndef SERUMEDITOR_H
#define SERUMEDITOR_H
#include <juce_audio_processors/juce_audio_processors.h>

#include "Presets.h"


class SerumEditor {
public:
    static void loadSerumPreset(const Preset& preset, juce::AudioPluginInstance* serumInstance);
};



#endif //SERUMEDITOR_H
