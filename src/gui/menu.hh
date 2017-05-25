#include <stdio.h>
#include <stdlib.h> //itoa function
#include <string.h> //strcmp
#include <sstream> // Stringstream
#include <iostream> //getline() function

class Menu {

public:
    int screen;
    Menu(Client);
private:
    std::vector<product> products; // List of products
    product game; // Single product
    std::string filterType; // For custom filter
    std::string filterQuery; // For filter
    int count; // Number of games in database
    bool error; // Error flag
    void printMenu(int);
};
