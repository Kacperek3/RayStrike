#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include <fcntl.h>


class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void startLobbyBroadcast(const std::string& message, int port);
    void stopLobbyBroadcast();

    void startTCPListener(int preferredPort = 0);
    void stopTCPListener();

    int getTCPPort() const;

    void onClientConnected(std::function<void(std::string)> callback);

private:
    int udpSocket;
    int tcpSocket;
    sockaddr_in broadcastAddr;
    sockaddr_in tcpAddr;

    std::atomic<bool> broadcasting;
    std::atomic<bool> listening;

    std::thread broadcastThread;
    std::thread listenerThread;

    std::function<void(std::string)> clientConnectedCallback;
};
