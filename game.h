#ifndef GAME_H
#define GAME_H

#include "server.h"

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

typedef struct {
    int * board;
    Client * players;
    int * points;
    int turn;
    int state;
} Game;

extern Game init_game(Client * players);
extern char * start_turn(Game game);
char * print_board(int * board);

#endif

#endif
