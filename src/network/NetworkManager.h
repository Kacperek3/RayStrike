#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include <fcntl.h>
#include <unordered_map>
#include <mutex>
#include <sys/time.h>


struct LobbyInfo {
    std::string name;
    std::string ip;
    int port;
    std::string playerName;
    time_t lastSeen;

    std::string getUID() const {
        return ip + ":" + std::to_string(port);
    }
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void startLobbyBroadcast(const std::string& message, int port);
    void stopLobbyBroadcast();

    void startTCPListener(int preferredPort = 0);
    void stopTCPListener();

    int getTCPPort() const;

    void onClientConnected(std::function<void(std::string)> callback);


    void startLobbyDiscovery(int port = 8888);
    void stopLobbyDiscovery();
    std::unordered_map<std::string, LobbyInfo> getDiscoveredLobbies();

    bool connectToServer(const std::string& ip, int port);
    int getClientSocket() const;


private:
    int udpSocket;
    int tcpSocket;
    sockaddr_in broadcastAddr;
    sockaddr_in tcpAddr;

    std::atomic<bool> broadcasting;
    std::atomic<bool> listening;

    std::thread broadcastThread;
    std::thread listenerThread;

    std::function<void(std::string)> clientConnectedCallback;


    int discoverySocket;
    std::atomic<bool> discovering;
    std::thread discoveryThread;
    std::unordered_map<std::string, LobbyInfo> discoveredLobbies;
    std::mutex lobbyMutex;

    int clientSocket = -1;
};
