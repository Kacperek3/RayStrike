#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

class NetworkLobbyManager {
public:
    NetworkLobbyManager(int socketServerForClient, int socketClient);
    ~NetworkLobbyManager();

    void StartListening();
    bool Send(const std::string& message);
    bool HasMessages();
    std::string PopMessage();
    bool WaitForMessage(std::string& outMessage, int timeoutMs);

    bool IsConnected() const { return _connected; }
    void ReleaseSocket(); 
    bool _isServer = false;

private:
    void ReceiveThreadFunc();

    std::string _receiveBuffer;
    int _socketServerForClient;
    int _socketClient;
    std::atomic<bool> _running{false};
    std::atomic<bool> _connected{true};
    
    std::thread _receiveThread;
    std::mutex _queueMutex;
    std::condition_variable _cv;
    std::queue<std::string> _messageQueue;
};