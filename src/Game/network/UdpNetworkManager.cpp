#include "UdpNetworkManager.h"
#include <cerrno> // For errno
#include <cstring> // For strerror (already in .h but good practice for .cpp if used directly)
#include <iostream> // For std::cerr (already in .h)

const int SYNC_MAGIC_REQ = 0x11223344;
const int SYNC_MAGIC_ACK = 0x55667788;

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

ssize_t UdpNetworkManager::recv_all(int sockfd, void *buf, size_t len, int flags, int timeoutMs) {
    size_t total_received = 0;
    char *p = static_cast<char*>(buf);
    auto startTime = std::chrono::steady_clock::now();

    while (total_received < len) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > timeoutMs) {
            std::cerr << "recv_all: Timeout waiting for data." << std::endl;
            return -1;
        }

        ssize_t received_now = recv(sockfd, p + total_received, len - total_received, flags);

        if (received_now == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
            std::cerr << "recv_all failed: " << strerror(errno) << std::endl;
            return -1;
        } else if (received_now == 0) {
            std::cerr << "recv_all: Connection closed by peer." << std::endl;
            return 0;
        }
        total_received += received_now;
    }
    return total_received;
}


bool UdpNetworkManager::SyncTcpHandshake() {
    int timeout = 5000; 
    auto start = std::chrono::steady_clock::now();

    int targetMagic = _isServer ? SYNC_MAGIC_REQ : SYNC_MAGIC_ACK;
    const char* targetPtr = reinterpret_cast<const char*>(&targetMagic);
    std::string role = _isServer ? "Host" : "Client";

    if (!_isServer) {
        std::cout << "Client: Sending handshake request..." << std::endl;
        int req = SYNC_MAGIC_REQ;
        if (send(_tcpSocket, &req, sizeof(req), 0) != sizeof(req)) return false;
        std::cout << "Client: Waiting for Host ACK (Sliding Window Scan)..." << std::endl;
    } else {
        std::cout << "Host: Waiting for Client handshake (Sliding Window Scan)..." << std::endl;
    }

    std::vector<char> history;
    bool synced = false;

    while (!synced) {
        char byte = 0;
        ssize_t res = recv(_tcpSocket, &byte, 1, MSG_DONTWAIT);
        
        if (res == 1) {
            history.push_back(byte);
            if (history.size() > sizeof(int)) {
                history.erase(history.begin());
            }

            if (history.size() == sizeof(int)) {
                if (memcmp(history.data(), targetPtr, sizeof(int)) == 0) {
                    std::cout << role << ": Handshake FOUND despite garbage!" << std::endl;
                    synced = true;
                }
            }
        } else if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else if (res == 0) {
            std::cerr << role << ": Connection closed during handshake." << std::endl;
            return false;
        }

        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > timeout) {
            std::cerr << role << ": Handshake timeout!" << std::endl;
            return false;
        }
    }

    if (_isServer) {
        int ack = SYNC_MAGIC_ACK;
        if (send(_tcpSocket, &ack, sizeof(ack), 0) != sizeof(ack)) {
            std::cerr << "Host: Failed to send ACK." << std::endl;
            return false;
        }
        std::cout << "Host: Sent ACK." << std::endl;
    } else {
        std::cout << "Client: Handshake successful!" << std::endl;
    }
    
    return true;
}

bool UdpNetworkManager::SetupUdpConnection() {
    std::cout << (_isServer ? "Host" : "Client") << " UdpNetworkManager: Setting up UDP connection..." << std::endl;

    // KROK 0: SYNCHRONIZACJA (To naprawia problem "kto wszedł pierwszy")
    // Host już nie śpi 1500ms, tylko czeka aktywnie na sygnał od klienta.
    if (!SyncTcpHandshake()) {
        std::cerr << "TCP Sync failed. Aborting UDP setup." << std::endl;
        return false;
    }

    // --- Reszta kodu jest podobna, ale bezpieczniejsza dzięki recv_all z timeoutem ---

    _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpSocket == -1) return false;

    // Bind do losowego portu
    sockaddr_in myAddr{};
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = 0; 
    if (bind(_udpSocket, (sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
        close(_udpSocket); return false;
    }

    socklen_t len = sizeof(myAddr);
    getsockname(_udpSocket, (sockaddr*)&myAddr, &len);
    int myPort = ntohs(myAddr.sin_port);
    std::cout << "My UDP Port: " << myPort << std::endl;

    int remotePort = 0;
    ssize_t bytes;


    if (_isServer) {
        if (send(_tcpSocket, &myPort, sizeof(myPort), 0) != sizeof(myPort)) return false;
        
        if (recv_all(_tcpSocket, &remotePort, sizeof(remotePort), 0) <= 0) return false;
        
        const char* start_msg = "START";
        if (send(_tcpSocket, start_msg, 5, 0) != 5) return false;

    } else {
        if (recv_all(_tcpSocket, &remotePort, sizeof(remotePort), 0) <= 0) return false;
        
        if (send(_tcpSocket, &myPort, sizeof(myPort), 0) != sizeof(myPort)) return false;

        char buffer[6] = {0};
        if (recv_all(_tcpSocket, buffer, 5, 0) <= 0) return false;
        if (strncmp(buffer, "START", 5) != 0) {
             std::cerr << "Client: START confirmation invalid." << std::endl;
             return false;
        }
    }

    std::cout << "Remote UDP Port received: " << remotePort << std::endl;

    sockaddr_in peerAddr{};
    socklen_t peerLen = sizeof(peerAddr);
    getpeername(_tcpSocket, (sockaddr*)&peerAddr, &peerLen);
    
    _remoteAddr = peerAddr;
    _remoteAddr.sin_port = htons(remotePort);

    _connected = true;
    std::cout << "UDP Connection Established!" << std::endl;
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
            buffer[received] = '\0'; // Null-terminate the received data

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