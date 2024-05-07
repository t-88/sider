
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "network.h"
#include "client.h"


int client() 
{
  int out = 0;

  int fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd == -1) {
    out = -1;
    printf("ERROR: failed create client socket\n");
    goto clean_up;
  }
  
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(6969);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
  
  out = connect(fd,(const struct sockaddr*)&addr,sizeof(addr));
  if(out) 
  {
    printf("ERROR: failed to connect to server\n");
    goto clean_up;
  }
  send_req(fd,"sup");
  send_req(fd,"sup");
   recv_req(fd);
   recv_req(fd);

clean_up:
  close(fd);
  return out;
}
