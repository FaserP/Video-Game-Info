#include "client.hh"

/*
 * Setup the connection to the server. 
 */
Client::Client(char * ip, char * port) {
int result; // Hold return codes from function calls
struct addrinfo hints; // Perfered socket type (TCP)
struct addrinfo *res; // Linked list of valid addrinfo structs
memset(&hints, 0, sizeof(hints)); // Empty the hints struct
hints.ai_family = AF_UNSPEC; // We want ipv4 or ipv6
hints.ai_socktype = SOCK_STREAM; // TCP connection only

    // Place all valid addresses found in res
    result = getaddrinfo(ip, port, &hints, &res);
    if (result != 0) {
        // Use my error method instead?
        printf("getaddrinfo error : %s\n",gai_strerror(result));
        exit(1);
    }
    
    // We should really check to make sure res is what we wanted
    // But for now, we will assume the first result is right
    // and created the socket with the data
    this->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (this->sockfd < 0) {
        error("Error opening socket");
    }
   
    // Add a timeout value of 10 seconds for input functions.
    // If the server goes down then the client will not block
    // for ever waiting for the server to respond
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    result = setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));

    if (result < 0) {
        error("Error adding a timeout to socket");
    }

    // Connect to the server
    result = connect(this->sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res); // Clear the linked list memory
    
    if (result < 0) {
        error("Error connecting to server");
    }
    
    checkStatusCode();
}

/**
 * Return the number of rows in the games table from the
 * remote database
 */
int Client::getNumRows() {
    this->sendRequest((unsigned char *) "GET COUNT");
    if (this->checkStatusCode()) {
        std::string s(recv_line(this->sockfd));
        return atoi(s.c_str());
    }
    return 0;
}

/**
 * Recieves a reply from the server and returns based on what the
 *  reply states.
 * @return True if the reply is successful, false if there is a problem
 */
bool Client::checkStatusCode() {
    // Check the status of the incomming data
    checkConnection(this->sockfd);
    // All is okay, get the data
    std::string s(recv_line(this->sockfd));
    
    // The status codes have a length of 4 :S
    if (s.compare(0, 4, SUCCESS) == 0) {
        return true;
    } else if(s.compare(0, 4, NOT_FOUND) == 0) {
        return false;
    } else if(s.compare(0, 4, FORBIDDEN) == 0) {
        return false;
    } else if (s.compare(0, 4, UNAVAILABLE) == 0) {
        closeSocket();
        close("The server is too busy at the moment");
    }
    return false; // Some unknown value (function called out of turn)
                  // Maybe I should quit the program here
}

/**
 * Send a message to the server
 * This method seems pretty pointless at the moment
 * but is being used by the menu.
 */
bool Client::sendRequest(unsigned char * message) {
    // Perform some sanity checks perhaps?
    sendAll(this->sockfd, message);
    return true;
}

/**
 * Recieve the result of a query
 * @return std::vector<product> The list of products
 * If no products are found, one element will reside in the
 * vector. This product will have a number of zero
 */
std::vector<product> Client::recieveQuery() {
    // Check the number of bytes in the queue for errors
    checkConnection(this->sockfd);
    int elements = atoi((const char *) recv_line(this->sockfd).c_str());
    std::vector<product> games;
        while (elements-- > 0) {
            std::string s(recv_line(this->sockfd));
            games.push_back(stringToProduct(s));
        }
    return games;
}

/**
 * Close the connection with the server
 */ 
void Client::closeSocket() {
    shutdown(this->sockfd,1);
}
