#include "server.hh"

Server::Server(int port) {
	int yes = 1; // For socket option SO_REUSEADDR
	int result; // Just holds the return code from function calls
	struct sockaddr_in server_addr;
	// Setup the listen socket
	this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (this->sockfd < 0) {
	        error("Error opening socket");
        }

	// Allow for the re-binding of the listen socket
	result = setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (result < 0) {
			error("Error seting socket option SO_REUSEADDR");
        }

	// Set the listen socket to non-blocking
	setNonBlocking(this->sockfd);
	
	//Generate the server_addr struct
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_UNSPEC; // ipv4 or 6
	server_addr.sin_addr.s_addr = htons(INADDR_ANY); // Any local inteface
	server_addr.sin_port =htons(port);
	
	result = bind(this->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

		if (result < 0) {
			close(this->sockfd);
			error("Error binding listen socket");
		}
	
	// Listen for incoming connections
	result = listen(this->sockfd, QUEUE_SIZE);
		if (result < 0) {
			error("Error listening on listen socket");
        }
	
	// Only one socket exists so far (the listen socket).
	// It it currently the highest
	this->highSock = this->sockfd;
    
    // Clear the connectList
	memset((char *) &this->connectList, 0, sizeof(this->connectList));
   
    // Timeout for the select method 
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	// Main Loop
	while(true) {
		buildFDSet();
		result = select(this->highSock+1, &this->socks, NULL, NULL, &timeout);
		if(result < 0) { 
			error("Error while setting up select()");
        }
			
		if (result > 0) {
			// The socks fd_set is now set with sockets
			// that are ready for reading.
			// If the listen socket is ready that indicates
			// we have a new connection
			if (FD_ISSET(this->sockfd, &this->socks)) {
				addNewConnection();
            }
		
            // Process the rest of the connections	
			for (int i = 0; i < QUEUE_SIZE; i++) {
				if (FD_ISSET(this->connectList[i], &this->socks)) {
					handleConnection(i);
                }
			}
		}
	}
}

/**
 * Set a socket to be non blocking
 * Read/write calls to the socket will not
 * block the execution of the program
 */
void Server::setNonBlocking(int sock) {
    int opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        error("Error getting socket flags (fcntl)");
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        error("Error setting socket flag (fcntl)");
    }
}

/**
 * Build the socket set using the connectList
 * The FDSet gets cleared on each iteration of the
 * infinite while loop
 */
void Server::buildFDSet() {
	FD_ZERO(&this->socks);
	FD_SET(this->sockfd, &this->socks);
	
	// If there is fd in the connectList, add it to the fd_list
	for (int i = 0; i < QUEUE_SIZE; i++) {
        //Found something
		if (this->connectList[i] != 0) {
            // Add it to the socks set
			FD_SET(this->connectList[i], &this->socks);
            // If the added socket is 'higher' than the previous
            // high socket, just set the new socket to highSock
			if (this->connectList[i] > this->highSock) {
				this->highSock = this->connectList[i];	
            }
		}
	}
}

/**
 * The returned FD_SET shows the listen socket is ready to read
 * This indicates that a new client is wanting to connect
 */
void Server::addNewConnection() {
    // Only generate a sockaddr_in stuct so we can print the client's
    // IP and port
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
	int client = accept(this->sockfd, (struct sockaddr *) &addr, &addr_size);
    printf("New connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	if (client < 0) {
		error("Error opening client socket");
    }
    
	setNonBlocking(client);

    // See if there is any space left in the connectList	
	for (int i = 0; (i < QUEUE_SIZE) && (client != -1); i++) {
		// Found an empty spot for the socket
		if (this->connectList[i] == 0) {
			this->connectList[i] = client;
            // Send the 200 status code
            sendAll(client, (unsigned char *) SUCCESS);
			client = -1;
        }
	}
    
    // An empty spot cannot be found. Send the 503 code
    // to the client and close the socket	
	if (client != -1) {
		sendAll(client, (unsigned char *) UNAVAILABLE);
		close(client);
	}
}

/**
 * A socket wishes to communicate with the server
 * Handle this request
 * index is the index into the connectList array
 */
