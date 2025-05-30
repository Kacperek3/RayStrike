#include "UdpNetworkManager.h"
#include <cerrno> // For errno
#include <cstring> // For strerror (already in .h but good practice for .cpp if used directly)
#include <iostream> // For std::cerr (already in .h)

const char PORT_DATA_ACK = 'K'; // Define the ACK character

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
        if (_udpSocket == -1) {
            std::cerr << "Host: Failed to create UDP socket. errno: " << strerror(errno) << std::endl;
            return false;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = 0; // Bind to a random available port

        if (bind(_udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            std::cerr << "Host: Failed to bind UDP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }

        // Get assigned port
        socklen_t len = sizeof(serverAddr);
        if (getsockname(_udpSocket, (sockaddr*)&serverAddr, &len) == -1) {
            std::cerr << "Host: Failed to get socket name for UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        int myPort = ntohs(serverAddr.sin_port);
        std::cout << "Host: UDP socket bound to port: " << myPort << std::endl;

        // Send UDP port to client
        std::cout << "Host: Sending own UDP port " << myPort << " to client via TCP socket " << _tcpSocket << std::endl;
        ssize_t sentBytes = send(_tcpSocket, &myPort, sizeof(myPort), 0);
        if (sentBytes != sizeof(myPort)) {
            std::cerr << "Host: Failed to send UDP port to client. Sent " << sentBytes << " bytes, expected " << sizeof(myPort) << ". errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Host: Successfully sent own UDP port to client." << std::endl;

        // Wait for ACK from client that it received our port
        char clientAck = 0;
        std::cout << "Host: Waiting for ACK from client for host UDP port..." << std::endl;
        ssize_t ackReceivedBytes = recv(_tcpSocket, &clientAck, sizeof(clientAck), 0);
        if (ackReceivedBytes == -1) {
            std::cerr << "Host: recv failed when expecting ACK for host UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        } else if (ackReceivedBytes == 0) {
            std::cerr << "Host: TCP connection closed by client while waiting for ACK for host UDP port." << std::endl;
            close(_udpSocket);
            return false;
        } else if (ackReceivedBytes != sizeof(clientAck) || clientAck != PORT_DATA_ACK) {
            std::cerr << "Host: Failed to receive valid ACK for host UDP port. Received bytes: " << ackReceivedBytes << " char: " << (int)clientAck << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Host: Received ACK from client for host UDP port." << std::endl;

        // Get client UDP port (existing retry logic follows)
        int clientPort = 0; // Initialize to avoid using uninitialized value in logs if recv fails early
        std::cout << "Host: Waiting to receive client UDP port via TCP socket " << _tcpSocket << std::endl;
        
        const int MAX_RECV_RETRIES = 100; // Retry up to 100 times
        const std::chrono::milliseconds RECV_RETRY_DELAY(10); // Wait 10ms between retries
        ssize_t receivedBytes = -1;

        for (int i = 0; i < MAX_RECV_RETRIES; ++i) {
            receivedBytes = recv(_tcpSocket, &clientPort, sizeof(clientPort), 0);
            
            if (receivedBytes == sizeof(clientPort)) {
                break; // Successfully received the full port number
            }
            
            if (receivedBytes == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Data not yet available, socket is non-blocking
                    if (i < MAX_RECV_RETRIES - 1) { // Log retries, but not the last attempt if it also fails
                         // std::cout << "Host: recv for client UDP port: Resource temporarily unavailable (attempt " << i + 1 << "/" << MAX_RECV_RETRIES << "). Retrying..." << std::endl;
                    }
                    std::this_thread::sleep_for(RECV_RETRY_DELAY);
                    continue; // Retry
                } else {
                    // A different, more serious error occurred
                    std::cerr << "Host: recv failed when expecting client UDP port. errno: " << strerror(errno) << " (attempt " << i + 1 << ")" << std::endl;
                    close(_udpSocket);
                    return false;
                }
            } else if (receivedBytes == 0) {
                // TCP connection was closed by the client
                std::cerr << "Host: TCP connection closed by client while waiting for client UDP port (recv returned 0)." << std::endl;
                close(_udpSocket);
                return false;
            } else { // receivedBytes > 0 but not sizeof(clientPort)
                // Partial read, which is unusual for a small fixed-size piece of data like an int over TCP.
                // Treat as an error or retry. For simplicity, we'll log and let the loop retry.
                std::cerr << "Host: Received partial data for client UDP port. Expected " << sizeof(clientPort) << " bytes, got " << receivedBytes << " (attempt " << i + 1 << "). Retrying..." << std::endl;
                std::this_thread::sleep_for(RECV_RETRY_DELAY);
                // Potentially, one could try to read the remaining bytes, but for an int, a full retry is simpler.
            }
        }

        if (receivedBytes != sizeof(clientPort)) {
            std::cerr << "Host: Failed to receive client UDP port after " << MAX_RECV_RETRIES << " retries." << std::endl;
            if(receivedBytes == -1) {
                 std::cerr << "Host: Last recv error: " << strerror(errno) << std::endl;
            } else {
                 std::cerr << "Host: Last received bytes: " << receivedBytes << " (expected " << sizeof(clientPort) << ")" << std::endl;
            }
            close(_udpSocket);
            return false;
        }
        // clientPort is now populated
        std::cout << "Host: Received client UDP port: " << clientPort << std::endl;

        // Send ACK to client that we received their port
        char hostAck = PORT_DATA_ACK;
        std::cout << "Host: Sending ACK to client for client UDP port..." << std::endl;
        sentBytes = send(_tcpSocket, &hostAck, sizeof(hostAck), 0);
        if (sentBytes != sizeof(hostAck)) {
            std::cerr << "Host: Failed to send ACK for client UDP port. Sent " << sentBytes << " bytes, expected " << sizeof(hostAck) << ". errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Host: Successfully sent ACK for client UDP port." << std::endl;

        // Get client IP from TCP
        sockaddr_in tcpClientAddr{};
        socklen_t tcpLen = sizeof(tcpClientAddr);
        if (getpeername(_tcpSocket, (sockaddr*)&tcpClientAddr, &tcpLen) == -1) {
            std::cerr << "Host: Failed to get client IP address from TCP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        char clientIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &tcpClientAddr.sin_addr, clientIpStr, INET_ADDRSTRLEN);
        std::cout << "Host: Client IP from TCP: " << clientIpStr << ", Client TCP Port: " << ntohs(tcpClientAddr.sin_port) << std::endl;
        
        _remoteAddr = tcpClientAddr; // Copy IP
        _remoteAddr.sin_port = htons(clientPort); // Set to client's UDP port
        std::cout << "Host: Remote UDP address set to IP: " << clientIpStr << ", Port: " << clientPort << std::endl;

    }
    // Client side
    else {
        // Get server UDP port
        int serverPort;
        std::cout << "Client: Waiting to receive server UDP port via TCP socket " << _tcpSocket << std::endl;
        ssize_t receivedBytes = recv(_tcpSocket, &serverPort, sizeof(serverPort), 0);
        if (receivedBytes == -1) {
            std::cerr << "Client: recv failed when expecting server UDP port. errno: " << strerror(errno) << std::endl;
            return false;
        } else if (receivedBytes == 0) {
            std::cerr << "Client: TCP connection possibly closed by server while waiting for server UDP port (recv returned 0)." << std::endl;
            return false;
        } else if (receivedBytes != sizeof(serverPort)) {
            std::cerr << "Client: Failed to receive full server UDP port. Expected " << sizeof(serverPort) << " bytes, got " << receivedBytes << ". errno: " << strerror(errno) << std::endl;
            return false;
        }
        std::cout << "Client: Received server UDP port: " << serverPort << std::endl;

        // Send ACK to host that we received its port
        char clientAck = PORT_DATA_ACK;
        std::cout << "Client: Sending ACK to host for host UDP port..." << std::endl;
        ssize_t ackSentBytes = send(_tcpSocket, &clientAck, sizeof(clientAck), 0);
        if (ackSentBytes != sizeof(clientAck)) {
            std::cerr << "Client: Failed to send ACK for host UDP port. Sent " << ackSentBytes << " bytes, expected " << sizeof(clientAck) << ". errno: " << strerror(errno) << std::endl;
            // UDP socket not created yet, so no need to close it here
            return false;
        }
        std::cout << "Client: Successfully sent ACK for host UDP port." << std::endl;

        // Create UDP socket
        _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (_udpSocket == -1) {
            std::cerr << "Client: Failed to create UDP socket. errno: " << strerror(errno) << std::endl;
            return false;
        }

        sockaddr_in clientAddr{};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_addr.s_addr = INADDR_ANY;
        clientAddr.sin_port = 0; // Bind to a random available port

        if (bind(_udpSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
            std::cerr << "Client: Failed to bind UDP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }

        // Get client port and send to server
        socklen_t len = sizeof(clientAddr);
        if (getsockname(_udpSocket, (sockaddr*)&clientAddr, &len) == -1) {
            std::cerr << "Client: Failed to get socket name for UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        int myPort = ntohs(clientAddr.sin_port);
        std::cout << "Client: UDP socket bound to port: " << myPort << std::endl;

        //pause a bit to ensure the server is ready to receive
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Wait a bit to ensure server is ready

        std::cout << "Client: Sending own UDP port " << myPort << " to server via TCP socket " << _tcpSocket << std::endl;
        ssize_t sentBytes = send(_tcpSocket, &myPort, sizeof(myPort), 0);
        if (sentBytes != sizeof(myPort)) {
            std::cerr << "Client: Failed to send UDP port to server. Sent " << sentBytes << " bytes, expected " << sizeof(myPort) << ". errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Client: Successfully sent own UDP port to server." << std::endl;

        // Wait for ACK from host that it received our port
        char hostAck = 0;
        std::cout << "Client: Waiting for ACK from host for client UDP port..." << std::endl;
        receivedBytes = recv(_tcpSocket, &hostAck, sizeof(hostAck), 0);
        if (receivedBytes == -1) {
            std::cerr << "Client: recv failed when expecting ACK for client UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        } else if (receivedBytes == 0) {
            std::cerr << "Client: TCP connection closed by host while waiting for ACK for client UDP port." << std::endl;
            close(_udpSocket);
            return false;
        } else if (receivedBytes != sizeof(hostAck) || hostAck != PORT_DATA_ACK) {
            std::cerr << "Client: Failed to receive valid ACK for client UDP port. Received bytes: " << receivedBytes << " char: " << (int)hostAck << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Client: Received ACK from host for client UDP port." << std::endl;

        // Get server IP from TCP
        sockaddr_in tcpServerAddr{};
        socklen_t tcpLen = sizeof(tcpServerAddr);
        if (getpeername(_tcpSocket, (sockaddr*)&tcpServerAddr, &tcpLen) == -1) {
            std::cerr << "Client: Failed to get server IP address from TCP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        char serverIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &tcpServerAddr.sin_addr, serverIpStr, INET_ADDRSTRLEN);
        std::cout << "Client: Server IP from TCP: " << serverIpStr << ", Server TCP Port: " << ntohs(tcpServerAddr.sin_port) << std::endl;
        
        _remoteAddr = tcpServerAddr; // Copy IP
        _remoteAddr.sin_port = htons(serverPort); // Set to server's UDP port
        std::cout << "Client: Remote UDP address set to IP: " << serverIpStr << ", Port: " << serverPort << std::endl;
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