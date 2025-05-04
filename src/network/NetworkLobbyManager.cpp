#include "NetworkLobbyManager.h"


NetworkLobbyManager::NetworkLobbyManager(int socketServerForClient, int socketClient) 
    : _socketServerForClient(socketServerForClient), _socketClient(socketClient) {
    _running = true;

    if(socketServerForClient != -1) {
        _isServer = true;
    } else if(socketClient != -1) {
        _isServer = false;
    } else {
        std::cerr << "Invalid socket configuration" << std::endl;
        return;
    }


    StartListening();
}

void NetworkLobbyManager::StartListening() {
    _receiveThread = std::thread(&NetworkLobbyManager::ReceiveThreadFunc, this);
}

void NetworkLobbyManager::ReceiveThreadFunc() {
    int activeSocket = _isServer ? _socketServerForClient : _socketClient;
    
    while (_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(activeSocket, &readSet);

        timeval timeout{0, 100000}; // 100ms timeout

        int result = select(activeSocket + 1, &readSet, nullptr, nullptr, &timeout);
        
        if (result > 0) {
            char buffer[1024];
            ssize_t received = recv(activeSocket, buffer, sizeof(buffer) - 1, 0);

            if (received > 0) {
                buffer[received] = '\0';
                std::lock_guard<std::mutex> lock(_queueMutex);
                _messageQueue.push(std::string(buffer));
                _cv.notify_one();
            }
            else if (received == 0 || (received == -1 && errno != EAGAIN)) {
                {
                    std::lock_guard<std::mutex> lock(_queueMutex);
                    _messageQueue.push("__DISCONNECTED__");
                    _connected = false;
                    _cv.notify_one();
                }
                break;
            }
        }
        else if (result == -1) {
            // Błąd select
            break;
        }
    }
}

NetworkLobbyManager::~NetworkLobbyManager() {
    _running = false;
    if (_receiveThread.joinable()) {
        _receiveThread.join();
    }
    
    if (_isServer) {
        close(_socketServerForClient);
    } else {
        close(_socketClient);
    }
}

bool NetworkLobbyManager::Send(const std::string& message) {
    if (!_connected) return false;

    int activeSocket = _isServer ? _socketServerForClient : _socketClient;
    std::string msg = message + '\n';
    ssize_t sent = send(activeSocket, msg.c_str(), msg.size(), MSG_NOSIGNAL);
    
    if (sent == -1) {
        _connected = false;
        return false;
    }
    return sent == (ssize_t)msg.size();
}

bool NetworkLobbyManager::HasMessages() {
    std::lock_guard<std::mutex> lock(_queueMutex);
    return !_messageQueue.empty();
}

std::string NetworkLobbyManager::PopMessage() {
    std::lock_guard<std::mutex> lock(_queueMutex);
    if (_messageQueue.empty()) return "";
    
    std::string msg = _messageQueue.front();
    _messageQueue.pop();
    return msg;
}

bool NetworkLobbyManager::WaitForMessage(std::string& outMessage, int timeoutMs) {
    std::unique_lock<std::mutex> lock(_queueMutex);
    if (_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !_messageQueue.empty(); })) {
        outMessage = _messageQueue.front();
        _messageQueue.pop();
        return true;
    }
    return false;
}
