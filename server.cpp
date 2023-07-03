#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

std::vector<int> clientSockets;

void HandleClient(int clientSocket)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0)
        {
            // Client disconnected
            break;
        }

        std::string message = buffer;
        std::cout << "Received message: " << message << std::endl;

        // Forward the message to other clients
        for (int socket : clientSockets)
        {
            if (socket != clientSocket)
            {
                send(socket, buffer, strlen(buffer), 0);
            }
        }
    }

    // Remove client socket from the list
    auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
    if (it != clientSockets.end())
    {
        clientSockets.erase(it);
    }

    // Close the client socket
    close(clientSocket);
}

int main()
{
    // Create a socket for the server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    // Set up the server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Bind the socket to the server address
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        std::cerr << "Failed to bind socket to address" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0)
    {
        std::cerr << "Failed to listen for connections" << std::endl;
        return -1;
    }

    std::cout << "Server started. Listening for connections..." << std::endl;

    while (true)
    {
        // Accept a new client connection
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress),
                                  &clientAddressLength);
        if (clientSocket < 0)
        {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Add the client socket to the list
        clientSockets.push_back(clientSocket);

        // Start a new thread to handle the client
        std::thread clientThread(HandleClient, clientSocket);
        clientThread.detach();

        std::cout << "New client connected. Client" << std::endl;
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
