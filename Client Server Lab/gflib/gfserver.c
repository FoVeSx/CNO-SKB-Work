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
#include "gfserver.h"

/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */

#define BUFSIZE 4096

static const char *ok_header_template = "%s %s %zd %s"; // OK
static const char *invalid_header_template = "%s %s %s"; // FILE_NOT_FOUND OR ERROR
const char *gfheader_end_string = "\r\n\r\n";

const char *gfschema_string = "GETFILE";
const char *gfok_string = "OK";
const char *gffnf_string = "FILE_NOT_FOUND";
const char *gferror_string = "ERROR";


int open_listenfd(gfserver_t *gfs, char *port); // uses getaddrinfo api that i used in transfer for the listen portion of the server
ssize_t send_data(int socket, char *data, size_t length); // mimics transfer send
char *parse_request(char *request);

typedef struct gfserver_t
{
    // set functions
    int max_npending;
    unsigned short port;
    
    // handler, handlerarg
    ssize_t (*handler)(gfcontext_t *, char *, void *);
    void *handlerarg;
} gfserver_t;

typedef struct gfcontext_t 
{ 
    int clientfd; 
    char *file_path; 
} gfcontext_t; 

void gfs_abort(gfcontext_t *ctx)
{
    close(ctx->clientfd);
    free(ctx);
    ctx = NULL;
}

gfserver_t* gfserver_create()
{
    gfserver_t *gfs = malloc(sizeof(gfserver_t));
    bzero(gfs, sizeof(gfserver_t));
    return gfs;
}


ssize_t gfs_send(gfcontext_t *ctx, void *data, size_t len)
{
    assert(ctx != NULL);
    assert(data != NULL);

    ssize_t data_sent = send_data(ctx->clientfd, data, len);
    if (data_sent != len)
    {
        printf("Data failed to send to client.\n");
        return -1;
    }

    return data_sent;
}

// gftstatus_t is just an int this time, so OK = 200, etc.
ssize_t gfs_sendheader(gfcontext_t *ctx, gfstatus_t status, size_t file_len)
{
    assert(ctx != NULL);
    char header[BUFSIZE];
    ssize_t header_len = 0;

    // OK STATUS
    if (status == GF_OK)
    {
        sprintf(header, ok_header_template, gfschema_string, gfok_string, file_len, gfheader_end_string);
    }
    // FILE_NOT_FOUND_STATUS
    else if (status == GF_FILE_NOT_FOUND)
    {
        sprintf(header, invalid_header_template, gfschema_string, gffnf_string, gfheader_end_string);
    }
    // ERROR STATUS
    else
    {
        sprintf(header, invalid_header_template, gfschema_string, gferror_string, gfheader_end_string);
    }

    // printf("Testing Header Sent: %s\n", header);
    header_len = strlen(header);
    
    ssize_t header_data_sent = send_data(ctx->clientfd, header, header_len);
    if (header_data_sent < header_len)
    {
        printf("Header failed to send to client.\n");
        return -1;
    }

    return header_len;
}

// used transfer server start
void gfserver_serve(gfserver_t *gfs)
{
    assert(gfs != NULL);
    gfcontext_t *gfc = malloc(sizeof(gfcontext_t)); // needed for gfs_sendheader function, since we are not including a gfcontext_t under the gfs struct
    if (gfc == NULL)
    {
        perror("Error allocating memory for gfcontext_t");
    }

    typedef struct sockaddr SA;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof clientaddr;
    int listenfd, bytes_received;
    char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);

    // convert portno to string
    char *port = malloc(16);
    snprintf(port, 16, "%u", gfs->port);

    listenfd = open_listenfd(gfs, port);

    gfc->clientfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (gfc->clientfd < 0)
    {
        printf("Failed to establish client connection.\n");
    }

    // technically in one recv, we should get entire header, again bad assumption, better implementation is to use a while loop and receive data until /r/n/r/n is seen or many other conditions
    bytes_received = recv(gfc->clientfd, buffer, BUFSIZE, 0);
    if (bytes_received < 0)
    {
        printf("Error receiving request from client.\n");
    }
    if (bytes_received == 0)
    {
        printf("Did not receive any request from client.\n");
    }

    // need path to pass to handler, check schema, method, grab path
    char *path = parse_request(buffer);
    printf("Path is: %s\n", path);

    if (path != NULL)
    {
        gfs->handler(gfc, path, gfs->handlerarg);
    }
    else
    {
        gfs_sendheader(gfc, GF_FILE_NOT_FOUND, 0);
    }

    if(close(gfc->clientfd) < 0)
    {
        printf("Failed to close socket connection.\n");
    }

    // free up shit
    free(port);
    free(path);
    free(gfs);
    free(gfc);
    close(listenfd);
}

void gfserver_set_handlerarg(gfserver_t *gfs, void* arg)
{
    assert(gfs != NULL);
    gfs->handlerarg = arg;
}

void gfserver_set_handler(gfserver_t *gfs, ssize_t (*handler)(gfcontext_t *, char *, void*))
{
    assert(gfs != NULL);
    assert(handler != NULL);
    gfs->handler = handler;
}

void gfserver_set_maxpending(gfserver_t *gfs, int max_npending)
{
    assert(gfs != NULL);
    gfs->max_npending = max_npending;
}

void gfserver_set_port(gfserver_t *gfs, unsigned short port)
{
    assert(gfs != NULL);
    gfs->port = port;
}


// function that handles listening portion of server start
int open_listenfd(gfserver_t *gfs, char *port)
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
    
    if (listen(listenfd, gfs->max_npending) < 0)
    {
        close(listenfd);
        return -1;
    }

    return listenfd;
}

// function to handle sending data (copied from transfer)
ssize_t send_data(int socket, char *data, size_t length)
{
    ssize_t bytes_sent = 0;
    while (bytes_sent < length)
    {
        ssize_t temp_sent = send(socket, data + bytes_sent, length - bytes_sent, 0);
        if (temp_sent < 0)
        {
            printf("Sending content error.\n");
            return -1;
        }

        bytes_sent += temp_sent;
    }

    return bytes_sent;
}

// parse path
char *parse_request(char *request)
{
    char *temp;
    int path_length = 0;
    
    temp = strtok(request, " ");
    //printf("schema: %s\n", temp);
    if (temp == NULL)
    {
        return NULL;
    }

    if (strcmp(temp, gfschema_string) != 0)
    {
        return NULL;
    }
    

    temp = strtok(NULL, " ");
    if (strcmp(temp, "GET") != 0)
    {
        return NULL;
    }

    temp = strtok(NULL, "\r\n\r\n");
    if (temp == NULL)
    {
        return NULL;
    }

    path_length = strlen(temp) + 1;
    char *path = (char *)malloc(path_length * sizeof(char));
    strcpy(path, temp);

    return path;
}