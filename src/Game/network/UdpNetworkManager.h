#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>

class UdpNetworkManager {
public:
    UdpNetworkManager(int tcpSocketServer, int tcpSocketClient);
    ~UdpNetworkManager();

    bool Initialize();
    bool Send(const std::string& message);
    bool HasMessages();
    std::string PopMessage();
    bool WaitForMessage(std::string& outMessage, int timeoutMs);

    bool IsConnected() const { return _connected; }

private:
    void ReceiveThreadFunc();
    bool SetupUdpConnection();

    int _tcpSocket;
    int _udpSocket = -1;
    bool _isServer;
    struct sockaddr_in _remoteAddr;

    std::atomic<bool> _running{false};
    std::atomic<bool> _connected{false};

    std::thread _receiveThread;
    std::mutex _queueMutex;
    std::condition_variable _cv;
    std::queue<std::string> _messageQueue;
};
