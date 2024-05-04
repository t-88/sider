
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
#include "server.h"




int server() 
{
  int out = 0;

  // create a tcp scoket 
  int fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd == -1) 
  {
    printf("ERROR: failed create server socket\n");
    out = -1;
    goto clean_up;
  }

  // enable re-using the addr option, to avoid same addr problems later
  int val = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(6969);
  addr.sin_addr.s_addr = ntohl(0);

  out = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
  if(out) 
  {
    printf("ERROR: failed to bind socket \n");
    goto clean_up;
  }
  
  out = listen(fd,SOMAXCONN);
  if(out == -1) 
  {
    printf("ERROR: failed to listen\n");
    goto clean_up;
  }

  while(true) 
  {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(fd,(struct sockaddr*) &client_addr,&addrlen);
    if(client_fd < 0) 
    {
      printf("WARN: failed to accpect client\n");
    }

    recv_req(client_fd);    

    close(client_fd);
  }

  
clean_up:
  close(fd);
  return out;
}
