#include "SerumAudioDeviceModule.h"
#include "api/audio/audio_frame.h" // for webrtc::AudioFrame
#include <chrono>
#include <thread>

// Factory method
rtc::scoped_refptr<SerumAudioDeviceModule>
SerumAudioDeviceModule::Create(std::shared_ptr<AudioRingBuffer> ring)
{
    return rtc::scoped_refptr<SerumAudioDeviceModule>(
        new rtc::RefCountedObject<SerumAudioDeviceModule>(ring));
}

SerumAudioDeviceModule::SerumAudioDeviceModule(std::shared_ptr<AudioRingBuffer> ring)
    : ringBuffer_(ring),
      audioTransport_(nullptr)
{
}

// Example stubs just to fulfill the interface
int32_t SerumAudioDeviceModule::SetPlayoutDevice(WindowsDeviceType device)
{
    // If you don’t care about actual device selection, return success or -1
    return 0;
}

int32_t SerumAudioDeviceModule::SetRecordingDevice(WindowsDeviceType device)
{
    return 0;
}

int32_t SerumAudioDeviceModule::PlayoutIsAvailable(bool* available)
{
    if (available) *available = false; // not supported
    return 0;
}

int32_t SerumAudioDeviceModule::RecordingIsAvailable(bool* available)
{
    if (available) *available = true; // or false if not supported
    return 0;
}

int32_t SerumAudioDeviceModule::StereoPlayout(bool* enabled) const
{
    if (enabled) *enabled = false; // or true if you do stereo
    return 0;
}

int32_t SerumAudioDeviceModule::StereoRecording(bool* enabled) const
{
    if (enabled) *enabled = true; // you said you do stereo capturing
    return 0;
}

SerumAudioDeviceModule::~SerumAudioDeviceModule()
{
    StopRecording();
    Terminate();
}

//===============================================================
// AudioDeviceModule overrides
//===============================================================

int32_t SerumAudioDeviceModule::ActiveAudioLayer(AudioLayer* audioLayer) const
{
    if (audioLayer)
        *audioLayer = kDummyAudio;
    return 0;
}

int32_t SerumAudioDeviceModule::Init()
{
    initialized_ = true;
    return 0;
}

int32_t SerumAudioDeviceModule::Terminate()
{
    initialized_ = false;
    return 0;
}

bool SerumAudioDeviceModule::Initialized() const
{
    return initialized_;
}

// --- Device enumeration stubs ---
int16_t SerumAudioDeviceModule::PlayoutDevices() { return 0; }
int16_t SerumAudioDeviceModule::RecordingDevices() { return 1; }
int32_t SerumAudioDeviceModule::PlayoutDeviceName(uint16_t, char name[128], char guid[128])
{
    return -1;
}
int32_t SerumAudioDeviceModule::RecordingDeviceName(uint16_t, char name[128], char guid[128])
{
    if (name) strcpy(name, "SerumAudioDev");
    if (guid) strcpy(guid, "SerumAudioDevGUID");
    return 0;
}

// --- Device selection stubs ---
int32_t SerumAudioDeviceModule::SetPlayoutDevice(uint16_t) { return -1; }
int32_t SerumAudioDeviceModule::SetRecordingDevice(uint16_t) { return 0; }


// --- Audio Initialization stubs ---
int32_t SerumAudioDeviceModule::InitPlayout() { return 0; }
bool SerumAudioDeviceModule::PlayoutIsInitialized() const { return false; }
int32_t SerumAudioDeviceModule::InitRecording() { return 0; }
bool SerumAudioDeviceModule::RecordingIsInitialized() const { return initialized_; }

// --- Start/Stop audio ---
int32_t SerumAudioDeviceModule::StartPlayout()
{
    playing_ = true;
    return 0;
}
int32_t SerumAudioDeviceModule::StopPlayout()
{
    playing_ = false;
    return 0;
}
bool SerumAudioDeviceModule::Playing() const
{
    return playing_;
}

int32_t SerumAudioDeviceModule::StartRecording()
{
    if (!initialized_)
        return -1;

    recording_ = true;
    captureThread_ = std::thread(&SerumAudioDeviceModule::captureThreadFunc, this);
    return 0;
}
int32_t SerumAudioDeviceModule::StopRecording()
{
    recording_ = false;
    if (captureThread_.joinable())
        captureThread_.join();
    return 0;
}
bool SerumAudioDeviceModule::Recording() const
{
    return recording_;
}



// --- RegisterAudioCallback ---
int32_t SerumAudioDeviceModule::RegisterAudioCallback(webrtc::AudioTransport* audioCallback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioTransport_ = audioCallback;
    return 0;
}

