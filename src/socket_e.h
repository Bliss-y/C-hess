#pragma once
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#include "utils/ch_hash.cpp"
#include "utils/parsehttp.cpp"
#include "utils/sha1.cpp"
#include "utils/url.cpp"
#define DEFAULT_PORT "3000"
#define TH_ID HANDLE
#define SOCKET_NO_DATA WSAGetLastError() == WSAEWOULDBLOCK
#define SOCKET_ERR SOCKET_ERROR
struct st_socket
{
    int game_id;
    int thread_id;
    SOCKET socket;
    int side;
    st_socket *next;
    boolean locked;
    int player = false;
};

void handleClient(SOCKET clientSocket);
int socket_parse_ws(SOCKET socket, char *reciever, fd_set *fds);
void zerofds(fd_set *fds);
void setfd(fd_set *fds, SOCKET socket);