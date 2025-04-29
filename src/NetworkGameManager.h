#pragma once
#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <mutex>
#include <errno.h>
#include <string>

class NetworkGameManager {
public:
    NetworkGameManager(bool isServer, const std::string& ip, int port);
    ~NetworkGameManager();

    void SendPosition(float x, float y);
    bool IsServer() const { return _isServer; }
    void GetEnemyPosition(float& x, float& y);
    bool IsConnected() const { return _connected; }

private:
    std::thread _receiverThread;
    std::mutex _dataMutex;
    float _enemyX = 0.0f;
    float _enemyY = 0.0f;
    bool _isRunning;
    bool _connected;

    void ReceiveLoop();
    bool ReceivePosition(float& x, float& y);
    
    int _socket;
    int _clientSocket = -1;
    bool _isServer;

    void SetupServer(int port);
    void SetupClient(const std::string& ip, int port);
};