#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if 0
/*
 * Structs exported from netinet/in.h (for easy reference)
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

#define BUFSIZE 4096
#define MSGSIZE 16
#define LISTENQ 5
#define SOCKET_ERROR -1

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -n                  Maximum pending connections\n"                         \
"  -p                  Port (Default: 8080)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"port",        required_argument, NULL, 'p'},
        {"maxnpending", required_argument, NULL, 'n'},
        {"help",        no_argument,       NULL, 'h'},
        {NULL, 0,                          NULL, 0}
};

int open_listenfd(char *port);

int main(int argc, char **argv) {
    int option_char;
    int portno = 8080; /* port to listen on */
    int maxnpending = 5;

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "p:n:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'n': // server
                maxnpending = atoi(optarg);
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

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (maxnpending < 1) {
        fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
        exit(1);
    }


    /* Socket Code Here */
    
    typedef struct sockaddr SA;
    int listenfd, connfd;
    char buffer[MSGSIZE];

    // convert portno to string
    char *port = malloc(MSGSIZE);
    snprintf(port, MSGSIZE, "%u", portno);

    listenfd = open_listenfd(port);

    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof clientaddr;
    while (1)
    {
        // client connection
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        
        // actual echo portion
        bzero(buffer, MSGSIZE);

        if((read(connfd, buffer, MSGSIZE)) == SOCKET_ERROR)
            exit(0);

        printf("%s", buffer);

        if((write(connfd, buffer, strlen(buffer))) == SOCKET_ERROR)
            exit(0);

        close(connfd);
    }

    exit(0);
}

int open_listenfd(char *port)
{
    int listenfd, optval = 1;

    struct addrinfo hints;
    struct addrinfo *servinfo, *p; // points to results linkedlist

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // specifying only ipv4 populated in linkedlist
    hints.ai_socktype = SOCK_STREAM; // 
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // on any ip addr
    hints.ai_flags |= AI_NUMERICSERV; // using port no.

    //spit out list of ip addresses resolved by host
    getaddrinfo(NULL, port, &hints, &servinfo);

    // walk the list
    for (p = servinfo; p; p = p->ai_next)
    {
        /* create a socket descriptor*/
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        // obscure, frees up socket, less delay
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
        
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        
        close(listenfd);
    }

    freeaddrinfo(servinfo);
    if (!p)
        return -1;
    
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }

    return listenfd;
}