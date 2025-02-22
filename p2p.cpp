
#include <iostream>
#include <string>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define MANDATORY_IP "10.206.5.228"
#define MANDATORY_PORT 6555

using namespace std;

// Server-specific code
struct Peer {
    string ip_port;
    string team_name;
};

unordered_map<string, Peer> peerTable; // Stores unique peer entries based on ip_port
mutex peerTableMutex;

void handleTCPClient(SOCKET clientSocket) {
    char buffer[1024] = {0};

    // Receive the peer's initial registration message
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesReceived] = '\0';
    string receivedData(buffer);

    // Check if the message is a registration message
    size_t firstSpace = receivedData.find(' ');
    if (firstSpace == string::npos) {
        cout << "Invalid registration format received!\n";
        closesocket(clientSocket);
        return;
    }

    string ip_port = receivedData.substr(0, firstSpace);
    size_t secondSpace = receivedData.find(' ', firstSpace + 1);
    if (secondSpace == string::npos) {
        cout << "Invalid registration format received!\n";
        closesocket(clientSocket);
        return;
    }

    string team_name = receivedData.substr(firstSpace + 1, secondSpace - (firstSpace + 1));
    string message = receivedData.substr(secondSpace + 1);

    {
        lock_guard<mutex> lock(peerTableMutex);

        // Check for duplicate entries
        if (peerTable.find(ip_port) == peerTable.end()) {
            peerTable[ip_port] = {ip_port, team_name};
            cout << "New Peer: " << ip_port << " - " << team_name << endl;
        } else {
            cout << "Duplicate peer detected: " << ip_port << " - " << team_name << endl;
        }
    }

    // Handle further messages from the peer
    while (true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            // Peer disconnected
            {
                lock_guard<mutex> lock(peerTableMutex);
                peerTable.erase(ip_port);
                cout << "Peer disconnected: " << ip_port << " - " << team_name << endl;
            }
            closesocket(clientSocket);
            return;
        }

        buffer[bytesReceived] = '\0';
        string message(buffer);

        if (message == "exit") {
            // Peer wants to quit
            {
                lock_guard<mutex> lock(peerTableMutex);
                peerTable.erase(ip_port);
                cout << "Peer quit: " << ip_port << " - " << team_name << endl;
            }
            closesocket(clientSocket);
            return;
        }

        // Handle regular messages (optional)
        cout << "Received message from " << ip_port << ": " << message << endl;
    }
}

void handleUDPRequests(SOCKET udpSocket) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0,
                                    (sockaddr*)&clientAddr, &clientAddrSize);
        if (bytesReceived <= 0) continue;

        buffer[bytesReceived] = '\0';

        if (string(buffer) == "discover") {
            string peerList = "+---------------------+------------+\n";
            peerList += "|      IP:PORT       | Team Name  |\n";
            peerList += "+---------------------+------------+\n";

            {
                lock_guard<mutex> lock(peerTableMutex);
                for (auto &entry : peerTable) {
                    peerList += "| " + entry.second.ip_port + " | " + entry.second.team_name + " |\n";
                }
            }

            peerList += "+---------------------+------------+\n";

            sendto(udpSocket, peerList.c_str(), peerList.length(), 0,
                   (sockaddr*)&clientAddr, clientAddrSize);
        }
    }
}

void startServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Setup TCP Server
    SOCKET tcpServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_addr.s_addr = INADDR_ANY;
    tcpServerAddr.sin_port = htons(8080);

    bind(tcpServerSocket, (sockaddr*)&tcpServerAddr, sizeof(tcpServerAddr));
    listen(tcpServerSocket, 5);

    // Setup UDP Server
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpServerAddr;
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = INADDR_ANY;
    udpServerAddr.sin_port = htons(9090);

    bind(udpSocket, (sockaddr*)&udpServerAddr, sizeof(udpServerAddr));

    cout << "TCP Server listening on port 8080...\n";
    cout << "UDP Server listening on port 9090...\n";

    thread udpThread(handleUDPRequests, udpSocket);
    udpThread.detach();

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(tcpServerSocket, (sockaddr*)&clientAddr, &clientSize);

        cout << "Client connected!\n";
        thread clientThread(handleTCPClient, clientSocket);
        clientThread.detach();
    }

    closesocket(tcpServerSocket);
    closesocket(udpSocket);
    WSACleanup();
}

// Client-specific code
SOCKET client_socket;

void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Winsock initialization failed\n";
        exit(EXIT_FAILURE);
    }
}

void sendUDPDiscovery(int udpPort) {
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        cerr << "Failed to create UDP socket\n";
        return;
    }

    sockaddr_in udpServerAddr;
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = htons(udpPort);
    inet_pton(AF_INET, SERVER_IP, &udpServerAddr.sin_addr);

    string discoveryMessage = "discover";
    sendto(udpSocket, discoveryMessage.c_str(), discoveryMessage.length(), 0,
           (sockaddr*)&udpServerAddr, sizeof(udpServerAddr));

    char buffer[BUFFER_SIZE] = {0};
    sockaddr_in serverAddr;
    int serverAddrSize = sizeof(serverAddr);
    int bytesReceived = recvfrom(udpSocket, buffer, BUFFER_SIZE - 1, 0,
                                (sockaddr*)&serverAddr, &serverAddrSize);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "\n[UDP Peer List]:\n" << buffer << endl;
    }

    closesocket(udpSocket);
}

