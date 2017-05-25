#include <arpa/inet.h> // fpr inet_aton
#include <unistd.h> // for getopt

#include "src/system/client.cc"
#include "src/gui/menu.cc"
char *name;
char *ip;
char *port;

/*
 * Print a general usage error in case
 * of any problems with parameters
 */
void printUsage() {    
    printf("Usage: %s -h <hostname> -p <port>\nThe hostname is not required if connecting to localhost\n", name);
        exit(1);
    }

/*
 * Setup the connection to the server
 */
int main(int argc, char *argv[]) {
    name = argv[0]; // for printUsage()

    // Parse the arguments
    int flag;
    opterr = 0;
    while ((flag = getopt(argc, argv, "p:h:")) != -1) {
        switch (flag) {
            case 'h' :
                // Get the server ip address
                struct in_addr server_addr;
                if(inet_aton(optarg, &server_addr) == 0) {
                    printf("%s is not a valid address\n", optarg);
                    printUsage();
                } else {
                    ip = optarg;
                }
                break;
            case 'p' :
                // Check the supplied port contains only numbers
                for (int i = 0; i < strlen(optarg); i++) {
                    if (!isdigit(optarg[i])) {
                        printf("Port needs to be a number\n");
                        printUsage();
                    }
                }
                port = optarg;
                break;
            default:
                printUsage();
        }
    }
    
    if (ip == NULL) {
        ip = (char *) "127.0.0.1";
    }
    
    if (port == NULL) {
        printUsage();
    }
   
    Client client(ip, port);
    Menu menu(client);
}
