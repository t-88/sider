# Notes

**AF_INET**: domain means used  for IPv4  (AF_INET6 for IPv6)   

**SOCK_STREAM**: type of socket used for TCP connections, SOCK_DGRAM is for UDP   

**Nagle Algorithm**: TCP optimazation algorithm, sums up all the small packets into a big packet   
**SO_REUSEADDR**: without it bind will fail to connect to client after server restart, its related to TIME_WAIT, i think i stumbled to a problem like this when playing a ctf about a server build with c    

**Three ways to deal with concurrent connections**:
- forking, i know i can exploit it in pwn to baypass the canary protection. pretty cool 
- multi-threading
- non blocking ops: async ops..., read,write,connect have non blocking modes, every blocking action can be perfomed in smth called thread pools. 
  - use fcntl to set fd to nonblocking, used to read and write fd flags 
  - **poll** gets all the active fds that can perform operations, its used in non blocking mode. use **epool** for more advanced stuff.




### Info
- [how to get the next power of two](https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2)
- [bit twiddling hacks](https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2)

# Ref
- [beej.us guide](https://beej.us/guide/bgnet/html/split/)
- [make your own redis](https://build-your-own.org/redis/03_hello_cs)
