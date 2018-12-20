#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "pipe_networking.h"

static void sighandler(int signo) {
  if (signo == SIGINT){
    remove("known");
    exit(1);
  }
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  signal(SIGINT,sighandler);
  mkfifo("known", 0666);
  // You need to have both ends open before you can proceed in your code.
  int up = open("known",O_RDONLY);

  char client_message[8];
  read(up, client_message, 8);
  printf("Client pipe name: %s\n",client_message);
  remove("known");

  int down = open(client_message, O_WRONLY);
  *to_client = down;

  char client_response[1000] = "";
  while(1){
    read(up,client_response,1000);
    printf("\nClient's message\n===============\n%s\n===============\n",client_response);
    printf("Sent client their response but uppercased.\nWaiting for next message...\n");

    char * send_to_client = malloc(1000);
    int i;
    for (i=0; i<(int)strlen(client_response); i++){
      send_to_client[i] = toupper(client_response[i]);
    }
    write(down,send_to_client,1000);
  }

  return up;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  mkfifo("private", 0666);
  int up = open("known", O_WRONLY);
  write(up,"private",sizeof("private"));

  *to_server = up;

  int down = open("private", O_RDONLY);

  char response[1000] = "";
  remove("private");
  while(1){
    printf("Enter message to server: ");
    char message[1000] = "";
    fgets(message,1000,stdin);

    message[strlen(message) -1] = '\0';
    write(up,message,1000);

    read(down,response,1000);
    printf("\nServer Response\n===============\n%s\n===============\n", response);
  }

  return down;
}
