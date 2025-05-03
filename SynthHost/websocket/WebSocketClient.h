//
// Created by Mircea Nealcos on 4/24/2025.
//

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <atomic>
#include <deque>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <vector>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace ws = beast::websocket;
using tcp = net::ip::tcp;

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:

    using OnBinaryMessage = std::function<void(std::vector<uint8_t> const&)>;

    WebSocketClient(net::io_context& ioc,
                         std::string const& host,
                         std::string const& port,
                         std::string const& target);

    /// kick off the async connect/handshake chain
    void run();
    void close();

    /// queue up a binary frame to send
    void sendBinary(std::vector<uint8_t> data);

    OnBinaryMessage onMessage;

private:
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void onHandshake(beast::error_code ec);
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    void doWrite();
    void onWrite(beast::error_code ec, std::size_t bytes_transferred);
    bool isConnected() const { return _connected && _socket.is_open(); }
    net::io_context& _ioc;
    tcp::resolver _resolver;
    ws::stream<tcp::socket> _socket;
    beast::flat_buffer _buffer;
    std::deque<std::vector<uint8_t>> _writeQueue;
    std::string _host;
    std::string _port;
    std::string _target;
    std::atomic<bool> _connected { false };
};

#endif //WEBSOCKETCLIENT_H
