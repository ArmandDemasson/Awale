#ifndef GAME_H
#define GAME_H

#include "server.h"

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

typedef struct {
    int ** board;
    Client * players;
    int * scores;
    int turn;
    int state;
} Game;

extern Game init_game(Client * players);
extern char * start_turn(Game game);
int isPossiblePlay(Game game, int selectedHole);
int willStarve(Game game);
char * print_board(int ** board);
int emptyRow(int seeds);
int harvest(Game game, int selectedHole);
extern int play(Game game, int selectedHole);


#endif

#endif
