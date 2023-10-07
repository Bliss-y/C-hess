#include "socket_e.h"
#include "Windows.h"
#include "game.cpp"
#include "./utils/base64.cpp"
#define MAX_CONNECTION_POOL_SIZE 10

const char *MAGIC_KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

st_game connection_pool[MAX_CONNECTION_POOL_SIZE];
int connection_pool_size = MAX_CONNECTION_POOL_SIZE;
int connection_pool_filled = 0;

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    st_game *game = (st_game *)lpParam;
    gameLoop(game);
    return 0; // Thread exits
}

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
    printf("binded");

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("connecting failed!");
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    while (true)
    {

        // temporary client socket/
        SOCKET clientSocket = INVALID_SOCKET;

        clientSocket = accept(listener, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            printf(" Client connection failed");
            closesocket(clientSocket);
            continue;
        }
        handleClient(clientSocket);
    }
    return 0;
}

void setfd(fd_set *fds, SOCKET socket)
{
    FD_SET(socket, fds);
}

void zerofds(fd_set *fds)
{
    FD_ZERO(fds);
}

int socket_send_ws(SOCKET socket, char *msg, int msglen)
{
    char sendbuff[1000];
    sprintf(sendbuff, "%c%c%s", (char)0x81, (char)msglen, msg);
    send(socket, sendbuff, strlen(sendbuff), 0);
    return 0;
}

// REMOVE FDS IF NOT NEEDED.
int socket_parse_ws(SOCKET socket, char *reciever, fd_set *fds)
{
    char recb[2000];
    int res = recv(socket, recb, 2000, 0);
    int fin = 0;
    int opcode = 0;
    if (res == 0)
    {
        // connection closed!
        return 0;
    }
    else if (res == SOCKET_ERR && SOCKET_NO_DATA)
    {
        // no data!
        return SOCKET_ERR;
    }
    else if (res == SOCKET_ERR)
    {
        // ERR reading data from that place!
        return SOCKET_ERR;
    }
    fin = (recb[0] >> 7) & 1;
    opcode = recb[0] & 0x0F;
    int mask = (recb[1] >> 7) & 1;
    if (mask == 0)
    {
        return SOCKET_ERR;
    }
    uint64_t payload_length = recb[1] & 0x7F; // last 7 bits
    int current = 2;
    if (payload_length == 126)
    {
        // read next 16 bytes for payload lengthh
        payload_length = (uint16_t)recb[2];
        current = 4;
    }
    else if (payload_length == 127)
    {
        // read next 64 bytes for payload length
        payload_length = (uint64_t)recb[2];
        current = 10;
    }
    printf("payload_length: %lld\n", payload_length);
    char masking_key[] = {recb[current + 0], recb[current + 1], recb[current + 2], recb[current + 3]};
    current += 4;
    for (int i = 0; i < payload_length; i++)
    {
        reciever[i] = (char)(recb[i + current] ^ masking_key[i % 4]);
        printf("\n");
        printf("%c", reciever[i]);
        printf("\n");
    }
    reciever[payload_length] = '\0';
    return payload_length;
}

