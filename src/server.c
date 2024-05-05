
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>


#include "network.h"
#include "server.h"

typedef struct DA_Conn_ptr {
  Conn** items;
  size_t len;
  size_t capacity;
} DA_Conn_ptr;


typedef struct DA_Pollfd {
  struct pollfd* items;
  size_t len;
  size_t capacity;
} DA_Pollfd;



#define da_append(da,item)                                                     \
{                                                                              \
  if((da).capacity == 0)                                                       \
  {                                                                            \
    (da).capacity = 1;                                                         \
  }                                                                            \
  if((da).items == NULL) {                                                     \
    (da).items = realloc(da.items,sizeof(da.items[0]) * (da).capacity);        \
    assert((da).items != NULL && "ERROR: looks like you got no memory");       \
  } else  if((da).len == (da).capacity - 1)                                    \
  {                                                                            \
    (da).capacity *= 2;                                                        \
    (da).items = realloc(da.items,sizeof(da.items[0]) * (da).capacity);        \
    assert((da).items != NULL && "ERROR: looks like you got no memory");       \
  }                                                                            \
  (da).items[(da).len] = (item);                                               \
  (da).len += 1;                                                               \
} 

#define da_clear(da)                                                           \
{                                                                              \
  (da).len = 0;                                                                \
}


#define da_free(da)                                                            \
{                                                                              \
  free((da).items);                                                            \
}

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
 

  fd_set_nb(fd); 

  DA_Conn_ptr fd2conn = {.capacity = 16};
  DA_Pollfd poll_args = {.capacity = 16 };

  while(true) 
  {
    da_clear(poll_args);
    da_append(poll_args,  ((struct pollfd)  {fd, POLLIN ,0 }));

    for(int i = 0; i < fd2conn.len; i++) 
    {
      if(!fd2conn.items[i]) 
      {
        continue;
      }
      
      Conn* conn = fd2conn.items[i]; 
      struct pollfd pfd;
      pfd.fd = conn->fd;
      pfd.events = (conn->status == ConnStatus_Req) ? POLLIN : POLLOUT;
      pfd.events |= POLLERR;
      da_append(poll_args,pfd);
    }

    int rv = poll(poll_args.items,poll_args.len,1000);
    if(rv < 0) 
    {
      printf("ERROR: failed to poll connections");
      goto clean_up;
    }
  

    for(int i = 0; i < poll_args.len; i++) 
    {
      if(poll_args.items[i].revents == 0) 
      {
        continue;
      }
      Conn* conn = fd2conn.items[poll_args.items[i].fd];
      if(conn->status == ConnStatus_Exit) 
      {
        fd2conn.items[conn->fd] = NULL;
        close(conn->fd);
        free(conn);
      }
    }

    goto clean_up;


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
  da_free(poll_args);

  for(int i = 0; i < fd2conn.len; i++) 
  {
    if(fd2conn.items[i]) 
    {
      free(fd2conn.items[i]);
    }
  }
  da_free(fd2conn);

  

  close(fd);
  return out;
}
