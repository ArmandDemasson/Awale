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
   Game game;
   fd_set rdfs;

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
            strncpy(c.name, buffer, BUF_SIZE - 1);
            clients[actual] = c;
            actual++;
            strncat(buffer, " vient de se connecter\n", sizeof(buffer) + strlen(buffer) - 1);
            printf("%d joueurs sont connectés\n", actual);
            fflush(stdout);
            send_message_to_all_clients(clients, c, actual, buffer, 1);
            // if (actual == 2)
            // {
            //    game = init_game(clients);
            //    strncpy(buffer, start_turn(game), BUF_SIZE);
            //    for (int i = 0; i < actual; i++)
            //    {
            //       send_message_to_all_clients(clients, clients[i], actual, buffer, 1);
            //    }
            // }
         }
      }
      else
      {
         int i = 0;
         strcpy(buffer, "");
         for (i = 0; i < actual; i++)
         {
            Client client = clients[i];

            /* a client is talking */
            if (FD_ISSET(clients[i].sock, &rdfs))
            {
               // if (clients[i].name == game.players[game.turn].name)
               // {

               int c = read_client(clients[i].sock, buffer);

               printf("%s", buffer);
               /* client disconnected */
               if (c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, clients[i].name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, clients[i], actual, buffer, 1);
               }
               else if (strcmp(buffer, "list") == 0)
               {
                  display_list_clients(actual, clients, clients[i]);
               }
               else
               {
                  send_message_to_all_clients(clients, clients[i], actual, buffer, 0);
               }
               // int selectedHole = atoi(buffer);

               // if (play(game, selectedHole) == 0)
               // {
               //    printf("choisir un puit valide\n");
               //    fflush(stdout);
               // }
               // else
               // {
               //    if (game.turn == 0)
               //    {
               //       game.turn = 1;
               //    }
               //    else
               //    {
               //       game.turn = 0;
               //    }

               //    strncpy(buffer, start_turn(game), BUF_SIZE);
               //    for (int i = 0; i < actual; i++)
               //    {
               //       send_message_to_all_clients(clients, clients[i], actual, buffer, 1);
               //    }
               // }
            }

            break;
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

int main(int argc, char **argv)
{
   init();

   server_app();

   end();

   return EXIT_SUCCESS;
}
