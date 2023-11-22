#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Game init_game(Client * players){
    int *initial_board = malloc(12 * sizeof(int));

    // Initialisation du tableau board
    for (int i = 0; i < 12; i++) {
        initial_board[i] = 4;
    }

    int * initial_points = malloc(2 * sizeof(int));
    initial_points[0] = 0;
    initial_points[1] = 0;


    // Initialisation de la structure Game
    Game game = {
        .board = initial_board,
        .players = players,
        .points = initial_points,
        .turn = 0,
        .state = 1
    };

    // ... faites quelque chose avec la structure Game ...
    return game;
    // Libération de la mémoire allouée dynamiquement
    // free(initial_board);
}

char * start_turn(Game game){
    
    char * turn_logs = (char *)malloc(1024);

    sprintf(turn_logs,"Les scores actuels sont :\n");
    sprintf(turn_logs + strlen(turn_logs),"Joueur1 - %s : %d points\n", game.players[0].name,game.points[0]);
    sprintf(turn_logs + strlen(turn_logs),"Joueur2 - %s : %d points\n", game.players[1].name,game.points[1]);
    sprintf(turn_logs + strlen(turn_logs),"C'est le tour de %s\n\n",game.players[game.turn].name);
    char * printed_board = print_board(game.board);
    strncat(turn_logs,printed_board,1024);

    return turn_logs;

}

char * print_board(int * board){
    
    char *cadre = (char *)malloc(500);

 sprintf(cadre, "\nCadre de jeu d'Awalé:\n");
    sprintf(cadre + strlen(cadre), "     12     11     10      9      8      7    \n");
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |\n", board[11], board[10], board[9], board[8], board[7], board[6]);
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |\n", board[0], board[1], board[2], board[3], board[4], board[5]);
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "      1      2      3      4      5      6      \n");

    return cadre;
}