void handleIncomingConnections(int port) {
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Failed to create listen socket\n";
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        closesocket(listenSocket);
        return;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        closesocket(listenSocket);
        return;
    }

    cout << "Listening for incoming connections on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed\n";
            continue;
        }

        char buffer[BUFFER_SIZE] = {0};
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received message from peer: " << buffer << endl;
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
}

void connectToPeer(const string& peerAddress) {
    SOCKET peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (peerSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket for peer connection\n";
        return;
    }

    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(stoi(peerAddress.substr(peerAddress.find(':') + 1)));
    inet_pton(AF_INET, peerAddress.substr(0, peerAddress.find(':')).c_str(), &peerAddr.sin_addr);

    if (connect(peerSocket, (sockaddr*)&peerAddr, sizeof(peerAddr)) == SOCKET_ERROR) {
        cerr << "Failed to connect to peer: " << peerAddress << "\n";
        closesocket(peerSocket);
        return;
    }

    cout << "Connected to peer: " << peerAddress << "\n";

    string message;
    cout << "Enter message to send: ";
    getline(cin, message);
    send(peerSocket, message.c_str(), message.length(), 0);

    closesocket(peerSocket);
}

void connectToMandatoryPeer() {
    SOCKET mandatorySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mandatorySocket == INVALID_SOCKET) {
        cerr << "Failed to create socket for mandatory connection\n";
        return;
    }

    sockaddr_in mandatoryAddr;
    mandatoryAddr.sin_family = AF_INET;
    mandatoryAddr.sin_port = htons(MANDATORY_PORT);
    inet_pton(AF_INET, MANDATORY_IP, &mandatoryAddr.sin_addr);

    if (connect(mandatorySocket, (sockaddr*)&mandatoryAddr, sizeof(mandatoryAddr)) == SOCKET_ERROR) {
        cerr << "Failed to connect to mandatory peer: " << MANDATORY_IP << ":" << MANDATORY_PORT << "\n";
        closesocket(mandatorySocket);
        return;
    }

    cout << "Connected to mandatory peer: " << MANDATORY_IP << ":" << MANDATORY_PORT << "\n";

    string message = "Hello from client";
    send(mandatorySocket, message.c_str(), message.length(), 0);

    closesocket(mandatorySocket);
}

void receiveMessages(SOCKET socket) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        int bytesReceived = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            cout << "Server disconnected!\n";
            closesocket(socket);
            WSACleanup();
            exit(0);
        }

        buffer[bytesReceived] = '\0';
        cout << "\nReceived: " << buffer << endl;
    }
}

void sendMessages(string my_ip_port, string team_name, int udpPort) {
    // Start a thread to receive messages
    thread receiverThread(receiveMessages, client_socket);
    receiverThread.detach();

    string message;
    while (true) {
        cout << "\n***** Menu *****\n";
        cout << "1. Send message\n";
        cout << "2. Query active peers\n";
        cout << "3. Connect to active peers\n";
        cout << "0. Quit\n";
        cout << "Choose an option: ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) {
            cout << "Closing connection...\n";
            send(client_socket, "exit", 4, 0);
            closesocket(client_socket);
            WSACleanup();
            exit(0);
        }
        else if (choice == 1) {
            cout << "Enter recipient IP:PORT: ";
            string recipient;
            getline(cin, recipient);
            cout << "Enter message: ";
            getline(cin, message);
            string full_message = my_ip_port + " " + team_name + " " + message;
            send(client_socket, full_message.c_str(), full_message.length(), 0);
        }
        else if (choice == 2) {
            sendUDPDiscovery(udpPort); // Query active peers
        }
        else if (choice == 3) {
            cout << "Enter peer address (IP:PORT): ";
            string peerAddress;
            getline(cin, peerAddress);
            connectToPeer(peerAddress); // Connect to a discovered peer
        }
        else {
            cout << "Invalid option! Try again.\n";
        }
    }
}

void startClient() {
    initializeWinsock();

    // Connect to the mandatory peer
    connectToMandatoryPeer();

    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Connection failed\n";
        closesocket(client_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    string my_ip_port, team_name;
    cout << "Enter your name: ";
    getline(cin, team_name);
    cout << "Enter your port number: ";
    getline(cin, my_ip_port);

    // Register with the server
    string registrationMessage = my_ip_port + " " + team_name + " Hello";
    send(client_socket, registrationMessage.c_str(), registrationMessage.length(), 0);

    // Start a thread to listen for incoming connections
    int my_port = stoi(my_ip_port.substr(my_ip_port.find(':') + 1));
    thread listenerThread(handleIncomingConnections, my_port);
    listenerThread.detach();

    sendMessages(my_ip_port, team_name, 9090);
}

int main() {
    cout << "1. Start Server\n";
    cout << "2. Start Client\n";
    cout << "Choose an option: ";
    int choice;
    cin >> choice;
    cin.ignore();

    if (choice == 1) {
        startServer();
    } else if (choice == 2) {
        startClient();
    } else {
        cout << "Invalid option!\n";
    }

    return 0;
}