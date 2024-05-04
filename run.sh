#!/bin/sh

set -eax
gcc src/main.c src/server.c src/client.c src/network.c -o main -I./headers
./main $1
