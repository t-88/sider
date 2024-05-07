
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

  printf("recved: %.*s\n",msg_len,&conn->rbuf[4]);

  memcpy(&conn->wbuf[0],&msg_len,4);
  memcpy(&conn->wbuf[4],&conn->rbuf[4] ,msg_len);
  conn->wbuf_size = 4 + msg_len;

  size_t remain = conn->rbuf_size - 4 - msg_len;
  if(remain) 
  {
    memmove(conn->rbuf,&conn->rbuf[4 + msg_len],remain);
  }
  conn->rbuf_size = remain;
  conn->status = ConnStatus_Res;
  handle_res(conn);

  return conn->status == ConnStatus_Req;
}


