#ifndef NETWORK_H 
#define NETWORK_H 

#include <stdint.h>

#define MAX_MSG_SIZE  4096
typedef int FileDesc;


typedef enum {
  ConnStatus_Req,
  ConnStatus_Res,
  ConnStatus_Exit,
} ConnStatus;

typedef struct Conn {
  FileDesc fd;
  ConnStatus status; 
  
  size_t rbuf_size;
  char rbuf[4 + MAX_MSG_SIZE];

  size_t wbuf_size;
  size_t wbuf_sent;
  char wbuf[4 + MAX_MSG_SIZE];
} Conn;


int32_t nread(FileDesc fd,char* buf, size_t n);
int32_t nwrite(FileDesc fd,char* buf, size_t n);
int32_t send_req(FileDesc fd,const char* msg);
int32_t recv_req(FileDesc client_fd);
int fd_set_nb(FileDesc fd);


void handle_req(Conn* conn);
void handle_res(Conn* conn);
bool recv_req_conn(Conn* connn);



#endif //NETWORK_H
