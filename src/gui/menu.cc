#include "menu.hh"
/**
* The menu provides a visual interface for the program.
* It also uses the provided client object to communicate
* with the server
*/

Menu::Menu(Client client) {
	this->error = false; // Error flag initilisation
    this->count = client.getNumRows(); 
	/*
	 * The Menu tree
	 *
	 *	Add game information
	 *   	<follow onscreen guide>
	 *	Retrieve Game information
	 *       <Display all>
	 *           <Display individual via ID>
	 *       <Display individual via ID>
	 *       <Create Filter - select type>
	 *       	<Create Filter - enter query>
	 *				<Display filtered>
	 *					<Display individual via ID>
	 *  Exit program
     */
	bool loop = true; // Handles which menu is displayed
	while (loop) {
		printMenu(0); // Main menu

		std::string choice(getInput());
		
		/*
         * MENU SELECTION 
		 * Add game information
		 */
		if (choice == "1"){
            while (loop) {
                this->printMenu(9);

                // Name
                std::cout << "Name : ";
                std::string name;
                getline(std::cin, name);

                if (name == "0") break;

                // Genre
                std::cout << "Genre : ";
                std::string genre;
                getline(std::cin, genre);

                if (genre == "0") break;
        
                // Price
                std::cout << "Price : ";
                std::string price;
                getline(std::cin, price);

                if (price == "0") break;

                // Producer
                std::cout << "Producer : ";
                std::string producer;
                getline(std::cin, producer);

                if (producer == "0") break;
                
                // Description
                std::cout << "Description : ";
                std::string description;
                getline(std::cin, description);

                if (description == "0") break;

                while (loop) {
                    std::cout << "Submit game (y/n)\n";
    
                    char confirm;
                    std::cin >> confirm;
                    std::cin.ignore();
                    if (confirm == 'y') {
                        // Build the product struct
                        product game;
                        game.number = 0;
                        game.name = name;
                        game.type = genre;
                        game.price = price;
                        game.producer = producer;
                        game.description = description;

                        // Submit data
                        client.sendRequest((unsigned char *) "ADD PRODUCT");
                        // If the server is ready, send away!
                        if (client.checkStatusCode()) {
                            client.sendRequest((unsigned char *) productToString(game).c_str());
                            this->game = game;
                            printMenu(4);

                            choice = getInput();
                            if (choice == "0") {
                                loop = false;
                            }
                        // Some form of error, print a general message
                        } else {
                            this->error = true;
                        }

                    } else if (confirm == 'n') {
                        loop = false;
                    }

                }
            }
        loop = true;
		
		/*
		 * MENU SELECTION
		 * Retrieve game information
		 */
		} else if (choice == "2") {
			while (loop) {
				this->printMenu(2); // Retrieve type menu

				choice = getInput();
				
				// Exit the menu
				if (choice == "0") {
					loop = false;
				
				/*
				 * MENU SELECTION
				 * List all games
				 */
				} else if (choice == "1") {
					while (loop) {
					
						// Get the products from the server
						client.sendRequest((unsigned char *) "GET ALL");
						// If the server accepts, use recieveQuery
						if (client.checkStatusCode()) {
							this->products = client.recieveQuery();
						    this->printMenu(3); // Game listing menu
					    	
                            choice = getInput();
                            int choiceInt = atoi(choice.c_str());
			    			// Go back to retrieve type menu
		    				if (choice == "0") {
	    						loop = false;
							
    						/* 
						     * User selected a game. We already have the
					    	 * list of games so just reuse the products vector.
				    		 */
			    			} else if (choiceInt <= this->products.size()) {
		    					// Set the game for display by printMenu
	    						this->game = this->products.at(choiceInt - 1);
    							while (loop) {
								    this->printMenu(4); // Individual product menu
							    	choice = getInput();
						    		// Go back to game listing menu
					    			if (choice == "0") {
				    					loop = false;
			    					}
		    					}
	    						loop = true; // Loop again for game listings
    						}
						// non SUCCESS recieved, display error
						} else {
							this->error = true;
						}
					}
				loop = true; // Loop again for retrieve type menu
				choice = "0"; // Reset to prevent other if statements running this loop

				/*
				 * MENU SELECTION
				 * Retrieve a specific game
				 */
				} else if (choice == "2") {
                    bool main = true; // Allows user to go back to retrieve menu
					while (loop && main) {
						this->printMenu(5); // Selection prompt
						choice = getInput();
                        int choiceInt = atoi(choice.c_str());
						
						// Go back to retrieve type menu
						if (choice == "0") {
							loop = false;
							
						/*
						 * User entered a number, select it from the DB
						 */
						} else if (choiceInt > 0 && choiceInt <= this->count) {
							// Build the request
							std::stringstream ss;
							ss << "GET PRODUCT ";
							ss << choice << "\r";
							client.sendRequest((unsigned char *) ss.str().c_str());
							// If the server accepts, use recieveQuery
							if (client.checkStatusCode()) {
								this->game = client.recieveQuery().at(0);
								while (loop && main) {
									this->printMenu(4); // Individual product menu
									choice = getInput();
									// Go back to selection prompt
									if (choice == "0") {
										loop = false;
									} else if (choice == "1") {
                                        main = false;
                                    }
								}
								loop = true; // Loop again for selection prompt
								
							// non SUCCESS recieved, display error
							} else {
							this->error = true;
							}
                        // Number is less than zero
                        } else {
                        this->error = true;
                        }
					}
				loop = true; // Loop again for retrieve type menu
				choice = "0"; // Reset to prevent other if statements running this loop
				
				/*
				 * MENU SELECTION
				 * Retieve a list of filtered games
				 */
				} else if (choice == "3") {
                    bool main = true; // Allows user to go back to retreive menu
					while (loop && main) {
						this->printMenu(6); // Filter type menu

						choice = getInput();
						// Go back to retrieve type menu
						if (atoi(choice.c_str()) <= 0) {
							loop = false;
							
						/*
						 * User entered a choice
						 */
						} else if (atoi(choice.c_str()) <= 5) {
							// Set the filterType for later
							if (choice == "1") {
								this->filterType = "Name";
							} else if (choice == "2") {
								this->filterType = "Genre";
							} else if (choice == "3") {
								this->filterType = "Price";
							} else if (choice == "4") {
								this->filterType = "Producer";
							} else if (choice == "5") {
								this->filterType = "Description";
							}
							while (loop && main) {
								this->printMenu(7); // Enter query prompt

								// Get the whole line entered
								getline(std::cin, this->filterQuery);

								// Go back to filter type menu
								if (this->filterQuery == "0") {
									loop = false;
								
								/*
								 * A query was entered
								 */
								} else if (this->filterQuery.size() > 0) {
									// Build the request
									std::stringstream ss;
									ss << "GET FILTER \"";
									ss << this->filterType << '\"';
									ss << this->filterQuery << "\"\r";

									client.sendRequest((unsigned char *) ss.str().c_str());
									// If the server accepts, use recieveQuery
									if (client.checkStatusCode()) {
										this->products = client.recieveQuery();
										while (loop && main) {
											// Only one returned, display it
											if (this->products.size() == 1) {
												this->game = this->products.at(0);
												this->printMenu(4); // Individual product menu
											// Otherwise display the list
											} else {
												this->printMenu(8); // Filtered game listings
											}

											choice = getInput();
											// Go back to either query input or game listings
											if (choice == "0") {
												loop = false;
											
											// User entered a game index, display it
											} else if (atoi(choice.c_str()) <= this->products.size() &&
													this->products.size() > 1) {
												this->game = this->products.at(atoi(choice.c_str()) - 1);
												while (loop && main) {
													this->printMenu(4); // Individual product menu
													
													choice = getInput();
													// Go back to game listings
													if (choice == "0") {
														loop = false;
                                                    // Go back to the main retrieve menu
													} else if (choice == "1") {
                                                        main = false;
                                                    }
												} // End Individual game while loop
											loop = true;
											}
										} // End filter result while loop
									loop = true;
									// non SUCCESS recieved, display error
									} else {
									this->error = true;
									}
								}
							} // End enter query while loop
						loop = true;
						}
					} // End select type while loop 
				loop = true;
				choice = "0";
				}
			}
			// Allow the main loop to contine
			loop = true;
		} else if (choice == "3") {
			// Exit the program
			loop = false;
		} else {
			// Do nothing
		}
	}
}

