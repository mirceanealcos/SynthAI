#include <boost/asio/io_context.hpp>

#include "audio_engine/HeadlessAudioEngine.h"
#include "vst_hosting/PluginManager.h"
#include "utils/PluginEnum.h"
#include "audio_engine/SpeakerAudioEngine.h"
#include "audio_engine/utils/AudioRingBuffer.h"
#include "encoder/OpusEncoder.h"
#include "midi/MidiDeviceManager.h"
#include "websocket/WebSocketClient.h"
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512

using namespace std::chrono_literals;

int main() {
    // Setting up the audio engine and loading Serum user instance
    HeadlessAudioEngine audioEngine(48000, 512);
    PluginManager manager;
    juce::String errorMsg;
    auto userSerumInstance = manager.loadPlugin(PluginEnum::SERUM, 48000, 512, errorMsg);
    audioEngine.setPlugin(std::move(userSerumInstance));
    audioEngine.start();

    // Set up the connection to the websocket
    auto userSerumRingBuffer = audioEngine.getRingBuffer();
    boost::asio::io_context ioc{1};
    auto socket = std::make_shared<WebSocketClient>(
        ioc, "localhost", "8080", "/user/audio");

    socket->onMessage = [&](std::vector<uint8_t> message) {
        std::cout << "Received message of " << message.size() << " bytes" << std::endl;
    };

    socket->run();

    std::thread runner([&] { ioc.run(); });

    // Opus Encoder setup
    int opusErr;
    auto *encoder = new OpusEncoder(48000, 2, OPUS_APPLICATION_AUDIO);
    std::atomic<bool> running{true};

    // Audio encoding + sending over websocket
    std::thread sendThread([&]() {
        std::vector<float> pcmBuf(480);
        std::vector<unsigned char> outBuf(1500);

        while (running.load()) {
            size_t got = userSerumRingBuffer->read(pcmBuf.data(), pcmBuf.size());
            if (got < pcmBuf.size()) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int) std::round((512.0 / 48000.0) * 1000.0)));
                continue;
            }
            // encode to Opus
            int bytes = opus_encode_float(
                encoder->get_encoder(),
                pcmBuf.data(),
                480,
                outBuf.data(),
                1500
            );
            if (bytes < 0) {
                continue; // encoding error
            }

            socket->sendBinary({outBuf.data(), outBuf.data() + bytes});
        }
    });

    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);) {
        if (line == "quit") break;
    }

    running.store(false);
    sendThread.join();

    audioEngine.stop();
    opus_encoder_destroy(encoder);

    ioc.stop();
    runner.join();

    return 0;
}
