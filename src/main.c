#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"
#include "client.h"

#define DA_IMPLEMENTATION
#include "da.h"

#define HMAP_IMPLEMENTATION
#include "hm.h"


void main(void) {

  HMap* table = hm_init();

  hm_insert(table,"asd","asd");
  hm_insert(table,"ur mom","coool");
  hm_insert(table,"kinda cool","asd");
  hm_insert(table,"kinda cool","ur mom");
  hm_insert(table,"1","42 69");
  hm_print(table);


  char* val = hm_get(table,"1");
  printf("%s\n",val);

  hm_free(table);
}


void how_to_use_cmd() {
    printf("./run.sh <user> # user = {server , client}\n");
}

int main1(int argc, char** argv)
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
