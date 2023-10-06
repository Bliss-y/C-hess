#include "socket_e.h"
struct st_game
{
    int id;
    int socket_len;
    int game_status;
    char *current_fen;
    char *current_mov;
    int turn;
    TH_ID threadId;
    st_socket *sockets;
};

void broadcast(st_game *game, char *buff, int size)
{
    st_socket *socket = game->sockets;
    st_socket *prev = NULL;
    while (socket)
    {
        send(socket->socket, buff, size, 0);
    }
}

void gameLoop(st_game *game)
{
    while (!game->game_status)
    {
        st_socket *socket = game->sockets;
        st_socket *prev = NULL;
        while (socket)
        {
            char recb[2000];
            int res = recv(socket->socket, recb, 2000, 0);
            if (res == 0)
            {
                closesocket(socket->socket);
                if (prev)
                {
                    prev->next = socket->next;
                    closesocket(socket->socket);
                }
                else
                {
                    game->sockets = socket->next;
                    closesocket(socket->socket);
                }
                if (socket->player)
                {
                    free(socket);
                    game->game_status = -1;
                    break;
                }
                free(socket);
            }
        }
    }
    while (true && game->game_status == 1)
    {
        st_socket *socket = game->sockets;
        st_socket *prev = NULL;
        while (socket)
        {
            char recb[2000];
            int res = recv(socket->socket, recb, 2000, 0);
            if (res == 0)
            {
                if (prev)
                {
                    prev->next = socket->next;
                    closesocket(socket->socket);
                }
                else
                {
                    game->sockets = socket->next;
                    closesocket(socket->socket);
                }
                if (socket->player)
                {
                    break;
                }
                free(socket);
            }
            if (socket->player)
            {
                if (res > 0)
                {
                    if (game->turn == socket->side)
                    {
                        broadcast(game, recb, res);
                        game->turn = !((bool)game->turn);
                    }
                }
            }
            prev = socket;
            socket = socket->next;
        }
    }
    if (game->game_status > 1)
    {
        // broadcast the winner!;
    }
    st_socket *socket = game->sockets;
    st_socket *prev = NULL;
    while (socket)
    {
        if (prev)
        {
            prev->next = socket->next;
            closesocket(socket->socket);
        }
        else
        {
            game->sockets = socket->next;
            closesocket(socket->socket);
        }
        free(socket);
        socket = socket->next;
    }
    game->game_status = 0;
    game->socket_len = 0;
    game->sockets = NULL;
    if (game->current_fen)
    {
        free(game->current_fen);
    }
    if (game->current_mov)
    {
        free(game->current_mov);
    }
    return;
}
