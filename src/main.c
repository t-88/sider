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

#include "ht.h"


typedef struct HT_string {
  HT_Node node;
  char val[16];
} HT_string;

void main(void) {

  HTable* table = ht_init(4);

  ht_insert(table,"asd","asd");
  ht_insert(table,"ur mom","coool");
  ht_insert(table,"kinda cool","asd");
  ht_insert(table,"kinda cool","ur mom");
  ht_print(table);

  ht_free(table);
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
