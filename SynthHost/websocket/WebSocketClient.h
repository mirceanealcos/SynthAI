//
// Created by Mircea Nealcos on 4/24/2025.
//

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "../utils/WebSocketClientID.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    using JsonHandler = std::function<void(const json&)>;
    WebSocketClient(net::io_context& ioContext, std::string host, std::string port, std::string url, WebSocketClientID id);
    void run();
    void onJson(JsonHandler jsonHandler);
    void sendJson(const json& json);
    void close();
    WebSocketClientID getID();
private:
    WebSocketClientID id;
    tcp::resolver resolver;
    websocket::stream<tcp::socket> socket;
    beast::flat_buffer buffer;
    std::string host;
    std::string port;
    std::string url;
    JsonHandler jsonHandler;
    bool closing = false;
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec, tcp::endpoint);
    void onHandshake(beast::error_code ec);
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytes);
    void onWrite(beast::error_code ec, std::size_t bytes);
    void onClose(beast::error_code ec);

    static void fail(beast::error_code ec, char const* what);
};

#endif //WEBSOCKETCLIENT_H
