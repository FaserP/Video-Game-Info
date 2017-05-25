#include "database.hh"

/**
 * Open a database connection
 */
Database::Database() {
    int result;
    result = sqlite3_open("products.db", &this->connection);
    if (result > 0) {
        // Error, print a message and close the server.
        printf("Cannot open products database : %s\n" , sqlite3_errmsg(this->connection));
        sqlite3_close(this->connection);
        exit(1);
    }
}

/**
 * Issue a query on the products table and return a
 * vector of products
 * @param sql The SQL query to run. Prepared statements not yet supported
 * @return std::vector<product> A vector containing all the rows returned
 * If no rows are returned, a single product will exist in the vector
 * with the field "number" set to zero. A zero in the number is an
 * illegal number so use this for error checking
 */
std::vector<product> Database::Query(const char *sql, std::vector<std::string> params) {
    sqlite3_stmt *stmt;
    int result;
    std::vector<product> products;
    result = sqlite3_prepare_v2(this->connection, sql, -1, &stmt, NULL);
    // Error, print a message and close the server.
    if (result > 0) {
        printf("Cannot prepare statement : %s\n", sqlite3_errmsg(this->connection));
        sqlite3_close(this->connection);
        exit(1);
    }

    // Bind any values
        for (int i = 0; i < params.size(); i++) {
            sqlite3_bind_text(stmt, i + 1, params.at(i).c_str(), -2, SQLITE_STATIC); 
        }

        // For each row found, insert the rows into a struct
        for (int i = 0; sqlite3_step(stmt) == SQLITE_ROW; i++) {
            product game;
            game.number = sqlite3_column_int(stmt, 0);
            game.name = (const char *) sqlite3_column_text(stmt, 1);
            game.type = (const char *) sqlite3_column_text(stmt, 2);
            game.price = (const char *) sqlite3_column_text(stmt, 3);
            game.producer = (const char *) sqlite3_column_text(stmt, 4);
            game.description = (const char *) sqlite3_column_text(stmt, 5);
            products.push_back(game);
        }

    result = sqlite3_finalize(stmt);
    if (result > 0) {
        printf("Error closing the prepared statment : %s\n", sqlite3_errmsg(this->connection));
        sqlite3_close(this->connection);
        exit(1);
    }
    
    // Empty vector so add a product with number of zero
    if (products.size() == 0) {
        product game;
        game.number = 0;
        products.push_back(game);
    }
    return products;
}

unsigned int Database::getNumRows() {
    std::string sql("SELECT COUNT(*) FROM games");
    sqlite3_stmt *stmt;
    int result;
    result = sqlite3_prepare_v2(this->connection, sql.c_str(), -1, &stmt, NULL);
    // Error, print a message and close the server.
    if (result > 0) {
        printf("Cannot prepare statement : %s\n", sqlite3_errmsg(this->connection));
        sqlite3_close(this->connection);
        exit(1);
    }
    unsigned int res;
    for (int i = 0; sqlite3_step(stmt) == SQLITE_ROW; i++) {
        res = sqlite3_column_int(stmt, 0);
    }
    
    result = sqlite3_finalize(stmt);
    if (result > 0) {
        printf("Error closing the prepared statment : %s\n", sqlite3_errmsg(this->connection));
        sqlite3_close(this->connection);
        exit(1);
    }
    
    return res;
}

/**
 * Close the database connection assocated with this object
 */
void Database::Close() {
    sqlite3_close(this->connection);
}
