//
// Created by Mircea Nealcos on 5/12/2025.
//

#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H
#include <thread>

#include "../audio_engine/HeadlessAudioEngine.h"
#include "UDPAudioSender.h"
#include "../utils/StreamID.h"
#include "../vst_hosting/PluginManager.h"

class StreamManager {
public:
    explicit StreamManager(int blockSize = 512, int sampleRate = 48000, int port = 9000, StreamID id = USER, bool isAIEngine = false);

    ~StreamManager();

    void startStreaming();

    void stopStreaming();

    void setPreset(Preset preset);

    StreamID getStreamID();

    void setMidiSenderClient(std::shared_ptr<WebSocketClient> sender);

    double getSampleRate() { return sampleRate; }

    HeadlessAudioEngine* getAudioEngine() { return audioEngine.get(); }

private:
    void init(bool isAIEngine);

    StreamID id;
    std::unique_ptr<HeadlessAudioEngine> audioEngine;
    std::unique_ptr<UDPAudioSender> udpAudioSender;
    PluginManager pluginManager;
    std::thread streamingThread;
    std::atomic<bool> running;
    int blockSize;
    int sampleRate;
    int port;
};

#endif //STREAMMANAGER_H
