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
#include "streaming/UDPAudioSender.h"

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 1024

using namespace std::chrono_literals;

int main() {
    // Set up audio engine and Serum instance
    HeadlessAudioEngine audioEngine(SAMPLE_RATE, BLOCK_SIZE);
    PluginManager manager;
    juce::String errorMsg;
    auto userSerumInstance = manager.loadPlugin(PluginEnum::SERUM_PC, SAMPLE_RATE, BLOCK_SIZE, errorMsg);
    audioEngine.setPlugin(std::move(userSerumInstance));
    audioEngine.setPreset(Presets::ENGINE_HASH);
    audioEngine.start();

    // WebSocket setup
    auto userSerumRingBuffer = audioEngine.getRingBuffer();
    // boost::asio::io_context ioc{1};
    // auto socket = std::make_shared<WebSocketClient>(
    //     ioc, "localhost", "8080", "/user/audio");
    //
    // socket->onMessage = [&](std::vector<uint8_t> message) {
    //     std::cout << "Received message of " << message.size() << " bytes" << std::endl;
    // };
    //
    // socket->run();
    // std::thread runner([&] { ioc.run(); });

    UDPAudioSender udpSender("127.0.0.1", 9000);

    std::atomic<bool> running{true};

    std::thread sendThread([&]() {
        using clock = std::chrono::high_resolution_clock;
        auto nextTick = clock::now();
        const auto interval = std::chrono::microseconds(int64_t(1'000'000.0 * 1024 / 48000.0));

        std::vector<float> pcmBuf(1024);

        while (running.load()) {
            size_t got = userSerumRingBuffer->read(pcmBuf.data(), 1024);
            if (got < pcmBuf.size()) {
                // Not enough samples? Fill rest with zeros
                std::fill(pcmBuf.begin() + got, pcmBuf.end(), 0.0f);
            }

            udpSender.send(pcmBuf.data(), pcmBuf.size());

            nextTick += interval;
            std::this_thread::sleep_until(nextTick);
        }
    });

    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);) {
        if (line == "quit") break;
    }

    running.store(false);
    sendThread.join();
    audioEngine.stop();
    // ioc.stop();
    // runner.join();

    return 0;
}
