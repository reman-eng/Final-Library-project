#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#endif


#define PORT 8080
#define BUFFER_SIZE 4096


int main()
{
#ifdef _WIN32

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Socket setup failed!\n");
        return 1;
    }

#endif


    int serverSocket;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        printf("Socket creation failed!\n");
        return 1;
    }


    struct sockaddr_in serverAddress;

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);


    if (bind(
        serverSocket,
        (struct sockaddr *)&serverAddress,
        sizeof(serverAddress)) < 0)
    {
        printf("Bind failed!\n");
        return 1;
    }


    if (listen(serverSocket, 10) < 0)
    {
        printf("Listen failed!\n");
        return 1;
    }


    printf("\n");
    printf("=================================\n");
    printf("     LIBRARY HTTP SERVER\n");
    printf("=================================\n");
    printf("Server running on port %d\n", PORT);
    printf("Open: http://localhost:8080\n");
    printf("=================================\n\n");


    while (1)
    {
        struct sockaddr_in clientAddress;

#ifdef _WIN32
        int clientSize = sizeof(clientAddress);
#else
        socklen_t clientSize = sizeof(clientAddress);
#endif


        int clientSocket;

        clientSocket = accept(
            serverSocket,
            (struct sockaddr *)&clientAddress,
            &clientSize
        );


        if (clientSocket < 0)
        {
            printf("Connection failed!\n");
            continue;
        }


        char request[BUFFER_SIZE];

        memset(request, 0, sizeof(request));


        int receivedBytes;

        receivedBytes = recv(
            clientSocket,
            request,
            BUFFER_SIZE - 1,
            0
        );


        if (receivedBytes > 0)
        {
            request[receivedBytes] = '\0';

            printf("Browser request:\n");
            printf("%s\n", request);
        }


        FILE *htmlFile;

        htmlFile = fopen("Central Library Desk.html", "rb");


        if (htmlFile == NULL)
        {
            printf("HTML file not found!\n");

            const char *errorPage =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<h1>404 - HTML file not found</h1>";

            send(
                clientSocket,
                errorPage,
                strlen(errorPage),
                0
            );

#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif

            continue;
        }


        fseek(htmlFile, 0, SEEK_END);

        long fileSize;

        fileSize = ftell(htmlFile);

        rewind(htmlFile);


        char *htmlData;

        htmlData = malloc(fileSize + 1);


        if (htmlData == NULL)
        {
            printf("Memory allocation failed!\n");
            fclose(htmlFile);

#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif

            continue;
        }


        fread(htmlData, 1, fileSize, htmlFile);

        htmlData[fileSize] = '\0';

        fclose(htmlFile);


        char responseHeader[BUFFER_SIZE];

        int headerSize;

        headerSize = snprintf(
            responseHeader,
            sizeof(responseHeader),

            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n",

            fileSize
        );


        send(
            clientSocket,
            responseHeader,
            headerSize,
            0
        );


        send(
            clientSocket,
            htmlData,
            fileSize,
            0
        );


        printf("Library page sent successfully.\n\n");


        free(htmlData);


#ifdef _WIN32

        closesocket(clientSocket);

#else

        close(clientSocket);

#endif
    }


#ifdef _WIN32

    closesocket(serverSocket);
    WSACleanup();

#else

    close(serverSocket);

#endif


    return 0;
}