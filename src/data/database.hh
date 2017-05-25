#include <sqlite3.h>
#include <vector>

class Database {
private:
    sqlite3 *connection;
public:
    Database();
    unsigned int getNumRows();
    std::vector<product> Query(const char *, std::vector<std::string>);
    void Close();
};
