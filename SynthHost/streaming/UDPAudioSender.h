//
// Created by Mircea Nealcos on 5/6/2025.
//

#ifndef UDPAUDIOSENDER_H
#define UDPAUDIOSENDER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

class UDPAudioSender {
public:
    UDPAudioSender(const char* ip, int port) {
        WSADATA wsaData;
        int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaerr != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("Failed to create UDP socket");
        }

        memset(&destAddr, 0, sizeof(destAddr));
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &destAddr.sin_addr);
    }

    ~UDPAudioSender() {
        closesocket(sockfd);
        WSACleanup();
    }

    void send(const float* samples, size_t sampleCount) {
        size_t byteCount = sampleCount * sizeof(float);
        int sent = sendto(sockfd,
                          reinterpret_cast<const char*>(samples),
                          static_cast<int>(byteCount),
                          0,
                          reinterpret_cast<sockaddr*>(&destAddr),
                          sizeof(destAddr));

        if (sent == SOCKET_ERROR) {
            std::cerr << "UDP send failed: " << WSAGetLastError() << std::endl;
        }
    }

private:
    SOCKET sockfd;
    sockaddr_in destAddr;
};
#endif //UDPAUDIOSENDER_H
