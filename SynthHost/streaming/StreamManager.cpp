//
// Created by Mircea Nealcos on 5/12/2025.
//

#include "StreamManager.h"

StreamManager::StreamManager(int blockSize, int sampleRate, int port, StreamID id) {
    this->blockSize = blockSize;
    this->sampleRate = sampleRate;
    this->port = port;
    this->running.store(false);
    this->id = id;
    this->init();
}

StreamManager::~StreamManager() {
    streamingThread.join();
    audioEngine->stop();
}


void StreamManager::init() {
    this->audioEngine = std::make_unique<HeadlessAudioEngine>(sampleRate, blockSize);
    juce::String error;
    std::unique_ptr<juce::AudioPluginInstance> serumInstance;
    try {
        serumInstance = pluginManager.loadPlugin(PluginEnum::SERUM_PC, sampleRate, blockSize, error);
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        throw;
    }
    audioEngine->setPlugin(std::move(serumInstance));
    audioEngine->start();
    udpAudioSender = std::make_unique<UDPAudioSender>("127.0.0.1", port);
}

void StreamManager::startStreaming() {
    running.store(true);
    streamingThread = std::thread([&]() {
        const int FRAMES_PER_PACKET = 512;
        const int FLOATS_PER_PACKET = FRAMES_PER_PACKET * 2;
        using clock = std::chrono::high_resolution_clock;
        using us = std::chrono::microseconds;
        auto interval = us(int64_t(1'000'000.0 * FRAMES_PER_PACKET / sampleRate));
        auto nextTick = clock::now();
        auto ringBuffer = audioEngine->getRingBuffer();
        std::vector<float> pcmBuffer(FLOATS_PER_PACKET);
        while (running.load()) {
            size_t got = ringBuffer->read(pcmBuffer.data(), pcmBuffer.size());
            if (got < pcmBuffer.size()) {
                std::fill(pcmBuffer.begin() + got, pcmBuffer.end(), 0.0f);
            }
            udpAudioSender->send(pcmBuffer.data(), pcmBuffer.size());
            nextTick += interval;
            std::this_thread::sleep_until(nextTick);
        }
    });
}

void StreamManager::stopStreaming() {
    running.store(false);
}

void StreamManager::setPreset(Preset preset) {
    audioEngine->setPreset(preset);
}

StreamID StreamManager::getStreamID() {
    return id;
}
