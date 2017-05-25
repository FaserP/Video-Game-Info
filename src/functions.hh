#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
//#include "products/product.hh"

/**
 * Close a program using perror
 * Useful for debugging mostly
 */
void error(std::string msg) {
    perror(msg.c_str());
    exit(1);
}

/**
 * Print a message and exit the program
 * Useful for general critical errors
 */
void close(std::string message) {
    printf("%s\n", message.c_str());
    exit(1);
}

/*
 * Read a cstring from a socket. The incomming data
 * must contain the cstring EOL character \r\n.
 * The socket is read from one byte at a time until the
 * EOL bytes are read.
 */
int recv_line(int sockfd, unsigned char* &dest_buffer) {
#define EOL "\r\n"
#define EOL_SIZE 2
unsigned char *ptr;
int eol_matched = 0;

ptr = dest_buffer;
    while(read(sockfd, ptr, 1) == 1) { // Read a single byte
        if (*ptr == EOL[eol_matched]) {
            eol_matched++;
            if (eol_matched == EOL_SIZE) {
                *(ptr+1-EOL_SIZE) = '\0'; // Terminate the string
                return strlen((const char *)dest_buffer);
            }
        } else {
            eol_matched = 0;
        }
    ptr++;
    }
    return 0;
}

/**
 * Reduced complexity by only searching for carridge returns.
 * Lost robustness in the process.
 *
 */
std::string recv_line(int sockfd) {
    std::string ret;
    char ptr;
    // Read a byte of data
    while(read(sockfd, &ptr, 1) == 1) {
        ret.push_back(ptr);
        if (ptr == '\r') {
            return ret;
        }
    }
    return ret;
}

/*
 * Ensure a string is fully sent through the socket supplied
 * Return of 1 indicates success, 0 on failiure
 */
int sendAll(int sockfd, unsigned char * message) {
    int sentBytes, totalBytes;
    totalBytes = strlen((const char *)message);
    while (totalBytes > 0) {
        sentBytes = send(sockfd, message, totalBytes, 0);
        if (sentBytes == -1)
            return 0;
        totalBytes -= sentBytes; // Update the number of sent bytes
        message += sentBytes; // Move the message pointer along
    }
    return 1;
}

/**
 * Check to see if an incomming message will contain data
 * If the connection is closed or timed out then the program will end
 *
 */
void checkConnection(int sockfd) {
    char *buff = new char[1];
    int bytes = recv(sockfd, buff, 1, MSG_PEEK);
    delete[] buff;
    if (bytes == 0) {
       close("Server closed connection");
    } else if (bytes == -1) {
       close("Connection with the server timed out");
    }
}

/*
 * Saves casting to unsigned char * all the time
 */
int sendAll(int sockfd, const char * message) {
    return sendAll(sockfd, (unsigned char *) message);
}

/**
 * Serialise a product struct to a string with the following format
 * "<number"<name>"<type>"<price>"<product>"<description">\r
 * @param product The product to convert
 * @return std::string The string equivalent
 */
std::string productToString(product prod) {
    std::stringstream ss;
    ss << "\"" << prod.number << "\"";
    ss << prod.name << "\"";
    ss << prod.type << "\"";
    ss << prod.price << "\"";
    ss << prod.producer << "\"";
    ss << prod.description << "\"";
    ss << "\r";

    return ss.str();
}

/**
 * Deserialise a string to a product struct
 * @param prod The string to deserailise
 * @rturn product The equivalent product
 */
product stringToProduct(std::string prod) {
    int start, end;
    product game;
    
    // Number   
    start = prod.find_first_not_of('"');
    end = prod.find_first_not_of('"', start);
    game.number = atoi(prod.substr(start, end).c_str());
    
    // Name  
    start = prod.find_first_not_of('"', ++end);
    end = prod.find_first_of('"', start);
    game.name = prod.substr(start, end - start);

    // Type
    start = prod.find_first_not_of('"', ++end);
    end = prod.find_first_of('"', start);
    game.type = prod.substr(start, end - start);

    // Price
    start = prod.find_first_not_of('"', ++end);
    end = prod.find_first_of('"', start);
    game.price = prod.substr(start, end - start);

    // Producer
    start = prod.find_first_not_of('"', ++end);
    end = prod.find_first_of('"', start);
    game.producer = prod.substr(start, end - start);

    // Description
    start = prod.find_first_not_of('"', ++end);
    end = prod.find_first_of('"', start);
    game.description = prod.substr(start, end - start);
   
    return game; 
}

/**
 * Retrieve an integer from stdin
 * Used for the menu
 */
std::string getInput() {
    std::string ret;
    getline(std::cin, ret);
    return ret;
}
