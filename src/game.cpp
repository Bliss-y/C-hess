#include "socket_e.h"
#define GAME_STATUS_NONE 0
#define GAME_STATUS_ONGOING 2
#define GAME_STATUS_FINDING 1
#define GAME_STATUS_WINNER1 3
#define GAME_STATUS_WINNER2 4
#define GAME_STATUS_DRAW 5
#define GAME_STATUS_ABORDTED -1
#define GAME_SIDE_WHITE 0
#define GAME_SIDE_BLACK 1

struct st_game
{
    int id;
    int socket_len;
    int game_status;
    char *current_fen;
    char *current_mov;
    int turn;
    fd_set fds;
    TH_ID threadId;
    st_socket *sockets;
};

void broadcast(st_game *game, char *buff, int size)
{
    st_socket *socket = game->sockets;
    st_socket *prev = NULL;
    while (socket)
    {
        socket_send_ws(socket->socket, buff, size);
        socket = socket->next;
    }
}

void game_add_socket(st_game *game, st_socket *socket)
{
    if (!game->sockets)
    {
        game->sockets = socket;
        return;
    }
    st_socket *head = game->sockets;
    while (head->next)
    {
        head = head->next;
    }
    head->next = socket;
    return;
}

void gameLoop(st_game *game)
{
    printf("game_satus: %d\n", game->game_status);
    while (game->game_status == GAME_STATUS_FINDING)
    {
        st_socket *socket = game->sockets;
        st_socket *prev = NULL;
        while (socket)
        {
            char recb[2000];
            int res = socket_parse_ws(socket->socket, recb, &game->fds);
            if (res == 0)
            {
                printf("closed from client side!");
                closesocket(socket->socket);
                if (prev)
                {
                    prev->next = socket->next;
                }
                else
                {
                    game->sockets = socket->next;
                }
                if (socket->player)
                {
                    free(socket);
                    game->game_status = GAME_STATUS_ABORDTED;
                    break;
                }
                free(socket);
                socket = prev->next;
                continue;
            }
            else if (res == SOCKET_ERR && SOCKET_NO_DATA)
            {
                // no data!
            }
            else if (res == SOCKET_ERR)
            {
                // ERR reading data from that place!
                if (socket->player)
                {
                    game->game_status = GAME_STATUS_ABORDTED;
                    break;
                }
                else
                {
                    closesocket(socket->socket);
                    if (prev)
                    {
                        prev->next = socket->next;
                    }
                    else
                    {
                        game->sockets = socket->next;
                    }
                    socket = prev->next;
                    continue;
                }
            }
            else
            {
                printf("recbprinting!\n");
                printf(recb);
                printf("recbprinting end!\n");
                printf("\n");
                socket_send_ws(socket->socket, recb, res);
            }
            socket = socket->next;
        }
    }
    while (true && game->game_status == GAME_STATUS_ONGOING)
    {
        st_socket *socket = game->sockets;
        st_socket *prev = NULL;
        while (socket)
        {
            char recb[2000];
            int res = socket_parse_ws(socket->socket, recb, &game->fds);
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
                    free(socket);
                    game->game_status = GAME_STATUS_WINNER1;
                    printf("socket disconnected!\n");
                    break;
                }
                free(socket);
            }
            else if (res == SOCKET_ERR && SOCKET_NO_DATA)
            {
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
                if (res == 0)
                {
                    game->game_status = GAME_STATUS_WINNER2;
                }
            }
            prev = socket;
            socket = socket->next;
        }
    }
    printf("Game ended!");
    if (game->game_status > GAME_STATUS_ONGOING)
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
