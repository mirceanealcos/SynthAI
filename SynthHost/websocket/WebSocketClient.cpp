#include "WebSocketClient.h"
#include <iostream>

WebSocketClient::WebSocketClient(net::io_context &ioc, std::string const &host,
                                 std::string const &port, std::string const &target)
    : _ioc(ioc), _resolver(ioc), _socket(net::make_strand(ioc)),
      _host(host), _port(port), _target(target) {}

void WebSocketClient::run() {
    _resolver.async_resolve(_host, _port,
        beast::bind_front_handler(&WebSocketClient::onResolve, shared_from_this()));
}

void WebSocketClient::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        std::cerr << "Resolve error: " << ec.message() << "\n";
        return;
    }
    net::async_connect(_socket.next_layer(), results,
        beast::bind_front_handler(&WebSocketClient::onConnect, shared_from_this()));
}

void WebSocketClient::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if (ec) {
        std::cerr << "Connect error: " << ec.message() << "\n";
        return;
    }
    _socket.async_handshake(_host, _target,
        beast::bind_front_handler(&WebSocketClient::onHandshake, shared_from_this()));
}

void WebSocketClient::onHandshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "Handshake error: " << ec.message() << "\n";
        return;
    }
    _connected = true;
    doRead();
}

void WebSocketClient::doRead() {
    _socket.async_read(_buffer,
        beast::bind_front_handler(&WebSocketClient::onRead, shared_from_this()));
}

void WebSocketClient::onRead(beast::error_code ec, std::size_t) {
    if (ec) {
        std::cerr << "Read error: " << ec.message() << "\n";
        _connected = false;
        return;
    }

    auto const data = _buffer.data();
    std::vector<uint8_t> v(
        boost::asio::buffers_begin(data),
        boost::asio::buffers_end(data)
    );
    if (onMessage) onMessage(v);
    _buffer.consume(_buffer.size());
    doRead();
}

void WebSocketClient::sendBinary(std::vector<uint8_t> data) {
    if (!_connected) return;

    // Schedule write safely on the strand
    net::post(_socket.get_executor(), [self = shared_from_this(), data = std::move(data)]() mutable {
        bool writeInProgress = !self->_writeQueue.empty();
        self->_writeQueue.push_back(std::move(data));
        if (!writeInProgress)
            self->doWrite();
    });
}

void WebSocketClient::doWrite() {
    if (!_connected) return;
    _socket.binary(true);
    auto &front = _writeQueue.front();
    _socket.async_write(net::buffer(front),
        beast::bind_front_handler(&WebSocketClient::onWrite, shared_from_this()));
}

void WebSocketClient::onWrite(beast::error_code ec, std::size_t) {
    if (ec) {
        std::cerr << "Write error: " << ec.message() << "\n";
        _connected = false;
        return;
    }

    _writeQueue.pop_front();
    if (!_writeQueue.empty()) doWrite();
}

void WebSocketClient::close() {
    _connected = false;
    beast::error_code ec;
    _socket.close(ws::close_code::normal, ec);
    if (ec)
        std::cerr << "Close error: " << ec.message() << "\n";
}
