#include "UdpNetworkManager.h"
#include <cerrno> // For errno
#include <cstring> // For strerror (already in .h but good practice for .cpp if used directly)
#include <iostream> // For std::cerr (already in .h)

// ...existing code...
UdpNetworkManager::UdpNetworkManager(int tcpSocketServer, int tcpSocketClient) {
    if (tcpSocketServer != -1) {
        _isServer = true;
        _tcpSocket = tcpSocketServer;
        std::cout << "UdpNetworkManager created for HOST on TCP socket: " << _tcpSocket << std::endl;
    } else {
        _isServer = false;
        _tcpSocket = tcpSocketClient;
        std::cout << "UdpNetworkManager created for CLIENT on TCP socket: " << _tcpSocket << std::endl;
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
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager initializing..." << std::endl;
    if (!SetupUdpConnection()) {
        std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: SetupUdpConnection failed." << std::endl;
        return false;
    }
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: UDP connection setup complete. UDP Socket: " << _udpSocket << std::endl;
    
    _running = true;
    _receiveThread = std::thread(&UdpNetworkManager::ReceiveThreadFunc, this);
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Receive thread started." << std::endl;
    return true;
}

bool UdpNetworkManager::SetupUdpConnection() {
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Setting up UDP connection..." << std::endl;
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
        std::cout << "Client: Waiting to receive server UDP port via TCP socket " << _tcpSocket << std::endl;
        if (recv(_tcpSocket, &serverPort, sizeof(serverPort), 0) != sizeof(serverPort)) {
            std::cerr << "Client: Failed to receive server UDP port. errno: " << strerror(errno) << std::endl;
            return false;
        }
        std::cout << "Client: Received server UDP port: " << serverPort << std::endl;

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
        std::cout << "Client: Sent own UDP port " << myPort << " to server." << std::endl;

        // Get server IP from TCP
        sockaddr_in tcpServerAddr{};
        socklen_t tcpLen = sizeof(tcpServerAddr);
        getpeername(_tcpSocket, (sockaddr*)&tcpServerAddr, &tcpLen);
        
        _remoteAddr = tcpServerAddr;
        _remoteAddr.sin_port = htons(serverPort);
    }
    _connected = true;
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: UDP connected flag set to true." << std::endl;
    return true;
}

void UdpNetworkManager::ReceiveThreadFunc() {
    char buffer[2048];
    sockaddr_in fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);

    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: ReceiveThreadFunc started." << std::endl;

    while (_running) {
        if (_udpSocket == -1) {
            // std::cerr << (_isServer ? "Host" : "Client") << " UDP socket not initialized in ReceiveThreadFunc." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait a bit if socket not ready
            continue;
        }

        ssize_t received = recvfrom(_udpSocket, buffer, sizeof(buffer) - 1, 0, // sizeof(buffer) - 1 to leave space for null terminator
                                  (sockaddr*)&fromAddr, &fromLen);

        if (received > 0) {
            buffer[received] = '\\0'; // Null-terminate the received data

            // Basic validation: check if message is from the expected peer
            // This is a simplified check; in a real-world scenario, you might want to be more robust,
            // especially if not using a session-based protocol over UDP.
            // if (_connected && fromAddr.sin_addr.s_addr == _remoteAddr.sin_addr.s_addr && fromAddr.sin_port == _remoteAddr.sin_port) {
                std::lock_guard<std::mutex> lock(_queueMutex);
                _messageQueue.push(std::string(buffer));
                _cv.notify_one(); // Notify any waiting thread
            // } else {
            //     // Message from an unexpected source or when not "connected"
            //     // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Received packet from unexpected source or while not connected." << std::endl;
            // }
        } else if (received == 0) {
            // For UDP, recvfrom returning 0 is unusual.
            // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: recvfrom returned 0." << std::endl;
        } else { // received < 0
            if (!_running) break; // Exit if thread is being stopped

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Non-blocking socket and no data, or timeout on blocking socket with SO_RCVTIMEO
                // This is not necessarily an error, just means no data at the moment.
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Small sleep to prevent busy loop if socket is non-blocking
            } else if (errno == ECONNREFUSED) {
                // This can happen in UDP if a previous sendto to this address resulted in an ICMP "port unreachable".
                // The system might then refuse subsequent recvfrom calls from that address for a while.
                // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: UDP Receive error - Connection refused (ECONNREFUSED)." << std::endl;
            } else {
                std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: UDP Receive error in ReceiveThreadFunc: " << strerror(errno) << " (errno: " << errno << ")" << std::endl;
                if (errno == EBADF) { // Bad file descriptor, socket might have been closed
                    _running = false; // Stop the thread
                    break;
                }
                // For other errors, we might just log and continue, or implement more specific error handling.
            }
        }
    }
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: ReceiveThreadFunc ended." << std::endl;
}

bool UdpNetworkManager::Send(const std::string& message) {
    if (!_connected || _udpSocket == -1) {
        // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Cannot send, not connected or UDP socket invalid." << std::endl;
        return false;
    }
    if (message.empty()) {
        // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Attempted to send an empty message." << std::endl;
        return false; // Don't send empty messages
    }

    ssize_t sent = sendto(_udpSocket, message.data(), message.size(), 0,
                         (sockaddr*)&_remoteAddr, sizeof(_remoteAddr));
    if (sent == -1) {
        // std::cerr << (_isServer ? "Host" : "Client") << " UdpNetworkManager: sendto failed: " << strerror(errno) << std::endl;
    }
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