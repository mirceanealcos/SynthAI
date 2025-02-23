//
// Created by Mircea Nealcos on 2/22/2025.
//

#include "SerumEditor.h"


void SerumEditor::loadSerumPreset(const Preset& preset, juce::AudioPluginInstance* serumInstance) {
    if (serumInstance == nullptr) {
        std::cout << "Serum instance is null!" << std::endl;
        return;
    }
    juce::String presetPath (preset.path);
    juce::File presetFile (presetPath);
    if (!presetFile.existsAsFile()) {
        std::cout << "File " << presetPath << " was not found!" << std::endl;
    }
    juce::MemoryBlock presetBlock;
    if (presetFile.loadFileAsData(presetBlock)) {
        juce::VST3PluginFormat::setStateFromVSTPresetFile(serumInstance, presetBlock);
        // serumInstance->setStateInformation(presetBlock.getData(), (int) presetBlock.getSize());
        std::cout << "Preset: " << presetPath << " was loaded successfully into the Serum instance!" << std::endl;
    }
    else {
        std::cout << "Preset " << presetPath << " failed to load!" << std::endl;
    }
}
