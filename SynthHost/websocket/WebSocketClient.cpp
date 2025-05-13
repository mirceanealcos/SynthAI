#include "WebSocketClient.h"

#include <iostream>
#include <utility>

WebSocketClient::WebSocketClient(net::io_context &ioContext, std::string host, std::string port, std::string url,
                                 WebSocketClientID id)
    : resolver(net::make_strand(ioContext)), socket(net::make_strand(ioContext)), host(std::move(host)),
      port(std::move(port)),
      url(std::move(url)), id(id) {
}

void WebSocketClient::run() {
    resolver.async_resolve(host, port, beast::bind_front_handler(&WebSocketClient::onResolve, shared_from_this()));
}


void WebSocketClient::onJson(JsonHandler jsonHandler) {
    this->jsonHandler = std::move(jsonHandler);
}

void WebSocketClient::sendJson(const json &json) {
    auto stringJson = json.dump();
    net::post(socket.get_executor(), [self = shared_from_this(), stringJson=std::move(stringJson)]() {
        self->socket.async_write(net::buffer(stringJson), beast::bind_front_handler(&WebSocketClient::onWrite, self));
    });
}

void WebSocketClient::close() {
    if (closing) {
        return;
    }
    closing = true;
    net::post(socket.get_executor(), [self = shared_from_this()]() {
        self->socket.async_close(websocket::close_code::normal,
                                 beast::bind_front_handler(&WebSocketClient::onClose, self));
    });
}

void WebSocketClient::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        return fail(ec, "resolve");
    }
    net::async_connect(
        socket.next_layer(),
        results,
        beast::bind_front_handler(
            &WebSocketClient::onConnect,
            shared_from_this()
        )
    );
}

void WebSocketClient::onConnect(beast::error_code ec, tcp::endpoint) {
    if (ec) return fail(ec, "connect");
    socket.async_handshake(host, url, beast::bind_front_handler(&WebSocketClient::onHandshake, shared_from_this()));
}

void WebSocketClient::onHandshake(beast::error_code ec) {
    if (ec) return fail(ec, "handshake");
    doRead();
}

void WebSocketClient::doRead() {
    socket.async_read(buffer, beast::bind_front_handler(&WebSocketClient::onRead, shared_from_this()));
}

void WebSocketClient::onRead(beast::error_code ec, std::size_t bytes) {
    if (ec) {
        if (ec != websocket::error::closed) {
            fail(ec, "read");
        }
        return;
    }
    try {
        auto message = beast::buffers_to_string(buffer.data());
        auto j = json::parse(message);
        if (jsonHandler) jsonHandler(j);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    buffer.consume(bytes);
    if (!closing) doRead();
}

void WebSocketClient::onWrite(beast::error_code ec, std::size_t bytes) {
    if (ec) fail(ec, "write");
}

void WebSocketClient::onClose(beast::error_code ec) {
    if (ec) fail(ec, "close");
}

void WebSocketClient::fail(beast::error_code ec, char const *what) {
    std::cout << what << ": " << ec.message() << std::endl;
}

WebSocketClientID WebSocketClient::getID() {
    return id;
}