void Server::handleConnection(int index) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    getpeername(this->connectList[index], (struct sockaddr *) &addr, &addr_size);
    unsigned char buff[100];
    // Use MSG_PEEK so the data isn't removed from the socket queue
    // I want to see the incoming number of bytes only. If zero that
    // means a client has disconnected
    int bytes = recv(this->connectList[index], buff, 100, MSG_PEEK);
    if (bytes == 0) {
        // Print a message server side and close the socket
        printf("%s:%d hung up\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        
		close(this->connectList[index]);
        // Remove the sock from the connectList
		this->connectList[index] = 0;
	} else {
    // A request of some description has been
    // sent to the server. Find out what request
    // it was a serve it
    std::string request(recv_line(this->connectList[index]));
            
        if (request.compare(0, 9, "GET COUNT") == 0) {
        // A GET COUNT request. Return the number of games
        std::cout << inet_ntoa(addr.sin_addr) << ":" <<  ntohs(addr.sin_port) << " request : GET COUNT\n";
        sendAll(this->connectList[index], (unsigned char *) SUCCESS);
        Products dblink;
        int res = dblink.getNumRows();
        std::stringstream ss;
        ss << res << "\r";
        sendAll(this->connectList[index], (unsigned char *) ss.str().c_str());
        } else if (request.compare(0, 11, "ADD PRODUCT") == 0) {
        // An ADD PRODUCT request. The client wishes to add a product to the database
        std::cout << inet_ntoa(addr.sin_addr) << ":" <<  ntohs(addr.sin_port) << " request : ADD PRODUCT\n";
        sendAll(this->connectList[index], (unsigned char *) SUCCESS);
        // recieve the seralised product
        std::string gameString(recv_line(this->connectList[index]));
        product game = stringToProduct(gameString);
        Products dblink;
        dblink.addProduct(game);

        
        } else if (request.compare(0, 7, "GET ALL") == 0) {
        // A GET ALL request. We should return all games
        // Print a message server side for debugging
        std::cout << inet_ntoa(addr.sin_addr) << ":" <<  ntohs(addr.sin_port) << " request : GET ALL\n";
        sendAll(this->connectList[index], (unsigned char *) SUCCESS);
            
        // Retrieve all the products from the database
        Products dblink;
        std::vector<product> games = dblink.getAllProducts();
        // Send the number of elements in the vector
        std::stringstream ss;
        ss << games.size() << "\r"; // Apend EOL chars (only \r is needed?)
        sendAll(this->connectList[index], ss.str().c_str());
            // Send the elements one at a time
            for (int i = 0; i < games.size(); i++) {
                sendAll(this->connectList[index], productToString(games.at(i)).c_str());
            }


        } else if (request.compare(0, 11, "GET PRODUCT") == 0) {
        // A GET PRODUCT request. Requesting an individual product so
        // make sure the request contains a number (ID of the product)
        int end = request.find_first_of('\r', 12); // 12 should be after GET PRODUCT
        std::string s(request.substr(12, end - 12));

        bool digit = true;
            // Make sure the string contains only digits
            for (int i = 0; i < s.size();  i++) {
                if (!isdigit(s[i])) {
                    digit = false;
                }
            }
            if (digit) {
            // Does only contain digits, get the game!
            Products dblink;
            product game = dblink.getProduct(atoi(s.c_str()));
                if (game.number == 0) {
                // Returned game has a number of zero, this
                // is an ilegal value and therefore the game
                // could not be found. Send the NOT_FOUND
                // status
                sendAll(this->connectList[index], (unsigned char *) NOT_FOUND);
                } else {
                // Returned game contains a legal number,
                // send the SUCESS status and then send
                // the number of elements (1) and then
                // the serialised version of the game
                std::cout << inet_ntoa(addr.sin_addr) << ":"
                << ntohs(addr.sin_port)
                << " request : GET PRODUCT "
                << s << "\n";
                sendAll(this->connectList[index], (unsigned char *) SUCCESS);
                sendAll(this->connectList[index], (unsigned char *) "1\r");
                sendAll(this->connectList[index], productToString(game).c_str());
                }
            } else {
            // Request did not contain a number, send the
            // FORBIDDEN status
            sendAll(this->connectList[index], (unsigned char *) FORBIDDEN);
            }


        } else if (request.compare(0, 10, "GET FILTER") == 0) {
        // A GET FILTER request. Requesting a custom search of products
        // We need the filter type and query first
        // Get the filter type
        int start, end;
        start = 12;
        end = request.find_first_of('"', start);
        std::string type(request.substr(start, end - start));
          
        // Get the filter query
        start = ++end;
        end = request.find_first_of('"', start);
        std::string query(request.substr(start, end - start));

        Products dblink;
        // Get the products
        std::vector<product> games = dblink.getProducts(type.c_str(), query.c_str());
            if (games.at(0).number == 0) {
            // No games found
            sendAll(this->connectList[index], (unsigned char *) NOT_FOUND);
            } else {
            // Games were found!  Send the SUCCESS status,
            // then the number of elements and finaly the
            // serialised version of the game one at the time
            std::cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port)
            << " request : GET FILTER \"" << type << "\""
            << query << "\"\n";
            sendAll(this->connectList[index], (unsigned char *) SUCCESS);
            std::stringstream ss;
            ss << games.size() << '\r';
            sendAll(this->connectList[index], ss.str().c_str());
                for (int i = 0; i < games.size(); i++) {
                sendAll(this->connectList[index], productToString(games.at(i)).c_str());
                }
            }
        } else {
        // Nothing matched therefore the server doesn't know what to do
        sendAll(this->connectList[index], (unsigned char *) FORBIDDEN);
        } // End request parse
    } // End bytes == 0 check
} // End Function