void handleClient(SOCKET clientSocket)
{
    // keep recieving the bytes from client
    char recvbuf[4000];
    int rec_result, sendresult;
    int recvbufflen = 4000;
    bool done = false;
    do
    {
        rec_result = recv(clientSocket, recvbuf, recvbufflen, 0);
        if (rec_result > 0)
        {
            st_html filler = {};
            int res = chess_parse_html(recvbuf, rec_result, &filler);
            if (res)
            {
                printf("error parsing the html, might be bad request");
                closesocket(clientSocket);
                return;
            }
            if (!strcmp(filler.path, "/play"))
            {
                st_ch_hashelement *connection_header = ht_get_header(&filler, "Connection", 11);
                if (!connection_header && strcmp(connection_header->value, "Upgrade"))
                {
                    printf("Connection header was not Upgrade");
                    closesocket(clientSocket);
                    return;
                }
                connection_header = ht_get_header(&filler, "Upgrade", 8);
                if (!connection_header && strcmp(connection_header->value, "WebSocket"))
                {
                    closesocket(clientSocket);
                    return;
                }
                connection_header = ht_get_header(&filler, "X-request", 10);
                if (true || !strcmp(connection_header->value, "find"))
                {
                    u_long nonBlockingCommand = 1;
                    if (ioctlsocket(clientSocket, FIONBIO, &nonBlockingCommand) != 0)
                    {
                        closesocket(clientSocket);
                        return;
                    }
                    for (int i = 0; i <= MAX_CONNECTION_POOL_SIZE; i++)
                    {
                        if (connection_pool[i].game_status == GAME_STATUS_FINDING)
                        {
                            st_socket *sc = (st_socket *)malloc(sizeof(st_socket));
                            sc->game_id = i;
                            sc->player = true;
                            sc->side = GAME_SIDE_BLACK;
                            sc->socket = clientSocket;
                            game_add_socket(&connection_pool[i], sc);
                            connection_pool[i].socket_len++;
                            connection_header = ht_get_header(&filler, "Sec-WebSocket-Key", 18);
                            if (!connection_header)
                            {
                                closesocket(clientSocket);
                                return;
                            }
                            // make a successful connection
                            char result[41];
                            SHA1(result, connection_header->value, connection_header->vlen);
                            char responseheader[1024];
                            connection_pool[i].game_status = GAME_STATUS_ONGOING;
                            snprintf(responseheader, 1024, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", result);
                            send(clientSocket, responseheader, strlen(responseheader), 0);
                            return;
                        }
                    }

                    if (connection_pool_size > connection_pool_filled)
                    {
                        // spawn a thread lil bro
                        for (int i = 0; i < MAX_CONNECTION_POOL_SIZE; i++)
                        {
                            if (connection_pool[i].game_status == GAME_STATUS_NONE)
                            {
                                connection_pool[i].game_status = GAME_STATUS_FINDING;
                                st_socket *sc = (st_socket *)malloc(sizeof(st_socket));
                                FD_SET(clientSocket, &connection_pool[i].fds);
                                sc->game_id = i;
                                sc->player = true;
                                sc->side = 0;
                                sc->socket = clientSocket;
                                st_socket *head = connection_pool[i].sockets;
                                connection_pool[i].sockets = sc;
                                connection_pool[i].socket_len = 1;
                                connection_header = ht_get_header(&filler, "Sec-WebSocket-Key", 18);
                                if (!connection_header)
                                {
                                    closesocket(clientSocket);
                                    return;
                                }
                                // make a successful connection
                                char result[21];
                                const int len = connection_header->vlen - 1 + strlen(MAGIC_KEY);
                                char *v = (char *)malloc(len);
                                v[0] = '\0';
                                strcat(v, connection_header->value);
                                strcat(v, MAGIC_KEY);
                                SHA1(result, v, len);
                                free(v);
                                char hexresult[41];
                                char responseheader[1024];
                                base64_encode((uint8_t *)result, 20, hexresult);
                                snprintf(responseheader, 1024, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", hexresult);
                                send(clientSocket, responseheader, strlen(responseheader), 0);
                                HANDLE hThread;   // Thread handle
                                DWORD dwThreadId; // Thread ID
                                hThread = CreateThread(NULL, 0, ThreadFunction, &connection_pool[i], 0, &dwThreadId);
                                connection_pool[i].threadId = hThread;
                                return;
                            }
                        }
                    }
                    else
                    {
                        closesocket(clientSocket);
                        return;
                    }
                }
                // Use one of the connectionpools
                return;
            }
            sendresult = send(clientSocket, "HTTP/1.1 200\r\n\r\n<!DOCTYPE HTML> HELLO", 38, 0);
            done = true;
            if (sendresult == SOCKET_ERROR)
            {
                printf("sending failed");
                closesocket(clientSocket);
                return;
            }
        }
        else if (rec_result == 0)
        {
            printf("connection closed!");
            closesocket(clientSocket);
        }
        else
        {
            // something went wrong in our side while listening (5xx)
            printf("listening failed!");
            closesocket(clientSocket);
            return;
        }
    } while (rec_result > 0 && !done);
    closesocket(clientSocket);
}
