#include "PluginManager.h"

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
        throw new std::runtime_error("No plugin found at " + plugin.path);
    }

    auto instance = formatManager.createPluginInstance(*descriptions[0],
                                                        sampleRate,
                                                        blockSize,
                                                        error);

    if (instance == nullptr)
    {
        throw new std::runtime_error("The plugin could not be instantiated.");
    }
    return instance;
}
