#include "utils/StreamManager.h"
#include <iostream>

#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512

int main()
{
    StreamManager userStreamManager(BLOCK_SIZE, SAMPLE_RATE, 9000);
    userStreamManager.startStreaming();
    std::cout << "Type `quit` + Enter to exit.\n";
    for (std::string line; std::getline(std::cin, line);)
    {
        if (line == "quit") break;
    }
    userStreamManager.stopStreaming();
    return 0;
}
