#ifndef NETWORK_H 
#define NETWORK_H 

#include <stdint.h>

static const int32_t MAX_MSG_SIZE = 4096;

int32_t nread(int fd,char* buf, size_t n);
int32_t nwrite(int fd,char* buf, size_t n);
int32_t send_req(int fd,const char* msg);
int32_t recv_req(int client_fd);


#endif //NETWORK_H
