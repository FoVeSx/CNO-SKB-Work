#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h> 
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <getopt.h>
#include "gfclient.h"

#define BUFSIZE 4096

const char *request_template = "GETFILE GET %s\r\n\r\n";
const char *request_end_str = "\r\n\r\n";

// functions created in this file (not used in header file)
int create_socket(char *hostname, unsigned short portno);
int open_sockfd(char *host, char *port);
int send_request(gfcrequest_t *gfr, int socket);
int parse_header(gfcrequest_t *gfr, int socket);

// define the gfcrequest struct
struct gfcrequest_t
{
    // set_path, set_port, set_server
    char *server;
    unsigned short port;
    char *path;

    // get_bytesreceived, get_filelen, get_status
    size_t bytesreceived;
    size_t filelen;
    gfstatus_t status;

    // set_headerarg, set_headerfunc, set_writearg, set_writefunc
    void *headerarg;
    void *writearg;
    void (*headerfunc)(void *, size_t, void *);
    void (*writefunc)(void *, size_t, void *);
};

void gfc_cleanup(gfcrequest_t *gfr)
{
    assert(gfr != NULL);
    free(gfr);
    gfr = NULL;
}

gfcrequest_t *gfc_create()
{
    gfcrequest_t *result = malloc(sizeof(gfcrequest_t));
    bzero(result, sizeof(gfcrequest_t));
    return result;
}

size_t gfc_get_bytesreceived(gfcrequest_t *gfr)
{
    assert(gfr != NULL);
    return gfr->bytesreceived;
}

size_t gfc_get_filelen(gfcrequest_t *gfr)
{
    assert(gfr != NULL);
    return gfr->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t *gfr)
{
    assert(gfr != NULL);
    return gfr->status;
}

void gfc_global_init(){
}

void gfc_global_cleanup(){
}

int gfc_perform(gfcrequest_t *gfr)
{
    int sockfd_gfr = create_socket(gfr->server, gfr->port);

    if (sockfd_gfr < 0)
    {
        perror("Cannot establish connection.");
        gfr->status = GF_ERROR;
        close(sockfd_gfr);
        return -1;
    }

    if ((send_request(gfr, sockfd_gfr)) < 0 )
    {
        perror("Cannot send request to server.");
        gfr->status = GF_ERROR;
        close(sockfd_gfr);
        return -1;
    }

    return 0;
}

void gfc_set_headerarg(gfcrequest_t *gfr, void *headerarg)
{
    assert(gfr != NULL);
    gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t *gfr, void (*headerfunc)(void*, size_t, void *))
{
    assert(gfr != NULL);
    gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t *gfr, char* path)
{
    assert(gfr != NULL);
    gfr->path = path;
}

void gfc_set_port(gfcrequest_t *gfr, unsigned short port)
{
    assert(gfr != NULL);
    gfr->port = port;
}

void gfc_set_server(gfcrequest_t *gfr, char* server)
{
    assert(gfr != NULL);
    gfr->server = server;
}

void gfc_set_writearg(gfcrequest_t *gfr, void *writearg)
{
    assert(gfr != NULL);
    gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t *gfr, void (*writefunc)(void*, size_t, void *))
{
    assert(gfr != NULL);
    gfr->writefunc = writefunc;
}

char* gfc_strstatus(gfstatus_t status)
{
    char *strstatus = NULL;

    // could use switch but whatever
    if (status == GF_OK) 
    {
        strstatus = "OK";
    } 
    else if (status == GF_FILE_NOT_FOUND) 
    {
        strstatus = "FILE_NOT_FOUND";
    }
    else if (status == GF_INVALID) 
    {
        strstatus = "INVALID";
    }
    else if (status == GF_ERROR) 
    {
        strstatus = "ERROR";
    } 
    else 
    {
        strstatus = "UNKNOWN";
    }

    return strstatus;
}

int create_socket(char *hostname, unsigned short portno)
{
    int sockfd;

    // convert portno to string
    char *port = malloc(16);
    snprintf(port, 16, "%u", portno);

    // created this to utilize the getaddrinfo api
    sockfd = open_sockfd(hostname, port);

    return sockfd;
}

int open_sockfd(char *host, char *port)
{
    int sockfd;

    struct addrinfo hints;
    struct addrinfo *servinfo, *p; // points to results linkedlist

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //spit out list of ip addresses resolved by host
    getaddrinfo(host, port, &hints, &servinfo);

    // walk the list
    for (p = servinfo; p; p = p->ai_next)
    {
        /* create a socket descriptor*/
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        
        close(sockfd);
    }

    freeaddrinfo(servinfo);
    if (!p)
        return -1;
    else
        return sockfd; 
}

int send_request(gfcrequest_t *gfr, int sockfd)
{
    size_t request_length = 0;
    size_t bytes_sent = 0;
    size_t n = 0;

    char request[BUFSIZE];
    bzero(request, BUFSIZE);
    sprintf(request, request_template, gfr->path); //GETFILE GET %s\r\n\r\n

    request_length = strlen(request);

    while(n < request_length) // not include null terminating character; avoid using != 
    {
        bytes_sent = send(sockfd, request + n, request_length, 0);
        n += bytes_sent;
    }

    return 0;
}

/* 
Receive Header - GETFILE OK 107528\r\n\r\n
When the status is OK, the length should be a number expressed in ASCII (what sprintf will give you)
The sequence ‘\r\n\r\n’ marks the end of the header. All remaining bytes are the files contents.
No content may be sent if the status is FILE_NOT_FOUND or ERROR.
*/
int parse_header(gfcrequest_t *gfr, int sockfd)
{
    size_t byte_count = 0;
    char temp_buffer[BUFSIZE];
    char total_buffer[BUFSIZE * 2];

    bzero(temp_buffer, BUFSIZE);
    bzero(total_buffer, BUFSIZE * 2);

    // grab entire header until end sequence is seen
    do
    {
        byte_count = recv(sockfd, temp_buffer, BUFSIZE, 0);

        if (byte_count == 0) {
            gfr->status = GF_INVALID;
            return -1;
        }

        if (byte_count < 0) {
            gfr->status = GF_ERROR;
            return -1;
        }

        strcat(total_buffer, temp_buffer);
        bzero(temp_buffer, BUFSIZE);
    } while ({/* condition */});

}
