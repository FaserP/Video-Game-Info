#include <sstream> // For stringstream
#include "product.hh"
#include "database.cc"

class Products {
public:
    int getNumRows();
    product getProduct(unsigned int index);
    std::vector<product> getAllProducts();
    std::vector<product> getProducts(const char *, const char *);
    void addProduct(product);
    void closeLink();
};
