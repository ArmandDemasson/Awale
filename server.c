#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "server.h"
#include "game.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if (err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void server_app(void)
{
   SOCKET sock = init_server_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];
   Game games[MAX_GAMES];
   fd_set rdfs;

   Server server;
   server.max_clients = MAX_CLIENTS;
   server.actual_clients = 0;
   server.actual_games = 0;
   server.max_games = MAX_GAMES;

   while (1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for (i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if (FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if (FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = {0};
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, (unsigned int *)&sinsize);
         if (csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if (read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }
         else
         {

            /* what is the new maximum fd ? */
            max = csock > max ? csock : max;

            FD_SET(csock, &rdfs);

            Client c = {csock};
            c.isInGame = 0;
            c.actualGame = -1;
            strncpy(c.name, buffer, BUF_SIZE - 1);
            clients[actual] = c;
            actual++;
            strcpy(buffer, "Vous pouvez entrer :\nlist : pour afficher la liste des joueurs connectés\nplay <nom_du_joueur> : pour défier un joueur\ngames : pour afficher la liste des parties");
            write_client(clients[actual - 1].sock, buffer);
         }
      }
      else
      {
         int i = 0;
         for (i = 0; i < actual; i++)
         {
            Client client = clients[i];

            /* a client is talking */
            if (FD_ISSET(clients[i].sock, &rdfs))
            {

               int c = read_client(clients[i].sock, buffer);
               /* client disconnected  */
               if (c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, clients[i].name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, clients[i], actual, buffer, 1);
               }
               else if (clients[i].isInGame == 1)
               {
                  Game * game = &games[clients[i].actualGame];
                  if (strcasecmp(clients[i].name, game->players[game->turn]) == 0)
                  {
                     int resultTurn = play_turn(buffer, game);
                     printf("%d %d de tour lalala\n", game->turn, i);
                     fflush(stdout);
                     printf("%d resultTurn\n", resultTurn);
                     fflush(stdout);
                     if(resultTurn == 1){; 

                        printf("%d de tour esshhhs\n", game->turn);
                        fflush(stdout);

                        strncpy(buffer, start_turn(*game), BUF_SIZE);
                        for (int i = 0; i < 2; i++)
                        {
                           int client_idx = find_client_index_by_name(clients,actual,game->players[i]);
                           write_client(clients[client_idx].sock,buffer);
                        }
                     } else if (resultTurn == 0) {
                        printf("ayoooooo\n");
                        fflush(stdout);
                        char* msg = findWinner(*game);
                        strncpy(buffer, msg, BUF_SIZE);
                        free(msg);
                        for (int i = 0; i < 2; i++)
                        {
                           int client_idx = find_client_index_by_name(clients,actual,game->players[i]);
                           write_client(clients[client_idx].sock,buffer);
                           clients[i].isInGame = 0;
                        }
                     } else {
                        char* msg = "Coup impossible, veuillez choisir un puit valide" ;
                        strncpy(buffer, msg, BUF_SIZE);
                        write_client(clients[i].sock,buffer);
                     }

                  } else {
                     strcpy(buffer,"Pas ton tour\n");
                     write_client(clients[i].sock, buffer);
                  }
               }
               else
               {
                  if (strcmp(buffer, "list") == 0)
                  {
                     display_list_clients(actual, clients, clients[i]);
                  }
                  else if (strncmp(buffer, "play", strlen("play")) == 0)
                  {
                     challenge_client(buffer, clients, actual, i, &server, games);
                  } else if(strcmp(buffer, "games") == 0){
                     display_list_games(games, server.actual_games, clients, clients[i], server.actual_clients, buffer);
                  }
               }
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for (i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for (i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if (sender.sock != clients[i].sock)
      {
         if (from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         printf("%s\n", message);
         fflush(stdout);
         write_client(clients[i].sock, message);
      }
      strcpy(message, "");
   }
}

static int init_server_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int find_client_index_by_name(Client *clients, int actual, const char *name)
{
   int i;
   for (i = 0; i < actual; i++)
   {
      if (strcmp(clients[i].name, name) == 0)
      {
         return i;
      }
   }
   return -1;
}

void challenge_client(char *buffer, Client *clients, int actual, int i, Server *server, Game *games)
{
   // Le client a envoyé une demande de défi
   char *spacePos = strchr(buffer, ' ');
   if (spacePos != NULL)
   {
      char challengedClientName[BUF_SIZE];
      snprintf(challengedClientName, BUF_SIZE, "%s", spacePos + 1);

      // Vérifie si le client défié existe
      int challengedClientIndex = find_client_index_by_name(clients, actual, challengedClientName);
      if (challengedClientIndex != -1)
      {
         // Envoyer la demande de défi au client défié
         char challengeMsg[BUF_SIZE];
         strcpy(challengeMsg, clients[i].name);
         strcat(challengeMsg, " vous a défié pour une partie. Acceptez-vous ? [Y/n]\n");

         write_client(clients[challengedClientIndex].sock, challengeMsg);

         // Attente de la réponse du client défié
         char response[BUF_SIZE];
         if (read_client(clients[challengedClientIndex].sock, response) != -1)
         {

            if (strncasecmp(response, "y", strlen("y")) == 0)
            {

               Game game;
               char players[2][BUF_SIZE];
               strncpy(players[0], clients[i].name, BUF_SIZE - 1);
               players[0][BUF_SIZE - 1] = '\0';
               strncpy(players[1], challengedClientName, BUF_SIZE - 1);
               players[1][BUF_SIZE - 1] = '\0'; 
               game = init_game(players);
               if (server->actual_games < MAX_GAMES)
               {
                  games[server->actual_games] = game;
                  clients[i].isInGame = 1;
                  clients[challengedClientIndex].isInGame = 1;
                  clients[i].actualGame = server->actual_games;
                  clients[challengedClientIndex].actualGame = server->actual_games;
                  server->actual_games++;
               }
               else
               {
                  fprintf(stderr, "Nombre maximal de parties atteint.\n");
               }
               strncpy(buffer, start_turn(game), BUF_SIZE);

               for (int i = 0; i < 2; i++)
               {
                  int client_idx = find_client_index_by_name(clients, actual, game.players[i]);
                  write_client(clients[client_idx].sock, buffer);
               }
            }
            else if (strncasecmp(response, "n", strlen("n")) == 0)
            {
               char declineMsg[BUF_SIZE];
               strcpy(declineMsg, clients[challengedClientIndex].name);
               snprintf(declineMsg, BUF_SIZE, " refuse le défi\n");
               write_client(clients[i].sock, declineMsg);
            }
         }
         printf("%s", response);
         fflush(stdout);
      }
      else
      {
         char notPlayerMsg[BUF_SIZE];
         snprintf(notPlayerMsg, BUF_SIZE, "Ce joueur n'existe pas.\n");
         write_client(clients[i].sock, notPlayerMsg);
      }
   }
}

static void display_list_clients(int actual, Client *clients, Client client)
{
   char list_buffer[BUF_SIZE];
   list_buffer[0] = 0;
   strncat(list_buffer, "Joueurs connectés :\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
   int i;
   for (i = 0; i < actual; i++)
   {
      strncat(list_buffer, clients[i].name, sizeof(list_buffer) - strlen(list_buffer) - 1);
      strncat(list_buffer, "\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
   }
   write_client(client.sock, list_buffer);
}

void display_list_games(Game *games, int actual_games, Client *clients, Client client, int actual_clients, char *buffer)
{
   strcpy(buffer, "Liste des parties en cours :\n");

   for (int i = 0; i < actual_games; i++)
   {
      Game game = games[i];

      sprintf(buffer + strlen(buffer),
              "Partie %d: %s vs %s - Points: %d vs %d - Statut: %s\n",
              i + 1,
              game.players[0],
              game.players[1],
              game.scores[0],
              game.scores[1],
              game.state ? "En cours" : "Terminé");
   }
   write_client(client.sock,buffer);
}

int play_turn(char *buffer, Game * game)
{
   int selectedHole = atoi(buffer);
   int nextPossible = play(*game, selectedHole) ;
   int nextMove = 1;

   if (nextPossible == 0) 
   {
      nextMove = 0;
   }
   else if (nextPossible == 2) {
      nextMove = 2;
   }
   else
   {
      if (game->turn == 0)
      {
         game->turn = 1;
      }
      else
      {
         game->turn = 0;
      }
      
   }

   return nextMove;

}

char* findWinner(Game game) {

   char* message = (char*)malloc(BUF_SIZE);
   char winner[BUF_SIZE];
   int highScore = 0;
   int lowScore = 0;
   printf("on y est\n");
   fflush(stdout);

   if (game.scores[0] > game.scores[1]) {
      strncpy(winner, game.players[0], BUF_SIZE);
      highScore = game.scores[0];
      lowScore = game.scores[1];
      snprintf(message, BUF_SIZE, "%s a gagné la partie.\n Score final : %d - %d\n\n\n Vous pouvez défier à nouveau un joueur en tapant : play <nom_joueur>\n", winner, highScore, lowScore);
      printf("gagnant1\n");
      fflush(stdout);

   } else if (game.scores[0] < game.scores[1]) {
      strncpy(winner, game.players[1], BUF_SIZE);
      highScore = game.scores[1];
      lowScore = game.scores[0];
      snprintf(message, BUF_SIZE, "%s a gagné la partie.\n Score final : %d - %d\n\n\n Vous pouvez défier à nouveau un joueur en tapant : play <nom_joueur>\n", winner, highScore, lowScore);
      printf("gagnant 2\n");
      fflush(stdout);

   } else {
      snprintf(message, BUF_SIZE, "match nul, personne n'a gagné la partie\n\n\n Vous pouvez défier à nouveau un joueur en tapant : play <nom_joueur>\n");
   }

   return message;

}

int main(int argc, char **argv)
{
   init();

   server_app();

   end();

   return EXIT_SUCCESS;
}
