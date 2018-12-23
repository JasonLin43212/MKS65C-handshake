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
  while (1){
    mkfifo("known", 0666);
    // You need to have both ends open before you can proceed in your code.
    int up = open("known",O_RDONLY);

    char private_pipe[8];
    read(up, private_pipe, 8);
    printf("\nClient pipe name: %s\n",private_pipe);
    remove("known");

    char up_pipe[80] = "up";
    char down_pipe[80] = "down";

    strcat(up_pipe,private_pipe);
    strcat(down_pipe,private_pipe);

    int private_up = open(up_pipe, O_RDONLY);
    int private_down = open(down_pipe, O_WRONLY);
    *to_client = private_down;

    int child = fork();

    if (child == 0){
      char client_response[1000] = "";
      while(read(private_up,client_response,1000)){
        printf("\nClient %s message\n===============\n%s\n===============\n",private_pipe,client_response);
        printf("Sent client %s their response but uppercased.\nWaiting for next message...\n",private_pipe);

        char * send_to_client = malloc(1000);
        int i;
        for (i=0; i<(int)strlen(client_response); i++){
          send_to_client[i] = toupper(client_response[i]);
        }
        write(private_down,send_to_client,1000);
      }

      return up;
    }
  }


}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  char up_pipe[80] = "up";
  char down_pipe[80] = "down";

  char private_pipe[20] = "";
  sprintf(private_pipe,"%d",getpid());

  strcat(up_pipe, private_pipe);
  strcat(down_pipe, private_pipe);

  mkfifo(up_pipe, 0666);
  mkfifo(down_pipe, 0666);

  int up = open("known", O_WRONLY);
  write(up,private_pipe,sizeof(private_pipe));
  *to_server = up;
  close(up);

  int pipe_up = open(up_pipe, O_WRONLY);
  int pipe_down = open(down_pipe, O_RDONLY);

  char response[1000] = "";
  remove(up_pipe);
  remove(down_pipe);
  char message[1000] = "";

  while(1){
    printf("Enter message to server: ");
    fgets(message,1000,stdin);

    message[strlen(message) -1] = '\0';
    write(pipe_up,message,1000);

    read(pipe_down,response,1000);
    printf("\nServer Response\n===============\n%s\n===============\n", response);
  }

  return pipe_down;
}
