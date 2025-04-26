#include <fcntl.h>
#include <thread>
#include <chrono>
#include <sys/select.h>
#include "NetworkGameManager.h"

NetworkGameManager::NetworkGameManager(bool isServer, const std::string& ip, int port) 
    : _isServer(isServer), _isRunning(true) {
    
    if(_isServer) {
        SetupServer(port);
    } else {
        SetupClient(ip, port);
    }

    if(_connected) {
        _receiverThread = std::thread(&NetworkGameManager::ReceiveLoop, this);
    }
}

void NetworkGameManager::SetupServer(int port) {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
        std::cerr << "Server socket error: " << strerror(errno) << std::endl;
        return;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(_socket);
        return;
    }

    listen(_socket, SOMAXCONN);
    
    // Non-blocking accept
    fcntl(_socket, F_SETFL, O_NONBLOCK);
    
    // Wait for connection in separate thread
    std::thread([this](){
        sockaddr_in client{};
        socklen_t clientSize = sizeof(client);
        
        while(_isRunning && !_connected) {
            _clientSocket = accept(_socket, (sockaddr*)&client, &clientSize);
            if(_clientSocket > 0) {
                fcntl(_clientSocket, F_SETFL, O_NONBLOCK);
                _connected = true;
                std::cout << "Client connected!\n";
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

void NetworkGameManager::SetupClient(const std::string& ip, int port) {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
        std::cerr << "Client socket error: " << strerror(errno) << std::endl;
        return;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

    // Non-blocking connect
    fcntl(_socket, F_SETFL, O_NONBLOCK);
    
    if (connect(_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
        if(errno != EINPROGRESS) {
            std::cerr << "Connect error: " << strerror(errno) << std::endl;
            return;
        }
    }

    // Wait for connection
    fd_set set;
    FD_ZERO(&set);
    FD_SET(_socket, &set);
    timeval timeout{5, 0};
    
    if(select(_socket + 1, nullptr, &set, nullptr, &timeout) > 0) {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(_socket, SOL_SOCKET, SO_ERROR, &so_error, &len);
        
        if(so_error == 0) {
            _connected = true;
            std::cout << "Connected to server!\n";
        }
    }
}

NetworkGameManager::~NetworkGameManager() {
    _isRunning = false;
    if(_receiverThread.joinable()) {
        _receiverThread.join();
    }
    close(_socket);
    if(_isServer && _clientSocket != -1) {
        close(_clientSocket);
    }
}

void NetworkGameManager::ReceiveLoop() {
    while (_isRunning && _connected) {
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
    if(currentSocket <= 0) return false;

    float data[2]{};
    int bytesReceived = recv(currentSocket, data, sizeof(data), 0);
    
    if(bytesReceived == sizeof(data)) {
        x = data[0];
        y = data[1];
        return true;
    }
    
    if(bytesReceived == 0) {
        std::cerr << "Connection closed\n";
        _connected = false;
    }
    
    return false;
}

void NetworkGameManager::SendPosition(float x, float y) {
    if(!_connected) return;

    int currentSocket = _isServer ? _clientSocket : _socket;
    if(currentSocket <= 0) return;

    float data[2] = {x, y};
    int result = send(currentSocket, data, sizeof(data), MSG_NOSIGNAL);
    
    if(result == -1) {
        if(errno == EPIPE) {
            std::cerr << "Connection closed by peer\n";
            _connected = false;
        }
    }
}

void NetworkGameManager::GetEnemyPosition(float& x, float& y) {
    std::lock_guard<std::mutex> lock(_dataMutex);
    x = _enemyX;
    y = _enemyY;
}