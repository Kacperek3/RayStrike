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

ssize_t UdpNetworkManager::recv_all(int sockfd, void *buf, size_t len, int flags) {
    size_t total_received = 0;
    char *p = static_cast<char*>(buf);

    while (total_received < len) {
        ssize_t received_now = recv(sockfd, p + total_received, len - total_received, flags);

        if (received_now == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Gniazdo nieblokujące, brak danych w tej chwili
                // std::cout << "recv_all: Resource temporarily unavailable. Retrying..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Krótkie opóźnienie przed ponowieniem
                continue;
            }
            // Inny błąd
            std::cerr << "recv_all failed with error: " << strerror(errno) << std::endl;
            return -1;
        } else if (received_now == 0) {
            // Połączenie zamknięte przez drugą stronę
            std::cerr << "recv_all: Connection closed by peer." << std::endl;
            return 0; // lub -1 jeśli traktujemy to jako błąd w tym kontekście
        }
        total_received += received_now;
    }
    return total_received;
}


bool UdpNetworkManager::SetupUdpConnection() {
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Setting up UDP connection..." << std::endl;
    ssize_t bytes_processed;

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

        socklen_t len = sizeof(serverAddr);
        if (getsockname(_udpSocket, (sockaddr*)&serverAddr, &len) == -1) {
            std::cerr << "Host: Failed to get socket name for UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        int myPort = ntohs(serverAddr.sin_port);
        std::cout << "Host: UDP socket bound to port: " << myPort << std::endl;

        // 1. Host sends its UDP port to Client
        
        std::cout << "Host: Sending own UDP port " << myPort << " to client via TCP socket " << _tcpSocket << std::endl;
        
        //wait for the client to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(700)); // Optional: wait for client to be ready
        
        bytes_processed = send(_tcpSocket, &myPort, sizeof(myPort), 0);
        if (bytes_processed != sizeof(myPort)) {
            std::cerr << "Host: Failed to send UDP port to client. Sent " << bytes_processed << ". errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Host: Successfully sent own UDP port to client." << std::endl; // Added log

        int clientPort = 0;
        std::cout << "Host: Waiting to receive client UDP port via TCP socket " << _tcpSocket << std::endl;
        bytes_processed = recv_all(_tcpSocket, &clientPort, sizeof(clientPort), 0);
         if (bytes_processed <= 0) { // recv_all zwróci -1 lub 0 przy błędzie/zamknięciu
            std::cerr << "Host: Failed to receive client UDP port. recv_all returned " << bytes_processed << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "Host: Received client UDP port: " << clientPort << std::endl;

        sockaddr_in tcpClientAddr{};
        socklen_t tcpLen = sizeof(tcpClientAddr);
        if (getpeername(_tcpSocket, (sockaddr*)&tcpClientAddr, &tcpLen) == -1) {
            std::cerr << "Host: Failed to get client IP address from TCP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        char clientIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &tcpClientAddr.sin_addr, clientIpStr, INET_ADDRSTRLEN);
        
        _remoteAddr = tcpClientAddr;
        _remoteAddr.sin_port = htons(clientPort);
        std::cout << "Host: Remote UDP address set to IP: " << clientIpStr << ", Port: " << clientPort << std::endl;

    }
    // Client side
    else {
        // 1. Client waits to receive Host's UDP port
        int serverPort;
        std::cout << "Client: Waiting to receive server UDP port via TCP socket " << _tcpSocket << std::endl;
        bytes_processed = recv_all(_tcpSocket, &serverPort, sizeof(serverPort), 0);
        if (bytes_processed <= 0) {
            std::cerr << "Client: Failed to receive server UDP port. recv_all returned " << bytes_processed << std::endl;
            return false;
        }
        std::cout << "Client: Received server UDP port: " << serverPort << std::endl;

        // 2. Client sends ACK to Host (confirming Client received Host's port) - REMOVED
        // std::cout << "Client: Sending ACK to host for host UDP port..." << std::endl;
        
        // //wait for the host to be ready
        // std::this_thread::sleep_for(std::chrono::milliseconds(1100)); // Optional: wait for host to be ready

        // bytes_processed = send(_tcpSocket, &ack_char, sizeof(ack_char), 0);
        // if (bytes_processed != sizeof(ack_char)) {
        //     std::cerr << "Client: Failed to send ACK for host UDP port. Sent " << bytes_processed << ". errno: " << strerror(errno) << std::endl;
        //     return false;
        // }
        // std::cout << "Client: Successfully sent ACK for host UDP port." << std::endl;

        // Create and bind client's UDP socket
        _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (_udpSocket == -1) {
            std::cerr << "Client: Failed to create UDP socket. errno: " << strerror(errno) << std::endl;
            return false;
        }
        sockaddr_in clientAddr{};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_addr.s_addr = INADDR_ANY;
        clientAddr.sin_port = 0;
        if (bind(_udpSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
            std::cerr << "Client: Failed to bind UDP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        socklen_t len = sizeof(clientAddr);
        if (getsockname(_udpSocket, (sockaddr*)&clientAddr, &len) == -1) {
            std::cerr << "Client: Failed to get socket name for UDP port. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        int myPort = ntohs(clientAddr.sin_port);
        std::cout << "Client: UDP socket bound to port: " << myPort << std::endl;


        // 3. Client sends its UDP port to Host
        std::cout << "Client: Sending own UDP port " << myPort << " to server via TCP socket " << _tcpSocket << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(4000)); // Optional: wait for host to be ready
        //wait for the host to be ready


        for(int i = 0; i < 10; ++i) {
            std::cout << "Client: Attempting to send own UDP port to server, attempt " << (i + 1) << std::endl;
            bytes_processed = send(_tcpSocket, &myPort, sizeof(myPort), 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait a bit before retrying
        }
        std::cout << "send penis" << std::endl;

        if (bytes_processed != sizeof(myPort)) {
            std::cerr << "Client: Failed to send UDP port to server. Sent " << bytes_processed << ". errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        std::cout << "send penis 2" << std::endl;

        std::cout << "Client: Successfully sent own UDP port to server." << std::endl; // Added log

        // 4. Client waits for ACK from Host (confirming Host received Client's port) - REMOVED
        // std::cout << "Client: Waiting for ACK from host for client UDP port..." << std::endl;
        // bytes_processed = recv_all(_tcpSocket, &received_ack_char, sizeof(received_ack_char), 0);
        // if (bytes_processed <= 0 || received_ack_char != ack_char) {
        //     std::cerr << "Client: Failed to receive valid ACK for client UDP port. Bytes: " << bytes_processed << " Char: " << (int)received_ack_char << std::endl;
        //     close(_udpSocket);
        //     return false;
        // }
        // std::cout << "Client: Received ACK from host for client UDP port." << std::endl;

        // Configure remote address for UDP
        sockaddr_in tcpServerAddr{};
        socklen_t tcpLen = sizeof(tcpServerAddr);
        if (getpeername(_tcpSocket, (sockaddr*)&tcpServerAddr, &tcpLen) == -1) {
            std::cerr << "Client: Failed to get server IP address from TCP socket. errno: " << strerror(errno) << std::endl;
            close(_udpSocket);
            return false;
        }
        char serverIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &tcpServerAddr.sin_addr, serverIpStr, INET_ADDRSTRLEN);
        
        _remoteAddr = tcpServerAddr;
        _remoteAddr.sin_port = htons(serverPort);
        std::cout << "Client: Remote UDP address set to IP: " << serverIpStr << ", Port: " << serverPort << std::endl;
    }

    _connected = true;
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: UDP connection handshake complete. Connected flag set to true." << std::endl;
    // Gniazdo TCP (_tcpSocket) może zostać zamknięte tutaj, jeśli nie będzie już potrzebne.
    // Jednakże, jeśli planujesz używać go do dalszej sygnalizacji (np. rozłączanie), zostaw je otwarte.
    // Dla tego przykładu zakładam, że nie jest już potrzebne do komunikacji UDP.
    // close(_tcpSocket); // Rozważ zamknięcie TCP po konfiguracji
    // _tcpSocket = -1;
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