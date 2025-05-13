#include "controller/StreamController.h"
#include <iostream>
#include <boost/asio/io_context.hpp>

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512

using IoContext = boost::asio::io_context;

int main()
{
    IoContext ioContext;
    StreamController controller{ioContext};
    controller.addStreamManager(BLOCK_SIZE, SAMPLE_RATE, 9000, USER);
    controller.addWebSocketClient("localhost", "8080", "/user/preset", PRESET_CHANGER, &StreamController::changePreset);
    std::thread ioThread([&] { ioContext.run(); });
    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);)
    {
        if (line == "quit") break;
    }
    controller.shutdown();
    ioContext.stop();
    ioThread.join();
    return 0;
}
