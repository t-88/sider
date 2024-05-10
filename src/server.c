
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
#include <errno.h>
#include <signal.h>

#include "network.h"
#include "server.h"
#include "da.h"
#include "hm.h"

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

typedef struct DA_string {
  char** items;
  uint32_t capacity;
  uint32_t len;
} DA_string;




typedef struct ServerState {
  FileDesc fd;
  DA_Conn_ptr fd2conn;
  DA_Pollfd poll_args;
  HMap* storage;
} ServerState;
ServerState state;

void handle_req(Conn* conn) 
{
  while(true) 
  {
    assert(conn->rbuf_size < sizeof(conn->rbuf));
    int rv = 0;
    do {
      errno = 0;
      size_t read_size = sizeof(conn->rbuf) - conn->rbuf_size;
      rv = read(conn->fd,&conn->rbuf[conn->rbuf_size],read_size);
    } while(rv < 0 && errno == EINTR);


    if(rv < 0 && errno == EAGAIN){
      return;
    }

    if(rv < 0) {
      conn->status = ConnStatus_Exit;
      printf("ERROR: failed to read\n");
      // perror("read()");
      return;
    }

    if(rv == 0) 
    {
      if(conn->rbuf_size > 0) 
      {
        printf("ERROR: unexpected EOF\n");
      } else {
        printf("EOF\n");
      }
      conn->status = ConnStatus_Exit;
      return;
    }

    conn->rbuf_size += rv;
    assert(conn->rbuf_size <= sizeof(conn->rbuf));

    while(recv_req_conn(conn)) {}
    if(conn->status != ConnStatus_Req) {
      return;
    }
  }
}

void handle_res(Conn* conn) 
{
  while(true) 
  {
    size_t rv = 0;
    do {
      errno = 0;
      size_t remain = conn->wbuf_size - conn->wbuf_sent;
      rv = write(conn->fd,&conn->wbuf[conn->wbuf_sent],remain);
    } while(rv < 0 && errno == EINTR);

      
    if(rv < 0 && errno == EAGAIN) {
      printf("ERROR: response EAGAIN\n");
      return;
    } 


    if(rv < 0) 
    {
      printf("ERROR: failed to write to fd\n");
      conn->status = ConnStatus_Exit;
      return; 
    }

    conn->wbuf_sent += rv;
    
    // buffer was sent ssuccessfully
    if(conn->wbuf_sent == conn->wbuf_size) 
    {
      conn->status = ConnStatus_Req;
      conn->wbuf_sent = 0;
      conn->wbuf_size = 0;
      return;
    }

  }

}

typedef struct ResData {
  ResStatus status;
  char data[MAX_MSG_SIZE];
} ResData;

ResData execute_get(DA_string cmd) {
  // printf("get\n"); 
  assert(cmd.len == 2);
  
  ResData out = {0};
  out.status = ResStatus_Ok;
  char* val = hm_get(state.storage,cmd.items[1]);


  

  if(val == NULL) {
    out.status = ResStatus_NoKey;
  } else {
    strcpy(out.data,"get ");
    strcat(out.data,cmd.items[1]);
    strcat(out.data," ");
    strcat(out.data,val);
  }
  return out;
}

ResData execute_set(DA_string cmd) {
  // printf("set\n");
  assert(cmd.len == 3);

  ResData out = {0};
  out.status = ResStatus_Ok;
  hm_insert(state.storage,cmd.items[1],cmd.items[2]);

  strcpy(out.data,"set ");
  strcat(out.data,cmd.items[1]);
  strcat(out.data," ");
  strcat(out.data,cmd.items[2]);

  return out;
}

ResData execute_del(DA_string cmd) {
  // printf("del\n");
  assert(cmd.len == 2);

  ResData out = {0};
  out.status = ResStatus_Ok;
  hm_delete(state.storage,cmd.items[1]);


  strcpy(out.data,"del ");
  strcat(out.data,cmd.items[1]);
  return out;
}

ResData parse_execute_req(int32_t req_len,const char* req) 
{
  ParsedReq p = {0};
  DA_string cmd = {.capacity = 8}; 
  da_init(cmd);

  
  // split by space
  int last_i = 0;
  for(int i = 0; i < req_len; i++) 
  {
    if(req[i] == ' ' || i == req_len - 1) 
    {
      if(i == req_len - 1 && req[i] != ' ') {
        i++;
      }
      char* buf = malloc(i - last_i + 1);
      memcpy(buf,&req[last_i],i - last_i);
      buf[i - last_i] = '\0';
      last_i = i + 1;

      da_append(cmd,buf);
    }
  }
  
  ResData res_data = {}; 
  bool out = true;


  if(cmd.len == 2 && strcmp(cmd.items[0],"get") == 0) {
    res_data = execute_get(cmd);
  } else if(cmd.len == 2 && strcmp(cmd.items[0],"del") == 0) { 
     res_data = execute_del(cmd);
  } else if(cmd.len == 3 && strcmp(cmd.items[0],"set") == 0) { 
     res_data = execute_set(cmd);
  } else {
    res_data.status = ResStatus_CNF;
  }




  da_free_items(cmd);
  da_free(cmd);
  return res_data;
}
bool recv_req_conn(Conn* conn) 
{
  if(conn->rbuf_size < 4) 
  {
    return false;
  }
  
  uint32_t msg_len = 0;
  memcpy(&msg_len,conn->rbuf,4); 

  // not enough data 
  if(4 + msg_len > conn->rbuf_size) 
  {
    conn->status = ConnStatus_Exit;
    return false;
  }

  ResData res_data = parse_execute_req(msg_len,&conn->rbuf[4]);

  conn->status = ConnStatus_Res;
  conn->wbuf_sent = 0; 
  int32_t wlen = 4;
  char wbuf[4 + MAX_MSG_SIZE];
  
  if(res_data.status == ResStatus_Ok) {
    strcpy(wbuf,"[1] ");
    if(res_data.data) { 
      wlen += strlen(res_data.data);
      strcpy(&wbuf[4],res_data.data);
    }
  } else {
    strcpy(wbuf,"[0] ");
  }
  
  conn->wbuf_size = wlen + 4; 
  memcpy(&conn->wbuf[0],&wlen,4);
  memcpy(&conn->wbuf[4],wbuf,wlen);
  handle_res(conn);


  size_t remain = conn->rbuf_size - 4 - msg_len;
  if(remain) 
  {
    memmove(conn->rbuf,&conn->rbuf[4 + msg_len],remain);
  }
  conn->rbuf_size = remain;

  return conn->status == ConnStatus_Req;
}