void Menu::printMenu(int screen) {
	std::system("clear");
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	switch(screen) {
	case 0 :
		ss << "Add or Retrieve game information\n";
		ss << "Please select the item you wish to choose by\n";
		ss << "entering it's number and pressing ENTER\n\n";
		ss << "*****************************************\n";
		ss << "* Select your request                   *\n";
		ss << "* -----------------------               *\n";
		ss << "* 1. Add game information               *\n";
		ss << "* 2. Retrieve game information          *\n";
		ss << "* 3. Exit the program                   *\n";
		ss << "*****************************************\n";
    if (this->error) {
        ss << "* Previous request failed! Sorry :<     *\n";
        ss << "*****************************************\n";
    }
        ss << ">";
		break;
	case 2:
		ss << "Retrieve game information\n";
		ss << "Select which method of retrieval you want to use\n";
		ss << "Enter zero (0) to go back\n";
		ss << "******************************************\n";
		ss << "* Select your request                    *\n";
		ss << "* ----------------------                 *\n";
		ss << "* 1. List all games by id and title      *\n";
        ss << "*    title.                              *\n";
		ss << "* 2. Enter game ID                       *\n";
		ss << "* 3. Apply a filter and list the results *\n";
		ss << "******************************************\n";
    if (this->error) {
        ss << "* Previous request returned error        *\n";
        ss << "******************************************\n";
    }
        ss << ">";
		break;
	case 3 :
		ss << "Listing all games\n";
		ss << "Select which game you wish to view\n";
		ss << "Enter zero (0) to go back\n";
		ss << "******************************************\n";
	for (int i = 1; i <= this->products.size(); i++) {
		Product prod = this->products.at(i - 1);
		ss << "* " << i << ") " << prod.name << "\n";
	}
		ss << "*****************************************\n";
        ss << ">";

		break;
	case 4:
		ss << "Listing individual game\n";
        ss << "Enter zero (0) to go back one menu\n";
		ss << "Enter one (1) to go back to the main submenu\n";
		ss << "************************************\n";
		ss << "* Number      : " << this->game.number << "\n";
		ss << "* Name        : " << this->game.name << "\n";
		ss << "* Genre       : " << this->game.type << "\n";
		ss << "* Price       : " << this->game.price << "\n";
		ss << "* Producer    : " << this->game.producer << "\n";
		ss << "* Description : " << this->game.description << "\n";
		ss << "************************************\n";
        ss << ">";
		break;
	case 5 :
		ss << "Select an individual game\n";
		ss << "Enter zero (0) to go back\n";
		ss << "************************************\n";
		ss << "* Enter the game number to view    *\n";
		ss << "************************************\n";
    if (this->error) {
        ss << "* Previous request returned error  *\n";
        ss << "************************************\n";
    }
        ss << ">";
		break;
	case 6 :
		ss << "Apply a filter and list the results\n";
		ss << "Enter the type of filter you wish to use\n";
		ss << "Enter zero (0) to go back\n";
		ss << "************************************\n";
		ss << "* 1. Name        2. Genre          *\n";
		ss << "* 3. Price       4. Producer       *\n";
		ss << "* 5. Description                   *\n";
		ss << "************************************\n";
        ss << ">";
		break;
	case 7 :
		ss << "Apply a filter and list the results\n";
		ss << "Press zero (0) to go back\n";
		ss << "Enter the query you wish to filter by\n";
		ss << "Filtering by : " << this->filterType << "\n";
		ss << "************************************\n";
    if (this->error) {
        ss << "* Filter returned no results!      *\n";
        ss << "************************************\n";
    }
		ss << "Query : ";
		break;
	case 8 :
		ss << "Apply a filter and list the results\n";
		ss << "Press zero (0) to go back\n";
		ss << "Filtering by    : " << this->filterType << "\n";
		ss << "Filtering query : " << this->filterQuery << "\n";
		ss << "***********************************\n";
	for (int i = 1; i <= this->products.size(); i++) {
		Product prod = this->products.at(i - 1);
		ss << "* " << i << ") " << prod.name << "\n";
	}
        ss << ">";
        break;
    case 9 :
        ss << "Add game information\n";
        ss << "Follow the on screen instructions\n";
        ss << "Press zero (0) to go back once\n";
        ss << "Press one (1) to go back to main menu\n";
        ss << "***********************************\n";
        break;

	}
	std::cout << ss.str().c_str();
    this->error = false; // Reset the error bool
}
