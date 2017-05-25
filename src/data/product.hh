#include <string>

struct Product {
    unsigned int number;
    std::string name;
    std::string type;
    std::string price;
    std::string producer;
    std::string description;
};

typedef struct Product product;
