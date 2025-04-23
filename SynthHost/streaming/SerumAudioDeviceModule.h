#pragma once

#include "modules/audio_device/include/audio_device.h"
#include "../audio_engine/utils/AudioRingBuffer.h"
#include "api/scoped_refptr.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>


/**
 * @class SerumAudioDeviceModule
 * @brief A custom AudioDeviceModule that feeds audio from a ring buffer
 *        (provided by your headless VST engine) into WebRTC as if it were a microphone.
 *
 * This implementation does not handle speaker playout. All playout methods are stubs.
 */
class SerumAudioDeviceModule : public webrtc::AudioDeviceModule
{
public:
    /**
     * @brief Factory method to create a new SerumAudioDeviceModule.
     * @param ring A shared pointer to the ring buffer containing final VST audio.
     */
    static rtc::scoped_refptr<SerumAudioDeviceModule> Create(std::shared_ptr<AudioRingBuffer> ring);

    //--------------------------------------------------------------------------
    // RefCountedObject inherits from rtc::RefCountInterface internally,
    // so we don't need to override AddRef / Release methods explicitly.
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    // Inherited from webrtc::AudioDeviceModule (partial interface).
    //--------------------------------------------------------------------------

    //--- Initialization and Termination ---//
    int32_t ActiveAudioLayer(AudioLayer* audioLayer) const override;
    int32_t Init() override;
    int32_t Terminate() override;
    bool Initialized() const override;

    //--- Device Enumeration ---//
    int16_t PlayoutDevices() override;
    int16_t RecordingDevices() override;
    int32_t PlayoutDeviceName(uint16_t index,
                              char name[128],
                              char guid[128]) override;
    int32_t RecordingDeviceName(uint16_t index,
                                char name[128],
                                char guid[128]) override;

    //--- Device Selection ---//
    int32_t SetPlayoutDevice(uint16_t index) override;
    int32_t SetRecordingDevice(uint16_t index) override;


    //--- Audio Initialization ---//
    int32_t InitPlayout() override;
    bool PlayoutIsInitialized() const override;
    int32_t InitRecording() override;
    bool RecordingIsInitialized() const override;

    //--- Start/Stop Audio ---//
    int32_t StartPlayout() override;
    int32_t StopPlayout() override;
    bool Playing() const override;
    int32_t StartRecording() override;
    int32_t StopRecording() override;
    bool Recording() const override;


    //--- Audio Transport Callback Registration ---//
    /**
     * @brief Registers the AudioTransport, which WebRTC uses to receive captured data.
     */
    int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;

    //--- Advanced/Optional Methods (stubs here) ---//

    // Output volume
    int32_t InitSpeaker() override;
    bool SpeakerIsInitialized() const override;
    int32_t InitMicrophone() override;
    bool MicrophoneIsInitialized() const override;
    int32_t SpeakerVolumeIsAvailable(bool* available) override;
    int32_t SetSpeakerVolume(uint32_t volume) override;
    int32_t SpeakerVolume(uint32_t* volume) const override;
    int32_t MaxSpeakerVolume(uint32_t* maxVolume) const override;
    int32_t MinSpeakerVolume(uint32_t* minVolume) const override;
    int32_t MicrophoneVolumeIsAvailable(bool* available) override;
    int32_t SetMicrophoneVolume(uint32_t volume) override;
    int32_t MicrophoneVolume(uint32_t* volume) const override;
    int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const override;
    int32_t MinMicrophoneVolume(uint32_t* minVolume) const override;
    int32_t SpeakerMuteIsAvailable(bool* available) override;
    int32_t SetSpeakerMute(bool enable) override;
    int32_t SpeakerMute(bool* enabled) const override;
    int32_t MicrophoneMuteIsAvailable(bool* available) override;
    int32_t SetMicrophoneMute(bool enable) override;
    int32_t MicrophoneMute(bool* enabled) const override;
    int32_t StereoPlayoutIsAvailable(bool* available) const override;
    int32_t SetStereoPlayout(bool enable) override;
    int32_t StereoRecordingIsAvailable(bool* available) const override;
    int32_t SetStereoRecording(bool enable) override;
    int32_t PlayoutDelay(uint16_t* delayMS) const override;

    bool BuiltInAECIsAvailable() const override;
    bool BuiltInAGCIsAvailable() const override;
    bool BuiltInNSIsAvailable() const override;
    int32_t EnableBuiltInAEC(bool enable) override;
    int32_t EnableBuiltInAGC(bool enable) override;
    int32_t EnableBuiltInNS(bool enable) override;

    int32_t SetPlayoutDevice(WindowsDeviceType device) override;
    int32_t SetRecordingDevice(WindowsDeviceType device) override;

    // 2) Checking device availability
    int32_t PlayoutIsAvailable(bool* available) override;
    int32_t RecordingIsAvailable(bool* available) override;

    // 3) Stereo queries
    int32_t StereoPlayout(bool* enabled) const override;
    int32_t StereoRecording(bool* enabled) const override;

protected:
    /**
     * @brief Protected constructor for creation via static factory method.
     */
    SerumAudioDeviceModule(std::shared_ptr<AudioRingBuffer> ring);

    /**
     * @brief Destructor.
     */
    ~SerumAudioDeviceModule() override;

private:
    /**
     * @brief The capture thread function that periodically reads from the ring buffer
     *        and pushes audio to WebRTC via RecordedDataIsAvailable().
     */
    void captureThreadFunc();

    //--------------------------------------------------------------------------
    // Internal State
    //--------------------------------------------------------------------------
    std::shared_ptr<AudioRingBuffer>   ringBuffer_;      ///< The ring buffer with final VST audio.
    webrtc::AudioTransport*            audioTransport_;  ///< WebRTC transport callback (set by RegisterAudioCallback).
    std::thread                        captureThread_;
    std::mutex                         mutex_;
    std::atomic<bool>                  initialized_{false};
    std::atomic<bool>                  recording_{false}; ///< "Recording" from the ring buffer
    std::atomic<bool>                  playing_{false};   ///< Stub: we are not playing audio out

    // Configure the sample rate, channels, etc. You can make these configurable.
    const int sampleRate_ = 48000;
    const int channels_   = 2;
};

