//
// Created by Mircea Nealcos on 5/13/2025.
//

#include "StreamController.h"

StreamController::StreamController(boost::asio::io_context &ioContext) : ioContext(ioContext) {
}

void StreamController::addStreamManager(int blockSize, int sampleRate, int port, StreamID id, bool isAIEngine) {
    auto streamManager = std::make_shared<StreamManager>(blockSize, sampleRate, port, id, isAIEngine);
    streamManager->startStreaming();

    streams.push_back(streamManager);
}


void StreamController::addWebSocketClient(string host, string port, string url, WebSocketClientID id,
                                          JsonMethod onJsonMethod) {
    auto wsClient = std::make_shared<WebSocketClient>(ioContext, host, port, url, id);
    wsClient->onJson([this, onJsonMethod](const json &j) {
        (this->*onJsonMethod)(j);
    });
    wsClient->run();
    wsClients.push_back(wsClient);
}


void StreamController::shutdown() {
    for (auto wsClient: wsClients) {
        wsClient->close();
    }
    for (auto stream: streams) {
        stream->stopStreaming();
    }
    ioContext.stop();
}

std::shared_ptr<StreamManager> StreamController::getStreamManager(StreamID id) {
    for (auto stream: streams) {
        if (stream->getStreamID() == id) {
            return stream;
        }
    }
    throw std::runtime_error("Stream does not exist");
}

std::shared_ptr<WebSocketClient> StreamController::getWebSocketClient(WebSocketClientID id) {
    for (auto client: wsClients) {
        if (client->getID() == id) {
            return client;
        }
    }
    return nullptr;
}

std::shared_ptr<StreamManager> StreamController::getStream(StreamID id) {
    for (auto stream: streams) {
        if (stream->getStreamID() == id) {
            return stream;
        }
    }
    return nullptr;
}

void StreamController::setMidiSenderClient(WebSocketClientID sender, StreamID streamer) {
    auto stream = getStream(streamer);
    auto client = getWebSocketClient(sender);
    if (client != nullptr && stream != nullptr)
        stream->setMidiSenderClient(client);
}

StreamID StreamController::getStreamIDForRole(const std::string &role) {
    if (role == "bass") return AI_BASS;
    if (role == "pad") return AI_PAD;
    if (role == "pluck")return AI_PLUCK;
    if (role == "lead") return AI_LEAD;
    return USER;
}


// handler methods
void StreamController::changePreset(const json &j) {
    string preset = j.at("preset").get<string>();
    try {
        Preset foundPreset = Presets::getFromString(preset);
        std::shared_ptr<StreamManager> stream = getStreamManager(USER);
        stream->setPreset(foundPreset);
        std::shared_ptr<StreamManager> ai_bass_stream = getStreamManager(AI_BASS);
        std::shared_ptr<StreamManager> ai_lead_stream = getStreamManager(AI_LEAD);
        std::shared_ptr<StreamManager> ai_pad_stream = getStreamManager(AI_PAD);
        std::shared_ptr<StreamManager> ai_pluck_stream = getStreamManager(AI_PLUCK);
        if (foundPreset.type == "bass") {
            stream->getAudioEngine()->setMidiRole("bass");
            ai_lead_stream->setPreset(Presets::getRandomLead());
            ai_pad_stream->setPreset(Presets::getRandomPad());
            ai_pluck_stream->setPreset(Presets::getRandomPluck());
        }
        if (foundPreset.type == "lead") {
            stream->getAudioEngine()->setMidiRole("lead");
            ai_bass_stream->setPreset(Presets::getRandomBass());
            ai_pad_stream->setPreset(Presets::getRandomPad());
            ai_pluck_stream->setPreset(Presets::getRandomPluck());
        }
        if (foundPreset.type == "pad") {
            stream->getAudioEngine()->setMidiRole("pad");
            ai_bass_stream->setPreset(Presets::getRandomBass());
            ai_lead_stream->setPreset(Presets::getRandomLead());
            ai_pluck_stream->setPreset(Presets::getRandomPluck());
        }
        if (foundPreset.type == "pluck") {
            stream->getAudioEngine()->setMidiRole("pluck");
            ai_bass_stream->setPreset(Presets::getRandomBass());
            ai_lead_stream->setPreset(Presets::getRandomLead());
            ai_pad_stream->setPreset(Presets::getRandomPad());
        }
    } catch (const std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
    }
}

void StreamController::handleComposeOutput(const json &j) {
    auto role = j.at("role").get<string>();
    auto manager = getStreamManager(getStreamIDForRole(role));
    if (!manager) return;

    bool isOn = (j.at("type").get<string>() == "note_on");
    int note = j.at("note").get<int>();
    int vel = j.at("velocity").get<int>();
    juce::MidiMessage m = isOn
                              ? juce::MidiMessage::noteOn(1, note, (uint8_t) vel)
                              : juce::MidiMessage::noteOff(1, note);
    int64_t eventMs = j.at("timestamp").get<int64_t>();
    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    double deltaMs = double(eventMs - nowMs);
    if (deltaMs < 0) return;

    double sr = manager->getSampleRate();
    int delayS = int(deltaMs * sr / 1000.0 + 0.5);

    manager->getAudioEngine()->enqueueMidi(m, delayS);
}
