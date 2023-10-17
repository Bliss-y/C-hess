#include "socket_e.h"
#define GAME_STATUS_NONE 0
#define GAME_STATUS_ONGOING 2
#define GAME_STATUS_FINDING 1
#define GAME_MOVE_INVALID -1
#define GAME_STATUS_WINNER1 3
#define GAME_STATUS_WINNER2 4
#define GAME_STATUS_DRAW 5
#define GAME_STATUS_ABORDTED -1
#define GAME_SIDE_WHITE 0
#define GAME_SIDE_BLACK 1

bool check_checks(int k_position, int turn, char *fen);
struct st_game
{
    int id;
    int socket_len;
    int game_status;
    char current_fen[72];
    char *current_mov;
    int turn;
    fd_set fds;
    TH_ID threadId;
    int w_king_position;
    int b_king_position;
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

int game_fen(st_game *game, char *move, int nsize)
{

    // move
    // castling availability
    // en passant
    // 50-move / half move since last pawn move or capture
    // total number of move
}

int game_validate_move(st_game *game, int size, char *move)
{
    char piece = move[0];
    char action = move[1];   // space for move, x for takes
    int file = move[2] - 97; // 0-8
    int rank = move[3] - 97; // 0-8
    if (file > 7)
    {
        return GAME_MOVE_INVALID;
    }
    int finalfile = move[4] - 97;
    int finalrank = move[5] - 49;
    int moved_position = finalrank * 7 + finalfile;
    char newfen[64];
    int w_king_position = game->w_king_position;
    int b_king_position = game->b_king_position;
    memcpy(&newfen, game->current_fen, 64);
    if (action == 'x' && (game->current_fen[moved_position] > 122 - (game->turn) * 32 || game->current_fen[moved_position] < 90 - (game->turn) * 32))
    {
        return GAME_MOVE_INVALID;
    }
    switch (piece)
    {
    case 'P':
        const char fenlookupchar = 'p' - (game->turn * 32);
        int prev_required_board_position[] = {-1, -1};
        if (game->turn == GAME_SIDE_WHITE)
        {
            if (action == ' ')
            {
                prev_required_board_position[0] = (finalrank - 1) * 8 + finalfile;
                if (finalrank == 3)
                {
                    prev_required_board_position[1] = (finalrank - 16) * 8 + finalfile;
                }
            }
            else if (action == 'x')
            {
                prev_required_board_position[0] = moved_position - 7;
                prev_required_board_position[1] = moved_position - 9;
            }
        }
        else
        {
            if (action == ' ')
            {
                prev_required_board_position[0] = finalrank + 8;
                if (finalrank == 4)
                {
                    prev_required_board_position[1] = (8 * 6) + finalfile;
                }
            }
            if (action == 'x')
            {
                prev_required_board_position[0] = moved_position + 7;
                prev_required_board_position[0] = moved_position + 9;
            }
        }
        int prev_position = -1;
        if (prev_required_board_position[0] % 8 == file)
        {
            prev_position = prev_required_board_position[0];
        }
        else if (prev_required_board_position[1] % 8 == file)
        {
            prev_position = prev_required_board_position[1];
        }
        newfen[prev_position] = ' ';
        newfen[moved_position] = fenlookupchar;
        break;
    case 'R':
        if (finalfile != file && finalrank != finalrank)
        {
            return GAME_MOVE_INVALID;
        }
        bool valid = false;
        int multiplier = 1;
        int increment = 1;
        if (finalfile - file + finalrank - rank < 0)
        {
            multiplier = -1;
        }
        if (finalrank != rank)
        {
            increment = 8;
        }
        for (int i = increment; i != rank * file; i += multiplier * (increment))
        {
            if (newfen[i + moved_position] == ' ')
            {
                continue;
            }
            if (newfen[i + moved_position] == fenlookupchar && i + moved_position == file * rank)
            {
                valid = true;
                break;
            }
            break;
        }
        if (!valid)
        {
            return GAME_MOVE_INVALID;
        }
        newfen[file * rank] = ' ';
        newfen[moved_position] = fenlookupchar;
        break;
    case 'N':
        int knight_delta_vec[] = {6, -6, 15, -15, 10, -10, 17, -17};
        const char fenlookupchar = 'n' - (game->turn * 32);
        if (game->current_fen[file * rank] != 'n' - (game->turn * 32))
            return GAME_MOVE_INVALID;
        bool valid = false;
        for (int i = 0; i < 8; i++)
        {
            int nposition = moved_position + knight_delta_vec[i];
            int n_file = nposition % 8;
            int n_rank = nposition / 8;
            if (n_file > 0 && n_file < 7 && n_rank > 0 && n_rank < 7 && game->current_fen[nposition] == file * rank)
            {
                valid = true;
            }
        }
        if (!valid)
        {
            return GAME_MOVE_INVALID;
        }
        newfen[file * rank] = ' ';
        newfen[moved_position] = fenlookupchar;
        break;
    case 'B':
        const char fenlookupchar = 'b' - (game->turn * 32);
        if (game->current_fen[file * rank] != fenlookupchar)
            return GAME_MOVE_INVALID;
        int delta_multiplier = 1;
        int direction_multiplier = 1;
        int limiter = finalfile;
        if (rank < finalrank)
        {
            direction_multiplier = -1;
        }
        if (file < finalfile)
        {
            delta_multiplier = -1;
            limiter = 7 - finalfile;
        }
        bool valid = false;
        for (int delta = 1; delta <= limiter; delta++)
        {
            if (finalrank + delta > 7)
            {
                break;
            }
            int i = moved_position + direction_multiplier * (8 * delta + delta_multiplier * delta);
            if (game->current_fen[i] == ' ')
            {
                continue;
            }
            if (game->current_fen[i] == 'b' - (!game->turn * 32) && i == finalfile * finalrank)
            {
                valid = true;
                break;
            }
            break;
        }
        if (!valid)
        {
            return GAME_MOVE_INVALID;
        }
        newfen[file * rank] = ' ';
        newfen[moved_position] = fenlookupchar;
        break;
    case 'K':
        const char fenlookupchar = 'k' - (game->turn * 32);
        if (game->current_fen[file * rank] != fenlookupchar)
            return GAME_MOVE_INVALID;
        bool valid = false;
        int king_delta[] = {1, -1, 8, -8, 7, -7, 6, -6};
        for (int i = 0; i < 8; i++)
        {
            int nposition = moved_position + king_delta[i];
            int n_file = nposition % 8;
            int n_rank = nposition / 8;
            if (n_file > 0 && n_file < 7 && n_rank > 0 && n_rank < 7)
            {
                if (game->current_fen[nposition] == fenlookupchar - (!game->turn * 32))
                {
                    valid = true;
                }
            }
        }
        if (!valid)
        {
            return GAME_MOVE_INVALID;
        }
        newfen[file * rank] = ' ';
        newfen[moved_position] = fenlookupchar;

        break;
    case 'Q':
        const char fenlookupchar = 'q' - (game->turn * 32);
        if (game->current_fen[file * rank] != fenlookupchar)
            return GAME_MOVE_INVALID;
        const char fenlookupchar = 'b' - (game->turn * 32);
        if (game->current_fen[file * rank] != fenlookupchar)
            return GAME_MOVE_INVALID;
        int delta_multiplier = 1;
        int direction_multiplier = 1;
        int limiter = finalfile;
        if (rank < finalrank)
        {
            direction_multiplier = -1;
        }
        if (file < finalfile)
        {
            delta_multiplier = -1;
            limiter = 7 - finalfile;
        }
        bool valid = false;
        for (int delta = 1; delta <= limiter; delta++)
        {
            if (finalrank + delta > 7)
            {
                break;
            }
            int i = moved_position + direction_multiplier * (8 * delta + delta_multiplier * delta);
            if (game->current_fen[i] == ' ')
            {
                continue;
            }
            if (game->current_fen[i] == 'b' - (!game->turn * 32) && i == finalfile * finalrank)
            {
                valid = true;
                break;
            }
            break;
        }
        break;
    default:
        return -1;
    }
    int check_position = game->turn == GAME_SIDE_WHITE ? w_king_position : b_king_position;
    check_checks(check_position, game->turn, newfen);
}

bool check_checks(int k_position, int turn, char *fen)
{
    int k_file, k_rank;
    if (turn == GAME_SIDE_WHITE)
    {
        if (k_position < 42)
        {
            if (k_position + 9 == 'p' || k_position + 7 == 'p')
            {
                return true;
            }
        }
    }
    else
    {
        if (k_position < 42)
        {
            if (k_position - 9 == 'P' || k_position - 7 == 'P')
            {
                return true;
            }
        }
    }

    k_file = k_position % 8;
    k_rank = k_position / 8;
    int knight_delta_vec[] = {6, -6, 15, -15, 10, -10, 17, -17};
    for (int i = 0; i < 8; i++)
    {
        int nposition = k_position + knight_delta_vec[i];
        int n_file = nposition % 8;
        int n_rank = nposition / 8;
        if (n_file > 0 && n_file < 7 && n_rank > 0 && n_rank < 7)
        {
            if (fen[nposition] == 'n' - (!turn * 32))
            {
                return true;
            }
        }
    }
    // Rook
    // go up
    for (int i = k_position + 1; i < 64; i += 8)
    {
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'r' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    // go down
    for (int i = k_position - 8; i >= 0; i -= 8)
    {
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'r' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    // go left
    for (int i = k_position - 1; i >= 0; i -= 1)
    {
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'r' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    // go right
    for (int i = k_position + 1; i >= 0; i += 1)
    {
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'r' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    // for bishop
    for (int delta = 1; delta <= k_file; delta++)
    {
        if (k_rank + delta > 7)
        {
            break;
        }
        int i = k_position + 8 * delta - delta;
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'b' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    for (int delta = 1; delta <= 7 - k_file; delta++)
    {
        if (k_rank + delta > 7)
        {
            break;
        }
        int i = k_position + 8 * delta + delta;
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'b' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    for (int delta = 1; delta <= 7 - k_file; delta++)
    {
        if (k_rank - delta < 0)
        {
            break;
        }
        int i = k_position - 8 * delta + delta;
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'b' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    for (int delta = 1; delta <= k_file; delta++)
    {
        if (k_rank + delta > 7)
        {
            break;
        }
        int i = k_position - 8 * delta - delta;
        if (fen[i] == ' ')
        {
            continue;
        }
        if (fen[i] == 'b' - (!turn * 32) || fen[i] == 'q' - (!turn * 32))
        {
            return true;
        }
        break;
    }
    // for king
    int king_delta[] = {1, -1, 8, -8, 7, -7, 6, -6};
    for (int i = 0; i < 8; i++)
    {
        int nposition = k_position + king_delta[i];
        int n_file = nposition % 8;
        int n_rank = nposition / 8;
        if (n_file > 0 && n_file < 7 && n_rank > 0 && n_rank < 7)
        {
            if (fen[nposition] == 'k' - (!turn * 32))
            {
                return true;
            }
        }
    }
    return false;
}