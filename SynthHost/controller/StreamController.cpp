//
// Created by Mircea Nealcos on 5/13/2025.
//

#include "StreamController.h"

StreamController::StreamController(boost::asio::io_context &ioContext) : ioContext(ioContext) {}

void StreamController::addStreamManager(int blockSize, int sampleRate, int port, StreamID id) {
    auto streamManager = std::make_shared<StreamManager>(blockSize, sampleRate, port, id);
    streamManager->startStreaming();

    streams.push_back(streamManager);
}


void StreamController::addWebSocketClient(string host, string port, string url, WebSocketClientID id, JsonMethod onJsonMethod) {
    auto wsClient = std::make_shared<WebSocketClient>(ioContext, host, port, url, id);
    wsClient->onJson([this, onJsonMethod](const json& j) {
       (this->*onJsonMethod)(j);
    });
    wsClient->run();
    wsClients.push_back(wsClient);
}


void StreamController::shutdown() {
    for (auto wsClient : wsClients) {
        wsClient->close();
    }
    for (auto stream : streams) {
        stream->stopStreaming();
    }
    ioContext.stop();
}

std::shared_ptr<StreamManager> StreamController::getStreamManager(StreamID id) {
    for (auto stream : streams) {
        if (stream->getStreamID() == id) {
            return stream;
        }
    }
    throw std::runtime_error("Stream does not exist");
}

// handler methods
void StreamController::changePreset(const json &j) {
    string preset = j.at("preset").get<string>();
    try {
        Preset foundPreset = Presets::getFromString(preset);
        std::shared_ptr<StreamManager> stream = getStreamManager(USER);
        stream->setPreset(foundPreset);
    }
    catch (const std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
    }
}

std::shared_ptr<WebSocketClient> StreamController::getWebSocketClient(WebSocketClientID id)
{
    for (auto client : wsClients)
    {
        if (client->getID() == id)
        {
            return client;
        }
    }
    return nullptr;
}

std::shared_ptr<StreamManager> StreamController::getStream(StreamID id)
{
    for (auto stream : streams)
    {
        if (stream->getStreamID() == id)
        {
            return stream;
        }
    }
    return nullptr;
}

void StreamController::setMidiSenderClient(WebSocketClientID sender, StreamID streamer)
{
    auto stream = getStream(streamer);
    auto client = getWebSocketClient(sender);
    if (client != nullptr && stream != nullptr)
        stream->setMidiSenderClient(client);
}




