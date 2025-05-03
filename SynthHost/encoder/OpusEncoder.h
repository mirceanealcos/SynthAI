//
// Created by Mircea Nealcos on 4/24/2025.
//

#ifndef OPUSENCODER_H
#define OPUSENCODER_H
#include <stdexcept>
#include <opus/opus.h>
#include <vector>


class OpusEncoder {
public:
    OpusEncoder(int sampleRate, int channels, int application = OPUS_APPLICATION_AUDIO) {
        int err;
        _encoder = opus_encoder_create(sampleRate, channels, application, &err);
        if (err != OPUS_OK) {
            throw std::runtime_error("OpusEncoder create failed!");
        }
        _maxPacketBytes = 4000;
        _tmp.resize(_maxPacketBytes);
    }
    ~OpusEncoder() {
        if (_encoder != nullptr) {
            opus_encoder_destroy(_encoder);
        }
    }

    std::vector<unsigned char> encode(const float* pcm, int frameSize) {
        int n = opus_encode_float(_encoder, pcm, frameSize, _tmp.data(), _maxPacketBytes);
        if (n < 0) throw std::runtime_error("OpusEncoder encode failed!");
        return std::vector<unsigned char>(_tmp.begin(), _tmp.begin() + n);
    }

    OpusEncoder* get_encoder() {
        return _encoder;
    }

private:
    OpusEncoder* _encoder = nullptr;
    int _maxPacketBytes;
    std::vector<unsigned char> _tmp;
};



#endif //OPUSENCODER_H
