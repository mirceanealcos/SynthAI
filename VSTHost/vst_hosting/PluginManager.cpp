#include "PluginManager.h"
#include "../utils/PluginEnum.h"
#include <juce_audio_processors/juce_audio_processors.h>
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 1024

using namespace juce;

PluginManager::PluginManager()
{
    formatManager.addDefaultFormats();
}

PluginManager::~PluginManager()
{
}

std::unique_ptr<AudioPluginInstance> PluginManager::loadPlugin(
    const PluginDef& plugin, const double sampleRate, const int blockSize, String& error)
{
    OwnedArray<PluginDescription> descriptions;
    KnownPluginList pluginList;
    for (int i = 0; i < formatManager.getNumFormats(); ++i)
    {
        pluginList.scanAndAddFile(plugin.path, true, descriptions,
                             *formatManager.getFormat(i));
    }


    if (descriptions.size() == 0)
    {
        std::cout<<"No plugin found at " + plugin.path<< std::endl;
        return nullptr;
    }

    auto instance = formatManager.createPluginInstance(*descriptions[0],
                                                        sampleRate,
                                                        blockSize,
                                                        error);

    if (instance == nullptr)
    {
        std::cout<<"The plugin could not be instantiated."<< std::endl;
        return nullptr;
    }
    instance->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
    return instance;
}
