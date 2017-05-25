#include <unistd.h> // for getopt
#include "src/system/server.cc"
char *name;
char *port;

void printUsage() {
    printf("Usage: %s -p <port number>\n", name);
    exit(1);
}

/*
 * Start the product server
 * Supply a port number as the program argument
 */
int main(int argc, char *argv[]) {
    name = argv[0]; // for printUsage()

    // Parse the arguments
    int flag;
    opterr = 0;
    while ((flag = getopt(argc, argv, "p:")) != -1) {
        switch (flag) {
            case 'p' : 
                // Check the supplied port contains only numbers    
                for (int i = 0; i < strlen(optarg); i++) {
                    if (!isdigit(optarg[i])) {
                        printUsage();
                    }
                }
                port = optarg;
                break;
            default:
                printUsage();
            
        }
    }

    if (port == NULL) {
        printUsage();
    }
    
    //Start the server
    Server server(atoi(port));
}
