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

/* 
NOTES:
"%n" usefulness: will return number of characters read or printed by printf or scanf
EXAMPLE: input: 10 20 ===> scanf(%d%d%n, a, b, num) ====> 1,0,'',2,0, so num = 5
*/

#define BUFSIZE 4096

const char *request_template = "GETFILE GET %s\r\n\r\n";
//const char *response_template = "%s %s %zd\r\n\r\n%n"; wonky, bc the one case starts with a \n and doesnt include it
const char *response_template = "%s %s %zd%n";
const char *response_end_template = "\r\n\r\n";

// functions created in this file
int create_socket(char *hostname, unsigned short portno);
int open_sockfd(char *host, char *port);
int send_request(gfcrequest_t *gfr, int socket);
int parse_header(gfcrequest_t *gfr, int socket);
int parse_chunk(gfcrequest_t *gfr, int socket);

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
    shutdown(sockfd_gfr, SHUT_WR);

    if ((parse_header(gfr, sockfd_gfr)) < 0 )
    {
        perror("Failed to parse header.");
        close(sockfd_gfr);
        return -1;
    }

    if ((parse_chunk(gfr, sockfd_gfr)) < 0 )
    {
        perror("Failed to parse chunk data.");
        close(sockfd_gfr);
        return -1;
    }

    close(sockfd_gfr);
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

int create_socket(char *hostname, unsigned short portno)
{
    int sockfd;

    // convert portno to string
    char *port = malloc(20);
    snprintf(port, 20, "%u", portno);

    // created this to utilize the getaddrinfo api
    sockfd = open_sockfd(hostname, port);

    free(port);
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

int send_request(gfcrequest_t *gfr, int socket)
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
        bytes_sent = send(socket, request + n, request_length, 0);
        if(bytes_sent < 0)
        {
            printf("Failed to send request to server\n.");
            gfr->status = GF_ERROR;
            return -1;
        }
        n += bytes_sent;
    }

    return 0;
}

/* 
Receive Header - GETFILE OK 107528\r\n\r\n%data
When the status is OK, the length should be a number expressed in ASCII (what sprintf will give you)
The sequence ‘\r\n\r\n’ marks the end of the header. All remaining bytes are the files contents.
*/
int parse_header(gfcrequest_t *gfr, int socket)
{
    // Kinda bad - but assumption that data read in to ssize t is large
    // enough to be the header + potential file data so we only do one recv 
    size_t byte_count = 0;
    char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);

    //char header[BUFSIZE];
    //bzero(header, BUFSIZE);

    byte_count = recv(socket, buffer, BUFSIZE, 0);

    if (byte_count == 0)
    {
        gfr->status = GF_INVALID;
        return -1;
    }

    if (byte_count < 0)
    {
        gfr->status = GF_ERROR;
        return -1;
    }

    /*
    Header consists of: 
    GETFILE (SCHEMA) STATUS FILESIZE END CHUNKSTART (GET POSITION)
    Necessary variables for header
    */
    char str_schema[50];
    char str_status[50];
    size_t file_size = 0;
    int eom_start_position = 0; // this is actually the header without the end marker, chunk start position = this + marker (4), technically the end of marker start position

    printf("Received Response: %s\n", buffer);
    //printf("Received Header: %s", strtok(buffer, response_end_template));

    //strcat(header, strtok(buffer, response_end_template));
    //printf("Received Response: %s\n", header);

    /*
    PARSE HEADER PORTION
    note: sscanf returns number of fields filled or EOF unsuccessful
    */ 

    // printf("in the buffer:%s\n", buffer);
    if (sscanf(buffer, response_template, str_schema, str_status, &file_size, &eom_start_position) == EOF)
    {
        printf("SCANNING HEADER FAIL.\n");
        gfr->status = GF_INVALID;
        return -1;
    }

    /*
    VALIDATE HEADER PORTION
    */ 
    if (str_schema == NULL || str_status == NULL)
    {
        printf("INVALID HEADER.\n");
        gfr->status = GF_INVALID;
        return -1;
    }
    
    // SCHEMA CHECK - POSSIBLE "GET" SCHEMA? XD
    if (strcmp(str_schema, "GETFILE") != 0)
    {
        printf("INVALID HEADER.\n");
        gfr->status = GF_INVALID;
        return -1;
    }

    // STATUS CHECK - CHECK AGAINST SET: {OK,FILE_NOT_FOUND,ERROR}
    if (strcmp(str_status, "OK") == 0)
    {
        gfr->status = GF_OK;
    }
    else if (strcmp(str_status, "FILE_NOT_FOUND") == 0)
    {
        gfr->status = GF_FILE_NOT_FOUND;
    }
    else if (strcmp(str_status, "ERROR") == 0)
    {
        gfr->status = GF_ERROR;
    }
    else
    {
        printf("INVALID HEADER.\n");
        gfr->status = GF_INVALID;
        bzero(buffer, BUFSIZE);
        return -1;
    }
    /*
    if (gfr->headerfunc)
    {
        gfr->headerfunc(header, strlen(header), gfr->headerarg);
    }
    bzero(header, BUFSIZE);
    */

    gfr->filelen = file_size;

    // IT'S POSSIBLE THAT ON THE RECV, WE GOT FILE DATA ALONG WITH THE HEADER IN THE BUFFER
    // SO WE NEED TO DO FANCY CALLBACK STUFF HERE.
    if ((gfr->status == GF_OK) && (eom_start_position < byte_count))
    {
        //printf("bytes received over socket: %zu\n", byte_count);
        //printf("data start position: %d\n", eom_start_position);
        size_t write_chunk = byte_count - (eom_start_position + strlen(response_end_template));
        //printf("first grab byte count: %zu\n", write_chunk);
        gfr->writefunc(buffer + eom_start_position + strlen(response_end_template), write_chunk, gfr->writearg);
        gfr->bytesreceived += write_chunk;
        //printf("bytes received: %zu\n", gfr->bytesreceived);
        //printf("file length: %zu\n", gfr->filelen);
    }
    //bzero(buffer, BUFSIZE);
    return 0;
}


/*
Parse CHUNK / FILE CONTENT
*/
int parse_chunk(gfcrequest_t *gfr, int socket)
{
    char buffer[BUFSIZE];
    
    while(gfr->status == GF_OK && (gfr->bytesreceived < gfr->filelen))
    {
        bzero(buffer, BUFSIZE);
        size_t write_chunk = recv(socket, buffer, BUFSIZE, 0);
        if (write_chunk == 0)
        {
            printf("Chunk data corrupt.\n");
            return -1;
        }

        if (write_chunk < 0)
        {
            printf("Socket connection broke during transfer of chunk.\n");
            return -1;
        }

        gfr->writefunc(&buffer, write_chunk, gfr->writearg);
        gfr->bytesreceived += write_chunk;
    }

    return 0;
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

    return strstatus;
}