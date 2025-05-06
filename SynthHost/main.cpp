#include <corecrt_math_defines.h>
#include <boost/asio/io_context.hpp>

#include "audio_engine/HeadlessAudioEngine.h"
#include "vst_hosting/PluginManager.h"
#include "utils/PluginEnum.h"
#include "audio_engine/SpeakerAudioEngine.h"
#include "audio_engine/utils/AudioRingBuffer.h"
#include "encoder/OpusEncoderWrapper.h"
#include "midi/MidiDeviceManager.h"
#include "websocket/WebSocketClient.h"

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512

using namespace std::chrono_literals;

int main() {
    // Set up audio engine and Serum instance
    HeadlessAudioEngine audioEngine(SAMPLE_RATE, BLOCK_SIZE);
    PluginManager manager;
    juce::String errorMsg;
    auto userSerumInstance = manager.loadPlugin(PluginEnum::SERUM_PC, SAMPLE_RATE, BLOCK_SIZE, errorMsg);
    audioEngine.setPlugin(std::move(userSerumInstance));
    audioEngine.setPreset(Presets::SUBNET);
    audioEngine.start();

    // WebSocket setup
    auto userSerumRingBuffer = audioEngine.getRingBuffer();
    boost::asio::io_context ioc{1};
    auto socket = std::make_shared<WebSocketClient>(
        ioc, "localhost", "8080", "/user/audio");

    socket->onMessage = [&](std::vector<uint8_t> message) {
        std::cout << "Received message of " << message.size() << " bytes" << std::endl;
    };

    socket->run();
    std::thread runner([&] { ioc.run(); });

    std::atomic<bool> running{true};

    std::thread sendThread([&]() {
        std::vector<float> buffer(1024 + 128); // overlap buffer
        std::vector<float> overlap(128); // carry-over
        std::vector<float> pluginBlock(1024); // plugin audio
        OpusEncoderWrapper opus(48000, 1); // mono encoder

        while (running.load()) {
            size_t got = userSerumRingBuffer->read(pluginBlock.data(), 1024);
            if (got < 1024)
                std::fill(pluginBlock.begin() + got, pluginBlock.end(), 0.0f);

            // Copy last overlap
            for (int i = 0; i < 128; i++) buffer[i] = overlap[i];

            // Copy 896 new plugin samples
            for (int i = 0; i < 896; i++) buffer[128 + i] = pluginBlock[i];

            // Save last 128 for next overlap
            for (int i = 0; i < 128; i++) overlap[i] = buffer[960 + i];

            try {
                auto encoded = opus.encodeFrame(buffer.data(), 960);
                socket->sendBinary({encoded.begin(), encoded.end()});
            } catch (const std::exception &e) {
                std::cerr << "❌ Encoding failed: " << e.what() << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(18));
        }
    });

    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);) {
        if (line == "quit") break;
    }

    running.store(false);
    sendThread.join();
    audioEngine.stop();
    ioc.stop();
    runner.join();

    return 0;
}
