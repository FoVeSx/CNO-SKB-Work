#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>

/* Be prepared accept a response of this length */
#define BUFSIZE 4096
#define MSGSIZE 16
#define SOCKET_ERROR -1

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoclient [options]\n"                                                    \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -m                  Message to send to server (Default: \"Hello World!\"\n"\
"  -p                  Port (Default: 8080)\n"                                \
"  -s                  Server (Default: localhost)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"server",  required_argument, NULL, 's'},
        {"port",    required_argument, NULL, 'p'},
        {"message", required_argument, NULL, 'm'},
        {"help",    no_argument,       NULL, 'h'},
        {NULL, 0,                      NULL, 0}
};


int open_clientfd(char *host, char *port);

/* Main ========================================================= */
int main(int argc, char **argv) {
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 8080;
    char *message = "Hello World!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 's': // server
                hostname = optarg;
                break;
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'm': // server
                message = optarg;
                break;
            case 'h': // help
                fprintf(stdout, "%s", USAGE);
                exit(0);
                break;
            default:
                fprintf(stderr, "%s", USAGE);
                exit(1);
        }
    }

    if (NULL == message) {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */
    int clientfd;
    int status;
    char buffer[MSGSIZE];

    // convert portno to string
    char *port = malloc(MSGSIZE);
    snprintf(port, MSGSIZE, "%u", portno);

    // created this to utilize the getaddrinfo api
    clientfd = open_clientfd(hostname, port);

    if((status = write(clientfd, message, strlen(message))) == SOCKET_ERROR)
        exit(0);
    
    bzero(buffer, MSGSIZE);
    if((status = read(clientfd, buffer, MSGSIZE)) == SOCKET_ERROR)
        exit(0);

    printf("%s", buffer);

    close(clientfd);
    exit(0);
}

int open_clientfd(char *host, char *port)
{
    int clientfd;

    struct addrinfo hints;
    struct addrinfo *servinfo, *p; // points to results linkedlist

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // specifying only ipv4 populated in linkedlist
    hints.ai_socktype = SOCK_STREAM; // tcp connection
    hints.ai_flags = AI_NUMERICSERV; // using numeric port arg
    hints.ai_flags |= AI_ADDRCONFIG; // recommended for connections

    //spit out list of ip addresses resolved by host
    getaddrinfo(host, port, &hints, &servinfo);

    // walk the list
    for (p = servinfo; p; p = p->ai_next)
    {
        /* create a socket descriptor*/
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != SOCKET_ERROR)
            break;
        
        close(clientfd);
    }

    freeaddrinfo(servinfo);
    if (!p)
        return SOCKET_ERROR;
    else
        return clientfd; 
}