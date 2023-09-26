
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "3000"

int main()
{
    /*
     *  Initialize Winsock.
        Create a socket.
        Bind the socket.
        Listen on the socket for a client.
        Accept a connection from a client.
        Receive and send data.
        Disconnect.
     *
     */

    WSADATA wsaData;

    int start_result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (start_result != 0)
    {
        // handle error
        WSACleanup();
        return 1;
    }
    // hints is basically all the info about the socket (port and stuffs)
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    start_result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    if (start_result != 0)
    {
        WSACleanup();
        return 1;
    }

    SOCKET listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (listener == INVALID_SOCKET)
    {
        printf("listener failed to start");
        freeaddrinfo(result);
        WSACleanup();
        return 2;
    }

    // binding a socket

    start_result = bind(listener, result->ai_addr, (int)result->ai_addrlen);
    if (start_result == SOCKET_ERROR)
    {
        printf("binding failed");
        freeaddrinfo(result);
        closesocket(listener);
        WSACleanup();
        return 2;
    }
    freeaddrinfo(result);

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("connecting failed!");
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    /*
    There are several different programming techniques using Winsock that
    can be used to listen for multiple client connections.
    One programming technique is to create a continuous loop that checks for connection requests using the listen function
    (see Listening on a Socket).
    If a connection request occurs, the application calls the accept, AcceptEx,
    or WSAAccept function and passes the work to another thread to handle the request.
    Several other programming techniques are possible.
    src: https://learn.microsoft.com/en-us/windows/win32/winsock/accepting-a-connection
    */

    // temporary client socket
    SOCKET clientSocket = INVALID_SOCKET;

    clientSocket = accept(listener, NULL, NULL);

    if (clientSocket == INVALID_SOCKET)
    {
        printf(" Client connection failed");
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    // keep recieving the bytes from client
    char recvbuf[4000];
    int rec_result, sendresult;
    int recvbufflen = 4000;
    while (true)
    {
        rec_result = recv(clientSocket, recvbuf, recvbufflen, 0);
        if (rec_result > 0)
        {
            printf("recieved %d bytes: \n", rec_result);
            printf(recvbuf);
            // success case echoes
            sendresult = send(clientSocket, "HTTP/1.1 200\n\n<!DOCTYPE HTML> HELLO", 34, 0);
            if (sendresult == SOCKET_ERROR)
            {
                printf("sending failed");
                closesocket(listener);
                WSACleanup();
                return 1;
            }
        }
        else if (rec_result == 0)
        {
            // connection close!
            printf("connection closed!");
        }

        else
        {
            // something went wrong in our side while listening (5xx)
            printf("listening failed!");
            closesocket(listener);
            WSACleanup();
            return 1;
        }
    }

    return 0;
}
