#ifndef GAME_H
#define GAME_H


#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#define MAX_SPECTATE 100

typedef struct {
    int ** board;
    char ** players;
    int * scores;
    int turn;
    int state;
    char ** spectators;
    int nbSpectate;
} Game;

extern Game init_game(char  ** players, char ** spectators);
extern char * start_turn(Game game);
int isPossiblePlay(Game game, int selectedHole);
int willStarve(Game game);
char * print_board(int ** board, Game game);
int emptyRow(int seeds);
int harvest(Game game, int selectedHole);
void addSpectate(Game game, char* spectator);
extern int play(Game game, int selectedHole);


#endif

#endif
