#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include "../utils/PluginEnum.h"
#include <juce_audio_processors/juce_audio_processors.h>


class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    std::unique_ptr<juce::AudioPluginInstance> loadPlugin (const PluginDef& plugin,
                                                           double sampleRate,
                                                           int blockSize,
                                                           juce::String& error);

private:
    juce::AudioPluginFormatManager formatManager;
};
#endif