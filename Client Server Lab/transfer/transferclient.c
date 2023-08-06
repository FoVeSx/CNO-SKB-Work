#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFSIZE 4096
#define SOCKET_ERROR -1

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  transferclient [options]\n"                                                \
"options:\n"                                                                  \
"  -h                  Show this help message\n"                              \
"  -o                  Output file (Default foo.txt)\n"                       \
"  -p                  Port (Default: 8080)\n"                                \
"  -s                  Server (Default: localhost)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
        {"server", required_argument, NULL, 's'},
        {"port",   required_argument, NULL, 'p'},
        {"output", required_argument, NULL, 'o'},
        {"help",   no_argument,       NULL, 'h'},
        {NULL, 0,                     NULL, 0}
};

int open_sockfd(char *host, char *port);

/* Main ========================================================= */
int main(int argc, char **argv) {
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 8080;
    char *filename = "foo.txt";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:o:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 's': // server
                hostname = optarg;
                break;
            case 'p': // listen-port
                portno = atoi(optarg);
                break;
            case 'o': // filename
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

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }


    /* Socket Code Here */
    int sockfd, byte_count;
    char buffer[BUFSIZE]; //store received data into this buffer
    FILE *fp;

    // convert portno to string
    char *port = malloc(16);
    snprintf(port, 16, "%u", portno);

    // created this to utilize the getaddrinfo api
    sockfd = open_sockfd(hostname, port);

    // receive data from server and write it to a file
    fp = fopen(filename, "a+"); //opens a text file for both reading and writing (per instructions) in appending mode, creates file if it does not exist

    // continue to receive data until no more data to recv
    while(1)
    {
        bzero(buffer, BUFSIZE);
        byte_count = recv(sockfd, buffer, BUFSIZE, 0);
        
        //printf("Bytes received: %d \n", byte_count);

        // error out if byte_count < 1
        if(byte_count < 1){
            break;
        }
        else{
            fwrite(buffer, sizeof(char), byte_count, fp);
        }
    }

    fclose(fp);
    close(sockfd);
    exit(0);
}

int open_sockfd(char *host, char *port)
{
    int sockfd;

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
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != SOCKET_ERROR)
            break;
        
        close(sockfd);
    }

    freeaddrinfo(servinfo);
    if (!p)
        return SOCKET_ERROR;
    else
        return sockfd; 
}