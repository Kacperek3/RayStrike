
#include <fcntl.h>
#include <thread>
#include <chrono>
#include "NetworkGameManager.h"

NetworkGameManager::NetworkGameManager(bool isServer, const std::string& ip, int port) 
    : _isServer(isServer), _isRunning(true) {
    
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return;
    }

    // Ustaw gniazdo jako nieblokujące
    int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

   // NetworkGameManager.cpp - poprawiona sekcja connect
    if (!_isServer) {
        if (connect(_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
            if (errno != EINPROGRESS) { // Tylko dla non-blocking socket
                std::cerr << "Connect failed: " << strerror(errno) << std::endl;
                close(_socket);
                return;
            }
            
            // Czekaj na zakończenie połączenia (ważne dla non-blocking)
            fd_set set;
            FD_ZERO(&set);
            FD_SET(_socket, &set);
            timeval timeout{5, 0}; // 5 sekund timeout
            if (select(_socket + 1, nullptr, &set, nullptr, &timeout) <= 0) {
                std::cerr << "Connection timeout" << std::endl;
                close(_socket);
                return;
            }
        }
    }

    _receiverThread = std::thread(&NetworkGameManager::ReceiveLoop, this);
}

NetworkGameManager::~NetworkGameManager() {
    _isRunning = false;
    if (_receiverThread.joinable()) {
        _receiverThread.join();
    }
    close(_socket);
    if (_isServer) {
        close(_clientSocket);
    }
}

void NetworkGameManager::ReceiveLoop() {
    while (_isRunning) {
        float x, y;
        if (ReceivePosition(x, y)) {
            std::lock_guard<std::mutex> lock(_dataMutex);
            _enemyX = x;
            _enemyY = y;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool NetworkGameManager::ReceivePosition(float& x, float& y) {
    int currentSocket = _isServer ? _clientSocket : _socket;
    float data[2]{};
    
    int bytesReceived = recv(currentSocket, data, sizeof(data), 0);
    if (bytesReceived == sizeof(data)) {
        x = data[0];
        y = data[1];
        return true;
    } else if (bytesReceived == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return false;
        } else {
            std::cerr << "Recv error: " << strerror(errno) << std::endl;
            return false;
        }
    } else if (bytesReceived == 0) {
        std::cerr << "Connection closed" << std::endl;
        return false;
    }
    return false;
}

void NetworkGameManager::GetEnemyPosition(float& x, float& y) {
    std::lock_guard<std::mutex> lock(_dataMutex);
    x = _enemyX;
    y = _enemyY;
}
// NetworkGameManager.cpp
void NetworkGameManager::SendPosition(float x, float y) {
    int currentSocket = _isServer ? _clientSocket : _socket;
    
    if (currentSocket <= 0) {
        std::cerr << "Invalid socket descriptor!" << std::endl;
        return;
    }

    float data[2] = {x, y};
    int result = send(currentSocket, data, sizeof(data), MSG_NOSIGNAL);
    
    if (result == -1) {
        if (errno == EPIPE) {
            std::cerr << "Connection closed by peer" << std::endl;
        } else {
            std::cerr << "Send error: " << strerror(errno) << std::endl;
        }
    }
}