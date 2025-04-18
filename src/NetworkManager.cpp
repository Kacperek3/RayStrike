#include "NetworkManager.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

NetworkManager::NetworkManager() : udpSocket(-1), tcpSocket(-1), broadcasting(false), listening(false) {}

NetworkManager::~NetworkManager() {
    stopLobbyBroadcast();
    stopTCPListener();
}

void NetworkManager::startLobbyBroadcast(const std::string& message, int port) {
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    int enable = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    broadcasting = true;
    broadcastThread = std::thread([=]() {
        while (broadcasting) {
            sendto(udpSocket, message.c_str(), message.size(), 0,
                   (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void NetworkManager::stopLobbyBroadcast() {
    broadcasting = false;
    if (broadcastThread.joinable()) broadcastThread.join();
    if (udpSocket != -1) close(udpSocket);
}

void NetworkManager::startTCPListener(int preferredPort) {
   tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(preferredPort);

    if (bind(tcpSocket, (sockaddr*)&tcpAddr, sizeof(tcpAddr)) < 0) {
        std::cerr << "Failed to bind TCP socket\n";
        return;
    }

    listen(tcpSocket, SOMAXCONN);

    int flags = fcntl(tcpSocket, F_GETFL, 0);
    fcntl(tcpSocket, F_SETFL, flags | O_NONBLOCK);

    listening = true;

    listenerThread = std::thread([=]() {
        while (listening) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int client = accept(tcpSocket, (sockaddr*)&clientAddr, &clientLen);
            if (client >= 0) {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
                if (clientConnectedCallback) clientConnectedCallback(ip);
                close(client);
            } else {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept error");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    });
}

void NetworkManager::stopTCPListener() {
    listening = false;

    if (listenerThread.joinable() && std::this_thread::get_id() != listenerThread.get_id()) {
        std::cout << "Stopping TCP listener thread...\n";
        try {
            std::cout << "1";
            listenerThread.join();
            std::cout << "2";
        } catch (const std::system_error& e) {
            std::cerr << "Błąd przy joinowaniu wątku: " << e.what() << std::endl;
        }
    }

    if (tcpSocket != -1) {
        close(tcpSocket);
        tcpSocket = -1;
    }

}

int NetworkManager::getTCPPort() const {
    socklen_t len = sizeof(tcpAddr);
    getsockname(tcpSocket, (sockaddr*)&tcpAddr, &len);
    return ntohs(tcpAddr.sin_port);
}

void NetworkManager::onClientConnected(std::function<void(std::string)> callback) {
    clientConnectedCallback = callback;
}
