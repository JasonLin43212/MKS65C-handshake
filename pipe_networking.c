#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  if (mkfifo("known", 0666) == -1){
    printf("Error making known pipe\n");
    exit(1);
  }
  if (errno){
    printf("Error #%d: %s",errno,strerror(errno));
    exit(1);
  }
  // You need to have both ends open before you can proceed in your code.
  int up = open("known",O_RDONLY);

  char client_message[8];
  read(up, client_message, 8);
  printf("Server recieved the client's message, which is: %s\n",client_message);

  int down = open(client_message, O_WRONLY);

  write(down,"I have gotten your response",sizeof("I have gotten your response"));

  char client_response[100];
  read(up,client_response,100);

  printf("Server got the client's reply: %s\n",client_response);
  printf("Three way handshake done. Shutting down...\n");

  remove("known");

  return 0;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  if (mkfifo("private", 0666) == -1){
    printf("Error making pipe");
  }
  if (errno){
    printf("Error #%d: %s",errno,strerror(errno));
    exit(1);
  }

  int up = open("known", O_WRONLY);
  write(up,"private",sizeof("private"));

  int down = open("private", O_RDONLY);
  char response[100];
  read(down,response,100);

  printf("The server has responded with: %s\n", response);
  printf("Telling the server that I heard the response.\n");
  write(up,"I got your response!",sizeof("I got your response!"));

  printf("Three way handshake done. Shutting down...\n");

  remove("private");

  return 0;
}
