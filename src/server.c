// Server

// Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// Constants
#define PORT 1026
#define MAX_USERS 20

// Main Method
int main()
{
    int fileDesc[20];
    char *message = calloc(1024, sizeof(char));
    char *temp = calloc(2048, sizeof(char));
    int unique = 0;
    int counter = 19;
    int signal = 0;

    // Allocate memory for usernames
    char *userNames[20];
    for (int i = 0; i < MAX_USERS; i++)
    {
        userNames[i] = calloc(1024, sizeof(char));
    }

    // Create server socket endpoint
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int clientFD = 0;

    // Initialize client address
    struct sockaddr_in clientAddress;

    // Initialize server address
    struct sockaddr_in serverAddress;
    memset(&(serverAddress), '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = inet_addr("10.0.2.15");

    // Bind server socket endpoint
    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

    // Open socket up to listen for clients
    listen(serverSocket, 5);
    printf("Listening...\n");
    int cliAddrLen = sizeof(clientAddress);

    // Initialize file descriptor set
    fd_set allFDS;
    fd_set senderFD;

    // Clear file descriptors and add server socket
    int fdFlag = 0;
    FD_ZERO(&allFDS);
    FD_SET(serverSocket, &allFDS);
    int maxFD = serverSocket;

    // Wait to receive signals from clients
    while(1)
    {
        // Clear previous sender file descriptor set
        FD_ZERO(&senderFD);
        senderFD = allFDS;

        // Wait for signal to be sent by client
        select(maxFD +1, &senderFD, NULL, NULL, NULL);

        // Case 1: Client attempts to connect to server
        if (FD_ISSET(serverSocket, &senderFD))
        {
            // Accept client
            clientFD = accept(serverSocket, (struct sockaddr *) &clientAddress, &cliAddrLen);

            // Add sender to file descriptor set
            fdFlag++;
            FD_SET(clientFD, &allFDS);
            fileDesc[fdFlag - 1] = clientFD;

            // Prompt user to input username
            char *promptName = calloc(1024, sizeof(char));
            strcpy(promptName, "Enter username: ");
            send(clientFD, promptName, strlen(promptName), 0);

            // Clear the memory and receive the input
            memset(promptName, '\0', 1024);
            recv(clientFD, promptName, 1024, 0);

            // Validate that username has not already been added
            while(counter >= 0 && strcmp(userNames[counter], promptName) != 0)
            {
                counter--;
            }

            // Username is unique
            if (counter == -1)
            {
                unique = 1;
            }
            // Username has already been added
            else
            {
                while(unique == 0)
                {
                    strcpy(promptName, "Username already exists. Enter a new username:");
                    send(clientFD, promptName, strlen(promptName), 0);

                    // Clear memory and receive input
                    memset(promptName, '\0', 1024);
                    recv(clientFD, promptName, 1024, 0);

                    // Reset counter and try again
                    counter = 19;
                    while(counter >= 0 && strcmp(userNames[counter], promptName) != 0)
                    {
                        counter--;
                    }

                    // Username is unique
                    if (counter == -1)
                    {
                        unique = 1;
                    }
                }
            }

            // Print new user to server terminal
            printf("New user added: %s\n", promptName);
            strcpy(userNames[fdFlag -1], promptName);

            // Reset unique and counter
            unique = 0;
            counter = 19;

            // Update max file descriptor
            if (clientFD > maxFD)
            {
                maxFD = clientFD;
            }
        }
        // Case 2: User sends message
        else
        {
            for (int j = 0; j < fdFlag; j++)
            {
                clientFD = fileDesc[j];

                // Client sent signal
                if (FD_ISSET(clientFD, &senderFD))
                {
                    memset(message, '\0', 1024);
                    signal = recv(clientFD, message, 1024, 0);

                    printf("%s sent message: %s\n", userNames[j], message);

                    // Disconnect client
                    if (strncmp("exit", message, 4) == 0)
                    {
                        FD_CLR(clientFD, &allFDS);
                        printf("%s disconnected\n", userNames[j]);
                        send(clientFD, "exit", 4, 0);

                        for (int x = j; j < fdFlag - 1; x++)
                        {
                            fileDesc[x] = fileDesc[x + 1];
                        }
                        fdFlag--;

                        if (clientFD == maxFD && fdFlag != 0)
                        {
                            for (int y = 1; y < fdFlag; y++)
                            {
                                if (fileDesc[y] > maxFD)
                                {
                                    maxFD = fileDesc[y];
                                }
                            }
                        }

                        memset(message, '\0', 1024);
                        close(clientFD);
                    }
                    else
                    {
                        // Send message to all other clients excluding sender
                        for (int k = 0; k < fdFlag; k++)
                        {
                            if (fileDesc[k] != clientFD)
                            {
                                strcat(temp, userNames[j]);
                                strcat(temp, "Message : ");
                                strcat(temp, message);

                                // Send message
                                send(fileDesc[k], temp, strlen(temp), 0);
                                memset(temp, '\0', 2048);
                            }
                        }

                        printf("Message Delivered\n");
                    }
                }
            }
        }
    }

    close(clientFD);

    // Exit code
    return 0;
}
