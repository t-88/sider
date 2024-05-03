#!/bin/sh

set -eax
gcc main.c server.c client.c -o main -I./headers
./main $1
