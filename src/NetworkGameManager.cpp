#include "NetworkGameManager.h"

NetworkGameManager::NetworkGameManager(bool isServer, const std::string& ip, int port) : _isServer(isServer)
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
        std::cerr << "Error creating socket!" << std::endl;
        return;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = inet_addr(ip.c_str());

    if (isServer) {
        if (bind(_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
            std::cerr << "Bind failed!" << std::endl;
            return;
        }

        listen(_socket, SOMAXCONN);

        sockaddr_in client;
        socklen_t clientSize = sizeof(client);
        _clientSocket = accept(_socket, (sockaddr*)&client, &clientSize);
        if (_clientSocket == -1) {
            std::cerr << "Client connection failed!" << std::endl;
            return;
        }
    } else {
        if (connect(_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
            std::cerr << "Connection failed!" << std::endl;
            return;
        }
    }
}

NetworkGameManager::~NetworkGameManager() {
    if (_isServer) {
        if (_clientSocket != -1) {
            close(_clientSocket);
        }
    }
    close(_socket);
}

void NetworkGameManager::SendPosition(float x, float y) {
    float data[2] = { x, y };

    if (_isServer) {
        send(_clientSocket, data, sizeof(data), 0);
    } else {
        send(_socket, data, sizeof(data), 0);
    }
}

bool NetworkGameManager::ReceivePosition(float& x, float& y) {
    float data[2];
    int bytesReceived = 0;

    if (_isServer) {
        bytesReceived = recv(_clientSocket, data, sizeof(data), 0);
    } else {
        bytesReceived = recv(_socket, data, sizeof(data), 0);
    }

    if (bytesReceived <= 0) {
        return false; // Rozłączono lub błąd
    }

    x = data[0];
    y = data[1];
    return true;
}
