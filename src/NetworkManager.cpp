#include "NetworkManager.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

NetworkManager::NetworkManager() : udpSocket(-1), tcpSocket(-1), broadcasting(false), listening(false) {}

NetworkManager::~NetworkManager() {
    stopLobbyBroadcast();
    stopTCPListener();
}

void NetworkManager::startLobbyBroadcast(const std::string& message, int port) {
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    int enable = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    broadcasting = true;
    broadcastThread = std::thread([=]() {
        while (broadcasting) {
            sendto(udpSocket, message.c_str(), message.size(), 0,
                   (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void NetworkManager::stopLobbyBroadcast() {
    broadcasting = false;
    if (broadcastThread.joinable()) broadcastThread.join();
    if (udpSocket != -1) close(udpSocket);
}

void NetworkManager::startTCPListener(int preferredPort) {
   tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(preferredPort);

    if (bind(tcpSocket, (sockaddr*)&tcpAddr, sizeof(tcpAddr)) < 0) {
        std::cerr << "Failed to bind TCP socket\n";
        return;
    }

    listen(tcpSocket, SOMAXCONN);

    int flags = fcntl(tcpSocket, F_GETFL, 0);
    fcntl(tcpSocket, F_SETFL, flags | O_NONBLOCK);

    listening = true;

    listenerThread = std::thread([=]() {
        while (listening) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int client = accept(tcpSocket, (sockaddr*)&clientAddr, &clientLen);
            if (client >= 0) {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
                if (clientConnectedCallback) clientConnectedCallback(ip);
                close(client);
            } else {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept error");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    });
}

void NetworkManager::stopTCPListener() {
    listening = false;

    if (listenerThread.joinable() && std::this_thread::get_id() != listenerThread.get_id()) {
        try {
            listenerThread.join();
        } catch (const std::system_error& e) {
            std::cerr << "Błąd przy joinowaniu wątku: " << e.what() << std::endl;
        }
    }

    if (tcpSocket != -1) {
        close(tcpSocket);
        tcpSocket = -1;
    }

}

int NetworkManager::getTCPPort() const {
    socklen_t len = sizeof(tcpAddr);
    getsockname(tcpSocket, (sockaddr*)&tcpAddr, &len);
    return ntohs(tcpAddr.sin_port);
}

void NetworkManager::onClientConnected(std::function<void(std::string)> callback) {
    clientConnectedCallback = callback;
}


void NetworkManager::startLobbyDiscovery(int port) {
    discoverySocket = socket(AF_INET, SOCK_DGRAM, 0);
    int enable = 1;
    setsockopt(discoverySocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(discoverySocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind error in discovery\n";
        return;
    }

    discovering = true;
    discoveryThread = std::thread([=]() {
        char buffer[1024];
        timeval timeout{0, 100000};
        setsockopt(discoverySocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        while(discovering) {
            sockaddr_in from;
            socklen_t fromLen = sizeof(from);
            int len = recvfrom(discoverySocket, buffer, sizeof(buffer)-1, 0, (sockaddr*)&from, &fromLen);
            
            if(len > 0) {
                buffer[len] = '\0';
                std::string message(buffer);
                size_t first_sep = message.find('|');

                if (first_sep != std::string::npos) {
                    std::string lobbyName = message.substr(0, first_sep);
                    size_t second_sep = message.find('|', first_sep + 1);

                    if (second_sep != std::string::npos) {
                        std::string port_str = message.substr(first_sep + 1, second_sep - (first_sep + 1));
                        int tcpPort = std::stoi(port_str);
                        std::string playerName = message.substr(second_sep + 1);

                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &from.sin_addr, ip, INET_ADDRSTRLEN);

                        LobbyInfo newLobby{
                            lobbyName,
                            ip,
                            tcpPort,
                            playerName, 
                            time(nullptr)
                        };

                        std::lock_guard<std::mutex> lock(lobbyMutex);
                        discoveredLobbies[newLobby.getUID()] = newLobby;
                    } else {
                        std::cerr << "Invalid message format: " << message << std::endl;
                    }
                }
            }
            
            // Czyszczenie starych wpisów (CO 1 SEKUNDĘ)
            auto now = time(nullptr);
            std::lock_guard<std::mutex> lock(lobbyMutex);
            for(auto it = discoveredLobbies.begin(); it != discoveredLobbies.end();) {
                if(now - it->second.lastSeen > 5) {
                    it = discoveredLobbies.erase(it);
                } else {
                    ++it;
                }
            }
        }
        close(discoverySocket);
    });
}

void NetworkManager::stopLobbyDiscovery() {
    discovering = false;
    if(discoveryThread.joinable()) discoveryThread.join();
}

std::unordered_map<std::string, LobbyInfo> NetworkManager::getDiscoveredLobbies() {
    std::lock_guard<std::mutex> lock(lobbyMutex);
    return discoveredLobbies;
}

bool NetworkManager::connectToServer(const std::string& ip, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket");
        return false;
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(clientSocket);
        return false;
    }
    
    if(connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        close(clientSocket);
        return false;
    }
    
    return true;
}

int NetworkManager::getClientSocket() const {
    return clientSocket;
}