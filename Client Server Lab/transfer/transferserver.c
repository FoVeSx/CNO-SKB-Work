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
#include <fcntl.h>

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
#define LISTENQ 5
#define SOCKET_ERROR -1

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  transferserver [options]\n"                                                \
"options:\n"                                                                  \
"  -f                  Filename (Default: bar.txt)\n"                         \
"  -h                  Show this help message\n"                              \
"  -p                  Port (Default: 8080)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"filename", required_argument, NULL, 'f'},
        {"port",     required_argument, NULL, 'p'},
        {"help",     no_argument,       NULL, 'h'},
        {NULL, 0,                       NULL, 0}
};

int open_listenfd(char *port);

/* Main ========================================================= */
int main(int argc, char **argv) {
    int option_char;
    int portno = 8080; /* port to listen on */
    char *filename = "bar.txt"; /* file to transfer */

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "f:p:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'f': // listen-port
                filename = optarg;
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

    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    /* Socket Code Here */

    typedef struct sockaddr SA;
    int listenfd, connfd, total_message_length, bytes_sent, n; // n to track where i am in file based on how many bytes sent
    char message[BUFSIZE];
    FILE *fp;

    // convert portno to string
    char *port = malloc(16);
    snprintf(port, 16, "%u", portno);

    listenfd = open_listenfd(port);

    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof clientaddr;

    /* send data till no more data to send, send in chunks, send a certain amount of bytes, and where we leave off we move the pointer */
    while (1)
    {
        // client connection
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);

        fp = fopen(filename, "r+"); //opens a text file for reading
        
        while(!feof(fp))
        {
            n = 0;
            total_message_length = 0;
            bytes_sent = 0;

            bzero(message, BUFSIZE); // clear previous message for set of next bytes to read and send
            total_message_length = fread(message, sizeof(char), 10, fp); // read 10 byte chunks, fread advances the pointer
            bytes_sent = send(connfd, message, total_message_length, 0); // not necessarily will send all the bytes read, so we need to store how many bytes are actually sent

            n = bytes_sent;

            if(bytes_sent < 1)
            {
                break;
            }
            else
            {
                while(n != total_message_length)
                {
                    bytes_sent = send(connfd, message + bytes_sent, total_message_length, 0);
                    n += bytes_sent;
                }
            }
        }

        fclose(fp);
        close(connfd);
    }
    
    close(listenfd);
    return 0;
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