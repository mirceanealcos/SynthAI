//
// Created by Mircea Nealcos on 5/13/2025.
//

#ifndef STREAMCONTROLLER_H
#define STREAMCONTROLLER_H
#include <boost/asio/io_context.hpp>
#include<nlohmann/json.hpp>

#include "../streaming/StreamManager.h"
#include "../websocket/WebSocketClient.h"
using json = nlohmann::json;
using string = std::string;

class StreamController {
public:
    using JsonMethod = void (StreamController::*)(const json&);

    explicit StreamController(boost::asio::io_context& ioContext);
    void addStreamManager(int blockSize, int sampleRate, int port, StreamID id);
    std::shared_ptr<StreamManager> getStreamManager(StreamID id);
    void addWebSocketClient(string host, string port, string url, WebSocketClientID id, JsonMethod onJsonMethod);
    void shutdown();

    // handler methods
    void changePreset(const json& j);

private:
    boost::asio::io_context& ioContext;
    std::vector<std::shared_ptr<StreamManager>> streams;
    std::vector<std::shared_ptr<WebSocketClient>> wsClients;
};



#endif //STREAMCONTROLLER_H
