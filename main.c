#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"
#include "client.h"



void how_to_use_cmd() {
    printf("./run.sh <user> # user = {server , client}\n");
}

int main(int argc, char** argv)
{
  if(argc <= 1) 
  {
    printf("ERROR: expected arg server or client\n");
    return -1;
  }

  if(strcmp(argv[1],"server") == 0) {
    server();
  } else if (strcmp(argv[1],"client") == 0) {
    client();
  } else {
    printf("ERROR: unknown user provided '%s'\n",argv[1]);
    how_to_use_cmd();
    return -1;
  }


  

  return 0;
}
