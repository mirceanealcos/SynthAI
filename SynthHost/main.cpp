#include <corecrt_math_defines.h>
#include <boost/asio/io_context.hpp>

#include "audio_engine/HeadlessAudioEngine.h"
#include "vst_hosting/PluginManager.h"
#include "utils/PluginEnum.h"
#include "audio_engine/utils/AudioRingBuffer.h"
#include "utils/serum/Presets.h"
#include "encoder/OpusEncoderWrapper.h"
#include "midi/MidiDeviceManager.h"
#include "streaming/UDPAudioSender.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#define SAMPLE_RATE 48000
#define BLOCK_SIZE   1024  // keep your JUCE engine block size if you want

int main()
{
    // 1) Set up the Headless JUCE engine + Serum
    HeadlessAudioEngine audioEngine (SAMPLE_RATE, BLOCK_SIZE);
    PluginManager        manager;
    juce::String         errorMsg;

    auto serum = manager.loadPlugin (PluginEnum::SERUM_PC,
                                     SAMPLE_RATE,
                                     BLOCK_SIZE,
                                     errorMsg);
    if (! serum) {
        std::cerr << "Failed to load plugin: " << errorMsg << "\n";
        return 1;
    }

    audioEngine.setPlugin (std::move(serum));
    audioEngine.setPreset (Presets::LEAD_1984);
    audioEngine.start();

    // 2) Grab your ring-buffer
    auto ringBuffer = audioEngine.getRingBuffer();

    // 3) Set up your UDP sender
    UDPAudioSender udpSender ("127.0.0.1", 9000);

    // 4) Configure packet/frame sizes to match Unity’s DSP settings
    constexpr int FRAMES_PER_PACKET = 512;            // Unity DSP buffer size in frames
    constexpr int CHANNELS          = 2;              // stereo
    constexpr int FLOATS_PER_PACKET = FRAMES_PER_PACKET * CHANNELS; // = 1024 floats

    std::cout << "Sending " << FLOATS_PER_PACKET
              << " floats every " << (1000.0 * FRAMES_PER_PACKET / SAMPLE_RATE)
              << " ms\n";

    // Calculate a steady send interval
    using clock     = std::chrono::high_resolution_clock;
    using us        = std::chrono::microseconds;
    auto interval   = us(int64_t(1'000'000.0 * FRAMES_PER_PACKET / SAMPLE_RATE));
    auto nextTick   = clock::now();

    std::atomic<bool> running { true };

    // 5) Launch the send thread
    std::thread sendThread ([&]() {
        std::vector<float> pcmBuf (FLOATS_PER_PACKET);

        while (running.load())
        {
            // Read exactly FLOATS_PER_PACKET floats from your ring buffer
            size_t got = ringBuffer->read (pcmBuf.data(),
                                           pcmBuf.size());

            if (got < pcmBuf.size())
            {
                // Zero-pad the rest
                std::fill (pcmBuf.begin() + got,
                           pcmBuf.end(), 0.0f);
            }

            // Send the full packet
            udpSender.send (pcmBuf.data(), pcmBuf.size());

            // Sleep until exactly the next tick
            nextTick += interval;
            std::this_thread::sleep_until (nextTick);
        }
    });

    // 6) Wait for user to quit
    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);)
    {
        if (line == "quit") break;
    }

    // 7) Clean up
    running.store(false);
    sendThread.join();
    audioEngine.stop();

    return 0;
}
