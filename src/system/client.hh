// Network libs
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Core libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

// STL
#include <vector>

// Custom libs
#include "../data/product.hh"
#include "../functions.hh"
#include "status_codes.hh"

class Client {
public:
    int port, sockfd, recievefd;
public:
    Client(char *, char *);
    int getNumRows();
    bool sendRequest(unsigned char *);
    std::vector<product> recieveQuery();
    bool checkStatusCode();
    void closeSocket();
};
