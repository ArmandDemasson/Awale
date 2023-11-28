#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Game init_game(char ** players){

    int **initial_board = (int **)malloc(2 * sizeof(int *));
    for (int i = 0; i < 2; i++) {
        initial_board[i] = (int *)malloc(6 * sizeof(int));
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            initial_board[i][j] = 4;
        }
    }



    int * initial_scores = malloc(2 * sizeof(int));
    initial_scores[0] = 0;
    initial_scores[1] = 0;


    // Initialisation de la structure Game
    Game game = {
        .board = initial_board,
        .players = players,
        .scores = initial_scores,
        .turn = 0,
        .state = 1
    };

    // ... faites quelque chose avec la structure Game ...
    return game;
    // Libération de la mémoire allouée dynamiquement
}

char * start_turn(Game game){
    
    char * turn_logs = (char *)malloc(1024);

    sprintf(turn_logs,"Les scores actuels sont :\n");
    sprintf(turn_logs + strlen(turn_logs),"Joueur1 - %s : %d scores\n", game.players[0],game.scores[0]);
    sprintf(turn_logs + strlen(turn_logs),"Joueur2 - %s : %d scores\n", game.players[1],game.scores[1]);
    sprintf(turn_logs + strlen(turn_logs),"C'est le tour de %s\n\n",game.players[game.turn]);
    char * printed_board = print_board(game.board);
    strncat(turn_logs,printed_board,1024);

    return turn_logs;
}


char * print_board(int ** board){
    
    char *cadre = (char *)malloc(500);

 sprintf(cadre, "\nCadre de jeu d'Awalé:\n");
    sprintf(cadre + strlen(cadre), "      6      5      4      3      2      1    \n");
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |\n", board[1][5], board[1][4], board[1][3], board[1][2], board[1][1], board[1][0]);
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |  %2d  |\n", board[0][0], board[0][1], board[0][2], board[0][3], board[0][4], board[0][5]);
    sprintf(cadre + strlen(cadre), "  +------+------+------+------+------+------+\n");
    sprintf(cadre + strlen(cadre), "      1      2      3      4      5      6      \n");

    return cadre;
}

int isPossiblePlay(Game game, int selectedHole){

    if(emptyRow(game.board[game.turn][selectedHole-1]) == 0) {
        printf("veuillez sélectionner un puit contenant des graines");
        return 0;
    }

    int TotalOppRowSeed = 0;
    for (int i = 0; i<6; i++) {
		TotalOppRowSeed += game.board[(game.turn + 1) % 2][i];
    }
	
    if(TotalOppRowSeed == 0 && game.board[game.turn][selectedHole-1] < 6 - selectedHole) {
	    printf("vous ne pouvez pas jouer ce puit car votre adversaire est en famine, veuillez nourrir votre adversaire");
        return 0;
    }

    if(selectedHole < 1 || selectedHole > 6) {
        printf("veuillez sélectionner un puit numéroté entre 1 et 6");
        return 0;
    }

    return 1;
}

int willStarve(Game game) {

    int totalRowSeed = 0;
    int willStarve;
    for (int i = 0; i<6; i++) {
		totalRowSeed += game.board[game.turn][i];
    }
	if(totalRowSeed == 0) {
	    int hole = 0;
        willStarve = 1;
        for(int i = 0; i<6; i++) {
            if(game.board[(game.turn + 1) % 2][hole] >= 6-hole) {
                willStarve = 0;
                break;
            }
        }
        if(willStarve == 1) {
            int totalScore=0;
            for(int i = 0; i<6; i++) {
                totalScore += game.board[(game.turn + 1) % 2][i];
                //game.board[(game.turn + 1) % 2][i] = 0;
            }
		    game.scores[(game.turn + 1) % 2] += totalScore;
        }
        return willStarve;
    }

    return 0;

    
} 

int emptyRow(int seeds){
    if (seeds == 0) {
	printf("vous devez sélectionner un puit contenant des graines");
        return 0; // Invalid move, no seeds in the selected house
    }
}

int scoreLimit(Game game){
    if(game.scores[game.turn] >= 25){
        return 0;
    }
    return 1;
}

int harvest(Game game, int selectedHole){
    int seeds = game.board[game.turn][selectedHole-1];
    game.board[game.turn][selectedHole-1] = 0;
    int i;
    int recolte;
    int currentRow, currentCol; 
    for (i = 1; i <= seeds; i++) {
        currentRow = (int)(( game.turn+ (selectedHole -1 + i) / 6 ) % 2);
        currentCol = (selectedHole -1 + i) %6;
        game.board[currentRow][currentCol] = game.board[currentRow][currentCol] + 1;;
	    if(currentRow != game.turn && (game.board[currentRow][currentCol] == 2 || game.board[currentRow][currentCol] == 3)) {
		    recolte = 1;
        }else {
        recolte = 0;
        }
    }

    if(recolte) {
		i = currentCol;
        while((game.board[currentRow][i] == 2 || game.board[currentRow][i] == 3) && i >= 0) {
            game.scores[game.turn] += game.board[currentRow][i];
            game.board[currentRow][i] = 0;
            i--;
        }
    printf("Tant de graine récoltées\n");
    return 1;
	}else {
		printf("Aucune graine capturée\n");
        return 0;
	}
}

int play(Game game, int selectedHole){
    printf("%d\n", selectedHole);
    if(isPossiblePlay(game, selectedHole)) {
        harvest(game, selectedHole);
        if(scoreLimit(game) == 0){
            game.state = 0;
            printf("Partie gagnée au score");
            return 0;
        } 
        if(willStarve(game) == 1) {
            game.state = 0;
            printf("Partie terminée par famine");
            return 0;
        }
        return 1;
    } else {
        return 0;
    }
}
