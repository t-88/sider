
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

  char buf[4 + msg_len] ;
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
