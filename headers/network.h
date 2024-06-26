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



typedef enum {
  ReqStatus_Ok,
  ReqStatus_Err,

} ReqStatus;
typedef struct ParsedReq 
{
  ReqStatus status;
  char* resbuf;
  int32_t resbuf_size;
} ParsedReq;

typedef enum {
  ResStatus_Ok,
  ResStatus_NoKey,
  ResStatus_CNF, // cmd not found
} ResStatus;

int32_t nread(FileDesc fd,char* buf, size_t n);
int32_t nwrite(FileDesc fd,char* buf, size_t n);
int32_t send_req(FileDesc fd,const char* msg);
int32_t recv_req(FileDesc client_fd);
int fd_set_nb(FileDesc fd);





#endif //NETWORK_H
