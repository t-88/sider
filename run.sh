#!/bin/sh

set -eax
gcc -g src/main.c src/server.c src/client.c src/network.c -o main -I./headers
# g++ -g src/main.c src/server.c src/client.c src/network.c -o main -I./headers
./main $1
