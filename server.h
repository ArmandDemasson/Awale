#ifndef SERVER_H
#define SERVER_H

#include "game.h"

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1978
#define MAX_CLIENTS     100
#define MAX_GAMES 100

#define BUF_SIZE    1024

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   int isInGame;
   int actualGame;
}Client;

typedef struct
{
    int max_clients;
    int max_games;
    int actual_clients;
    int actual_games;
} Server;


static void init(void);
static void end(void);
static void server_app(void);
static int init_server_connection(void);
int find_client_index_by_name(Client *clients, int actual, const char *name);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);
void challenge_client(char * buffer, Client * clients, int actual, int i, Server * server, Game * games);
static void display_list_clients(int actual, Client *clients, Client client);
int play_turn(char *buffer, Game * game);
char* findWinner(Game game);
void display_list_games(Game *games, int actual_games, Client *clients, Client client, int actual_clients, char *buffer);



#endif /* guard */