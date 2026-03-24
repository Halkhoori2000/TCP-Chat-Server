// Client

// Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Constants
#define PORT 1026

// Validation Method
void validate(int code, char *errorMessage);

// Main Method
int main()
{
    int inputFD = 0;
    char *sendMessage = calloc(1024, sizeof(char));
    char *recieveMessage = calloc(1024, sizeof(char));

    // Initialize server address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_aton("10.0.2.15", &(serverAddress.sin_addr));
    memset(&(serverAddress.sin_zero), '0', 8);

    // Create client socket end point
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Validate socket was created and attempt to connect to server
    validate(clientSocket, "Unable to create socket");
    validate(connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(struct sockaddr)), "Unable to connect to server");


    // Initialize file descriptor set
    fd_set allFDS;

    // Clear file descriptors and add server socket
    FD_ZERO(&allFDS);
    FD_SET(inputFD, &allFDS);
    FD_SET(clientSocket, &allFDS);
    int maxFD = clientSocket;

    while(1)
    {
        // Clear file descriptors and add server socket
        FD_ZERO(&allFDS);
        FD_SET(inputFD, &allFDS);
        FD_SET(clientSocket, &allFDS);
        maxFD = clientSocket;

        // Wait for server to send message or for user to input message
        select (maxFD + 1, &allFDS, NULL, NULL, NULL);

        // User is sending a message
        if (FD_ISSET(inputFD, &allFDS))
        {
            // Reset message memory
            memset(sendMessage, '\0', 1024);

            // Send user input
            fgets(sendMessage, 1024, stdin);
            send(clientSocket, sendMessage, strlen(sendMessage), 0);

            // Reset message memory
            memset(sendMessage, '\0', 1024);
        }
        // User is receiving message from server
        else if (FD_ISSET(clientSocket, &allFDS))
        {
            // Receive message and output on terminal
            recv(clientSocket, recieveMessage, 1024, 0);

            // Check for exit condition
            if (strncmp("exit", recieveMessage, 4) == 0)
            {
                exit(1);
            }

            printf("%s\n", recieveMessage);

            // Reset message memory
            memset(recieveMessage, '\0', 1024);
        }
    }

    // Close sockets
    close(clientSocket);
    close(inputFD);

    // Exit code
    return 0;
}

// Validate function
void validate(int code, char *errorMessage)
{
    if (code < 0)
    {
        perror(errorMessage);
        exit(1);
    }
}
