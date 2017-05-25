#include <stdio.h> // Error reporting
#include <stdlib.h> // itoa
#include <string.h> // Memset

// Networking libs
#include <sys/socket.h> // Socket functions and stucts
#include <netinet/in.h> //AF_INET families
#include <arpa/inet.h> // Endian conversions
#include <fcntl.h> // To set non-blocking 

#include "../data/products.cc"
#include "../functions.hh"
#include "status_codes.hh"


#define QUEUE_SIZE 1 // The number of active connections available at any one time
class Server {
private:
    int sockfd;
    fd_set socks; // Contains all sockets to listen to for read/write ops.
    int connectList[QUEUE_SIZE]; // Holds the connected sockets
    int highSock; // Greatest fd for select()
public:
    Server(int);
private:
    void setNonBlocking(int); // Set a socket to be non-blocking
    void buildFDSet(); // Fill the fd_set with connections
    void addNewConnection(); // Add a new connection to the connectList
    void handleConnection(int); // Read from a socket
};
