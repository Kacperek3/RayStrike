#include "UdpNetworkManager.h"

UdpNetworkManager::UdpNetworkManager(int tcpSocketServer, int tcpSocketClient) {
    if (tcpSocketServer != -1) {
        _isServer = true;
        _tcpSocket = tcpSocketServer;
    } else {
        _isServer = false;
        _tcpSocket = tcpSocketClient;
    }
    memset(&_remoteAddr, 0, sizeof(_remoteAddr));
}

UdpNetworkManager::~UdpNetworkManager() {
    _running = false;
    if (_receiveThread.joinable()) {
        _receiveThread.join();
    }
    if (_udpSocket != -1) close(_udpSocket);
}

bool UdpNetworkManager::Initialize() {
    if (!SetupUdpConnection()) return false;
    
    _running = true;
    _receiveThread = std::thread(&UdpNetworkManager::ReceiveThreadFunc, this);
    return true;
}

bool UdpNetworkManager::SetupUdpConnection() {
    // Server side
    if (_isServer) {
        _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (_udpSocket == -1) return false;

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = 0;

        if (bind(_udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            close(_udpSocket);
            return false;
        }

        // Get assigned port
        socklen_t len = sizeof(serverAddr);
        getsockname(_udpSocket, (sockaddr*)&serverAddr, &len);
        int myPort = ntohs(serverAddr.sin_port);

        // Send UDP port to client
        if (send(_tcpSocket, &myPort, sizeof(myPort), 0) != sizeof(myPort)) return false;

        // Get client UDP port
        int clientPort;
        if (recv(_tcpSocket, &clientPort, sizeof(clientPort), 0) != sizeof(clientPort)) return false;

        // Get client IP from TCP
        sockaddr_in tcpClientAddr{};
        socklen_t tcpLen = sizeof(tcpClientAddr);
        getpeername(_tcpSocket, (sockaddr*)&tcpClientAddr, &tcpLen);
        
        _remoteAddr = tcpClientAddr;
        _remoteAddr.sin_port = htons(clientPort);
    }
    // Client side
    else {
        // Get server UDP port
        int serverPort;
        if (recv(_tcpSocket, &serverPort, sizeof(serverPort), 0) != sizeof(serverPort)) return false;

        // Create UDP socket
        _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (_udpSocket == -1) return false;

        sockaddr_in clientAddr{};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_addr.s_addr = INADDR_ANY;
        clientAddr.sin_port = 0;

        if (bind(_udpSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
            close(_udpSocket);
            return false;
        }

        // Get client port and send to server
        socklen_t len = sizeof(clientAddr);
        getsockname(_udpSocket, (sockaddr*)&clientAddr, &len);
        int myPort = ntohs(clientAddr.sin_port);
        if (send(_tcpSocket, &myPort, sizeof(myPort), 0) != sizeof(myPort)) return false;

        // Get server IP from TCP
        sockaddr_in tcpServerAddr{};
        socklen_t tcpLen = sizeof(tcpServerAddr);
        getpeername(_tcpSocket, (sockaddr*)&tcpServerAddr, &tcpLen);
        
        _remoteAddr = tcpServerAddr;
        _remoteAddr.sin_port = htons(serverPort);
    }
    _connected = true;
    return true;
}

void UdpNetworkManager::ReceiveThreadFunc() {
    char buffer[2048];
    sockaddr_in fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);

    while (_running) {
        ssize_t received = recvfrom(_udpSocket, buffer, sizeof(buffer), 0, 
                                  (sockaddr*)&fromAddr, &fromLen);
        if (received > 0) {
            std::lock_guard<std::mutex> lock(_queueMutex);
            _messageQueue.emplace(buffer, received);
            _cv.notify_one();
        }
        else if (received == -1 && errno != EAGAIN) {
            _connected = false;
            break;
        }
    }
}

bool UdpNetworkManager::Send(const std::string& message) {
    if (!_connected) return false;

    ssize_t sent = sendto(_udpSocket, message.data(), message.size(), 0,
                         (sockaddr*)&_remoteAddr, sizeof(_remoteAddr));
    return sent == static_cast<ssize_t>(message.size());
}

bool UdpNetworkManager::HasMessages() {
    std::lock_guard<std::mutex> lock(_queueMutex);
    return !_messageQueue.empty();
}

std::string UdpNetworkManager::PopMessage() {
    std::lock_guard<std::mutex> lock(_queueMutex);
    if (_messageQueue.empty()) return "";
    
    auto msg = _messageQueue.front();
    _messageQueue.pop();
    return msg;
}

bool UdpNetworkManager::WaitForMessage(std::string& outMessage, int timeoutMs) {
    std::unique_lock<std::mutex> lock(_queueMutex);
    if (_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                   [this] { return !_messageQueue.empty(); })) {
        outMessage = _messageQueue.front();
        _messageQueue.pop();
        return true;
    }
    return false;
}