int accept_new_conn(DA_Conn_ptr* fd2conn, FileDesc fd) 
{ 
  struct sockaddr_in client_addr = {};
  socklen_t addrlen = sizeof(client_addr);
  int client_fd = accept(fd,(struct sockaddr*) &client_addr,&addrlen);


  if(client_fd < 0) 
  {
    printf("ERROR: failed to accpect client\n");
    close(client_fd);
    return -1;
  }


  fd_set_nb(client_fd);
  Conn* conn = (Conn*)malloc(sizeof(*conn));
  if(!conn) 
  {
    printf("ERROR: malloc failed to allocate new Conn\n");
    close(client_fd);
    return -1;
  }

  conn->fd = client_fd;
  conn->status = ConnStatus_Req;
  conn->rbuf_size = 0;
  conn->wbuf_size = 0;
  conn->wbuf_sent = 0;
  da_set(*fd2conn,conn->fd,conn);
  
  return 0;
}



void handle_connection_io(Conn* conn) 
{
  if(conn->status == ConnStatus_Req) 
  {
    handle_req(conn);
  } else if(conn->status == ConnStatus_Res) 
  {
    handle_res(conn);
  } else {
    assert(false && "Unreachable: the code should've handled this state");
  }
}


void state_clean_up();

void INT_handler(int sig) {
  signal(sig,SIG_IGN);
  state_clean_up();
  printf("\nserver closed :)\n");
  exit(0);
}

int server() 
{
  signal(SIGINT,INT_handler);
  int out = 0;

  // create a tcp scoket 
  state.fd = socket(AF_INET,SOCK_STREAM,0);
  if(state.fd < 0) 
  {
    printf("ERROR: failed create server socket\n");
    out = -1;
    goto clean_up;
  }

  // enable re-using the addr option, to avoid same addr problems later
  int val = 1;
  setsockopt(state.fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(6969);
  addr.sin_addr.s_addr = ntohl(0);

  out = bind(state.fd,(struct sockaddr*)&addr,sizeof(addr));
  if(out) 
  {
    printf("ERROR: failed to bind socket \n");
    goto clean_up;
  }
  
  out = listen(state.fd,SOMAXCONN);



  if(out == -1) 
  {
    printf("ERROR: failed to listen\n");
    goto clean_up;
  }
 

  fd_set_nb(state.fd); 

  state.fd2conn = (DA_Conn_ptr) {.capacity = 16};

  da_init(state.fd2conn);
  state.poll_args = (DA_Pollfd) {.capacity = 16 };
  da_init(state.poll_args);

  state.storage = hm_init();

  while(true) 
  {
    da_clear(state.poll_args);
    da_append(state.poll_args,  ((struct pollfd)  {state.fd, POLLIN ,0 }));
    

    // change this mapping code
    // fd2conn wants to map fd to connections, but the way its doing it is wastfull
    for(int i = 0; i < state.fd2conn.capacity; i++) 
    {
      if(!state.fd2conn.items[i]) 
      {
        continue;
      }
      
      Conn* conn = state.fd2conn.items[i]; 
      struct pollfd pfd;
      pfd.fd = conn->fd;
      pfd.events = (conn->status == ConnStatus_Req) ? POLLIN : POLLOUT;
      pfd.events |= POLLERR;
      da_append(state.poll_args,pfd);
    }
    int rv = poll(state.poll_args.items,state.poll_args.len,1000);
    if(rv < 0) 
    {
      printf("ERROR: failed to poll connections");
      goto clean_up;
    }
  


    for(int i = 1; i < state.poll_args.len; i++) 
    {
      if(state.poll_args.items[i].revents == 0) 
      {
        continue;
      }

      Conn* conn = state.fd2conn.items[state.poll_args.items[i].fd];
      handle_connection_io(conn);

      if(conn->status == ConnStatus_Exit) 
      {
        state.fd2conn.items[conn->fd] = NULL;
        close(conn->fd);
        free(conn);
      }
    }
  

    if(state.poll_args.items[0].revents) 
    {
      accept_new_conn(&state.fd2conn,state.fd);
    }
  }

  
clean_up:
  state_clean_up();
  return out;
}



void state_clean_up() {
  da_free(state.poll_args);

  for(int i = 0; i < state.fd2conn.len; i++) 
  {
    if(state.fd2conn.items[i]) 
    {
      free(state.fd2conn.items[i]);
    }
  }
  da_free(state.fd2conn);
  hm_free(state.storage);
  close(state.fd);
}