//===============================================================
// captureThreadFunc - The core capturing logic
//===============================================================
void SerumAudioDeviceModule::captureThreadFunc()
{
    const int framesPer10ms = sampleRate_ / 100; // e.g. 480
    const int totalSamples  = framesPer10ms * channels_;
    std::vector<float> floatBuffer(totalSamples, 0.0f);
    std::vector<int16_t> int16Buffer(totalSamples, 0);

    while (recording_)
    {
        // Read from the ring buffer
        // ringBuffer_->read() is a custom function you define.
        ringBuffer_->read(floatBuffer.data(), totalSamples);

        // Convert float -> int16
        for (int i = 0; i < totalSamples; i++)
        {
            float samp = floatBuffer[i] * 32767.0f;
            if (samp > 32767.0f) samp = 32767.0f;
            if (samp < -32768.0f) samp = -32768.0f;
            int16Buffer[i] = static_cast<int16_t>(samp);
        }

        // Deliver to WebRTC
        {

            std::lock_guard<std::mutex> lock(mutex_);
            if (audioTransport_)
            {
                uint32_t newMicLevel = 0;
                // recordedDataIsAvailable parameters:
                // (data, frames, sampleRate, channels, totalSamples,
                //  totalDelayMS, clockDrift, currentMicVol, keyPressed, vadProbability)
                audioTransport_->RecordedDataIsAvailable(
                    int16Buffer.data(),
                    framesPer10ms,
                    sampleRate_,
                    channels_,
                    totalSamples,
                    0,    // totalDelayMS
                    0,    // clockDrift
                    1.0f, // currentMicVol
                    false,// keyPressed
                    newMicLevel // vadProbability
                );
            }
        }

        // Sleep for 10ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//===============================================================
// Stubs for advanced features
//===============================================================
int32_t SerumAudioDeviceModule::InitSpeaker() { return 0; }
bool SerumAudioDeviceModule::SpeakerIsInitialized() const { return false; }
int32_t SerumAudioDeviceModule::InitMicrophone() { return 0; }
bool SerumAudioDeviceModule::MicrophoneIsInitialized() const { return initialized_; }
int32_t SerumAudioDeviceModule::SpeakerVolumeIsAvailable(bool* available)
{
    if (available) *available = false;
    return 0;
}
int32_t SerumAudioDeviceModule::SetSpeakerVolume(uint32_t) { return -1; }
int32_t SerumAudioDeviceModule::SpeakerVolume(uint32_t* volume) const
{
    if (volume) *volume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::MaxSpeakerVolume(uint32_t* maxVolume) const
{
    if (maxVolume) *maxVolume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::MinSpeakerVolume(uint32_t* minVolume) const
{
    if (minVolume) *minVolume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::MicrophoneVolumeIsAvailable(bool* available)
{
    if (available) *available = false;
    return 0;
}
int32_t SerumAudioDeviceModule::SetMicrophoneVolume(uint32_t) { return -1; }
int32_t SerumAudioDeviceModule::MicrophoneVolume(uint32_t* volume) const
{
    if (volume) *volume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::MaxMicrophoneVolume(uint32_t* maxVolume) const
{
    if (maxVolume) *maxVolume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::MinMicrophoneVolume(uint32_t* minVolume) const
{
    if (minVolume) *minVolume = 0;
    return -1;
}
int32_t SerumAudioDeviceModule::SpeakerMuteIsAvailable(bool* available)
{
    if (available) *available = false;
    return 0;
}
int32_t SerumAudioDeviceModule::SetSpeakerMute(bool) { return -1; }
int32_t SerumAudioDeviceModule::SpeakerMute(bool* enabled) const
{
    if (enabled) *enabled = false;
    return -1;
}
int32_t SerumAudioDeviceModule::MicrophoneMuteIsAvailable(bool* available)
{
    if (available) *available = false;
    return 0;
}
int32_t SerumAudioDeviceModule::SetMicrophoneMute(bool) { return -1; }
int32_t SerumAudioDeviceModule::MicrophoneMute(bool* enabled) const
{
    if (enabled) *enabled = false;
    return -1;
}
int32_t SerumAudioDeviceModule::StereoPlayoutIsAvailable(bool* available) const
{
    if (available) *available = true;
    return 0;
}
int32_t SerumAudioDeviceModule::SetStereoPlayout(bool enable)
{
    // ignoring for now
    return 0;
}
int32_t SerumAudioDeviceModule::StereoRecordingIsAvailable(bool* available) const
{
    if (available) *available = true;
    return 0;
}
int32_t SerumAudioDeviceModule::SetStereoRecording(bool enable)
{
    return 0;
}
int32_t SerumAudioDeviceModule::PlayoutDelay(uint16_t* delayMS) const
{
    if (delayMS) *delayMS = 0;
    return 0;
}

bool SerumAudioDeviceModule::BuiltInAECIsAvailable() const { return false; }
bool SerumAudioDeviceModule::BuiltInAGCIsAvailable() const { return false; }
bool SerumAudioDeviceModule::BuiltInNSIsAvailable() const { return false; }
int32_t SerumAudioDeviceModule::EnableBuiltInAEC(bool) { return -1; }
int32_t SerumAudioDeviceModule::EnableBuiltInAGC(bool) { return -1; }
int32_t SerumAudioDeviceModule::EnableBuiltInNS(bool) { return -1; }
