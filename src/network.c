
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "network.h"
#include "da.h"

typedef struct DA_string {
  char** items;
  uint32_t capacity;
  uint32_t len;
} DA_string;


int32_t nread(int fd,char* buf, size_t n) 
{
  while(n > 0) 
  {
    
    errno = 0;
    int rv = read(fd,buf,n);
    if(rv <= 0) 
    {
      if(errno != 0) 
      {
        printf("ERROR: failed to read msg\n");
      }
      return -1;
    }
    assert(rv <= n && "ASSERTION FAILED: read more then the size of the buffer");
    n -= rv;
    buf += rv;
  }

  return 0;
} 

int32_t nwrite(int fd,char* buf, size_t n) 
{
  while(n > 0) 
  {
    int rv = write(fd,buf,n);
    if(rv <= 0) 
    {
      printf("ERROR: failed to write msg\n");
      return -1;
    }
    assert(rv <= n && "ASSERTION FAILED: wrote more then the size of the buffer");
    n -= rv;
    buf += rv;
  }

  return 0;
} 



int32_t send_req(int fd,const char* msg) 
{
  uint32_t msg_len = strlen(msg);
  if(msg_len > MAX_MSG_SIZE) 
  {
    printf("ERROR: failed to send req msg is too long\n");
    return -1;
  }
  char buf[4 + MAX_MSG_SIZE] ;
  memcpy(buf,&msg_len,4);
  memcpy(&buf[4],msg,msg_len);

  nwrite(fd,buf,4 + msg_len);
}


int32_t recv_req(int client_fd) {

  char buf[4 + MAX_MSG_SIZE + 1];
  
  errno = 0;
  int32_t rv = nread(client_fd,buf,4);
  if(rv) {
    if(errno == 0) 
    {
      printf("EOF\n");
    } else 
    {
      printf("ERROR: failed to recv request couldnt read msg len\n");
    }
    return -1;
  }

  int32_t msg_len = 0;
  memcpy(&msg_len,buf,4); 
  if(msg_len > MAX_MSG_SIZE) 
  {
    printf("ERROR: failed to recv request msg is too long\n");
    return -1;   
  }

  
  rv = nread(client_fd,&buf[4],msg_len);
  if(rv) {
    printf("ERROR: failed to recv request couldnt read msg\n");
    return -1;
  }

  buf[4 + MAX_MSG_SIZE] = '\0';

  printf("recv: %s\n",&buf[4]);
}


int fd_set_nb(int fd) 
{
  errno = 0;
  int flags = fcntl(fd,F_GETFL,0);
  if(errno) 
  {
    printf("ERROR: failed to get fd flags\n");
    return -1;
  }

  flags |= O_NONBLOCK;
  errno = 0;
  fcntl(fd,F_SETFL,flags);
  if(errno) 
  {
    printf("ERROR: failed to set fd flags\n");
    return -1;
  }
  return 0;
}


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



bool execute_get(DA_string cmd) {
  assert(cmd.len == 2);
  printf("get %s\n",cmd.items[1]);
  return true;
}

bool execute_set(DA_string cmd) {
  assert(cmd.len == 3);
  printf("set %s = %s\n",cmd.items[1],cmd.items[2]);
  return true;
}

bool execute_del(DA_string cmd) {
  assert(cmd.len == 2);
  printf("del %s\n",cmd.items[1]);
  return true;
}

bool parse_execute_req(int32_t req_len,const char* req) 
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
  
  bool out = true;
  if(cmd.len == 2 && strcmp(cmd.items[0],"get") == 0) {
    out = execute_get(cmd);
  } else if(cmd.len == 2 && strcmp(cmd.items[0],"del") == 0) { 
     out = execute_del(cmd);
  } else if(cmd.len == 3 && strcmp(cmd.items[0],"set") == 0) { 
     out = execute_set(cmd);
  } else {
    out = false;
  }
  da_free_items(cmd);
  da_free(cmd);
  return out;
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

  if(!parse_execute_req(msg_len,&conn->rbuf[4]))
  {
    return false;  
  }

  // printf("recved: %.*s\n",msg_len,&conn->rbuf[4]);

  size_t remain = conn->rbuf_size - 4 - msg_len;
  if(remain) 
  {
    memmove(conn->rbuf,&conn->rbuf[4 + msg_len],remain);
  }
  conn->rbuf_size = remain;

  return conn->status == ConnStatus_Req;
}

