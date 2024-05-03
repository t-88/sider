
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


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


  char wbuf[] = "SUP MA BOY";
  write(fd,wbuf,sizeof(wbuf));

  char rbuf[64];
  read(fd,rbuf,sizeof(rbuf) - 1);
  printf("MSG: server send: %s\n",rbuf);


clean_up:
  close(fd);
  return out;
}
