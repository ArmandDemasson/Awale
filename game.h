#ifndef GAME_H
#define GAME_H


#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

typedef struct {
    int ** board;
    char players[2][1024];
    int * scores;
    int turn;
    int state;
} Game;

extern Game init_game(char players[2][1024]);
extern char * start_turn(Game game);
int isPossiblePlay(Game game, int selectedHole);
int willStarve(Game game);
char * print_board(int ** board, Game game);
int emptyRow(int seeds);
int harvest(Game game, int selectedHole);
extern int play(Game game, int selectedHole);


#endif

#endif
