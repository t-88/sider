#ifndef SERVER_H
#define SERVER_H

#include "network.h"

void handle_req(Conn* conn);
void handle_res(Conn* conn);
bool recv_req_conn(Conn* connn);
int server();

#endif //SERVER_H
