#include "vst_hosting/PluginManager.h"
#include "utils/PluginEnum.h"


int main() {

    PluginManager manager;
    juce::String errorMsg;
    auto instance = manager.loadPlugin(PluginEnum::SERUM, 48000, 1024, errorMsg);

}
