#include <iostream>
#include "products.hh"


/**
 * Get a single product from the database
 */
product Products::getProduct(unsigned int index) {
    std::stringstream ss;
    ss << "SELECT * FROM games WHERE id = ?";
    std::string query(ss.str());
    ss.str(""); // Clear stream

    ss << index; // Insert the index into the string. Easy type  conversion
    
    std::vector<std::string> params;
    params.push_back(ss.str());

    Database database;
    // The returned result is a vector of products.
    // We are only asking for one and only one
    // product should exist as we are accessing by
    // primary key so just use at(0)
    product prod = database.Query(query.c_str(), params).at(0); 
    database.Close();
    return prod;
}

/**
 * Get a collection of products based on a filter
 */
std::vector<product> Products::getProducts(const char * type, const char * query) {
    std::vector<std::string> params;
    params.push_back(type);
    
    std::stringstream ss;
    ss << "SELECT * FROM games";
    ss << " WHERE " << type << " LIKE '%" << query << "%'";

    Database database;
    std::vector<product> products;
    products = database.Query(ss.str().c_str(), params);
    database.Close();
    return products;
}

/**
 * Gets all products from the database
 */
std::vector<product> Products::getAllProducts() {
    std::stringstream ss;
    ss << "SELECT * FROM games";
    std::vector<std::string> params;
    Database database;
    std::vector<product> products;
    products = database.Query(ss.str().c_str(), params);
    database.Close();
    return products;
}

int Products::getNumRows() {
    Database database;
    int res = database.getNumRows();
    database.Close();
    return res;
}

void Products::addProduct(product game) {
    Database database;
    std::stringstream ss;
    ss << "INSERT INTO games (name, genre, price, producer, description) VALUES ";
    ss << "(?, ?, ?, ?, ?)";

    std::vector<std::string> params;
    params.push_back(game.name.c_str());
    params.push_back(game.type.c_str());
    params.push_back(game.price.c_str());
    params.push_back(game.producer.c_str());
    params.push_back(game.description.c_str());
    database.Query(ss.str().c_str(), params);
}


/**
 * Unused code. scheduled for removal
 */
void Products::closeLink() {
    //this->database.Close();
    //products = NULL;
}
