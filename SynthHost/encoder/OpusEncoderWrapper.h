#ifndef OPUS_ENCODER_WRAPPER_H
#define OPUS_ENCODER_WRAPPER_H

#include <opus/opus.h>
#include <vector>
#include <stdexcept>

class OpusEncoderWrapper {
public:
    OpusEncoderWrapper(int sampleRate, int channels, int application = OPUS_APPLICATION_AUDIO)
        : sampleRate(sampleRate), channels(channels), maxPacketBytes(1500)
    {
        int err = 0;
        encoder = opus_encoder_create(sampleRate, channels, application, &err);
        opus_encoder_ctl(encoder, OPUS_BITRATE_MAX);
        if (err != OPUS_OK || encoder == nullptr) {
            throw std::runtime_error("Failed to create Opus encoder: " + std::string(opus_strerror(err)));
        }
    }

    ~OpusEncoderWrapper()
    {
        if (encoder != nullptr) {
            opus_encoder_destroy(encoder);
            encoder = nullptr;
        }
    }

    std::vector<uint8_t> encodeFrame(const float* pcm, int frameSize)
    {
        std::vector<uint8_t> encoded(maxPacketBytes);
        int bytesEncoded = opus_encode_float(encoder, pcm, frameSize, encoded.data(), maxPacketBytes);

        if (bytesEncoded < 0) {
            throw std::runtime_error("Opus encoding failed: " + std::string(opus_strerror(bytesEncoded)));
        }
        std::cout << bytesEncoded << std::endl;

        encoded.resize(bytesEncoded); // shrink to actual size
        return encoded;
    }

private:
    OpusEncoder* encoder = nullptr;
    int sampleRate;
    int channels;
    int maxPacketBytes;
};

#endif // OPUS_ENCODER_WRAPPER_H
