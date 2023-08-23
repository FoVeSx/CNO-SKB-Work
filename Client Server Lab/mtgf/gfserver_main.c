#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>

#include "gfserver.h"
#include "content.h"
#include "steque.h"

#define USAGE                                                          \
  "usage:\n"                                                           \
  "  gfserver_main [options]\n"                                        \
  "options:\n"                                                         \
  "  -h                  Show this help message.\n"                    \
  "  -c [content_file]   Content file mapping keys to content files\n" \
  "  -p [listen_port]    Listen port (Default: 8080)\n"                \
  "  -t [nthreads]       Number of threads (Default: 1)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"port", required_argument, NULL, 'p'},
    {"content", required_argument, NULL, 'c'},
    {"nthreads", required_argument, NULL, 't'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

extern ssize_t handler_setup(gfcontext_t *ctx, char *path, void *arg); 
extern ssize_t handler_get(gfcontext_t *ctx, char *path);

// GLOBALS
static steque_t *queue;
static pthread_t *worker_thread;
static int nthreads;
static gfserver_t *gfs;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_worker = PTHREAD_COND_INITIALIZER;

// Instead of just pushing into the queue a gfcontext request
// pass a struct that also contains file path, makes life easier
typedef struct queue_ctx
{
  gfcontext_t *ctx;
  char *file_path;
} queue_ctx;

// Populate struct with gfcontext client request and file path
void enqueue_ctx(gfcontext_t *ctx, char *file_path)
{
  queue_ctx *q_ctx = malloc(sizeof(queue_ctx));
  q_ctx->ctx = ctx;
  q_ctx->file_path = file_path;
  steque_enqueue(queue, q_ctx);
}

// Process the request, pop from queue, send content using transfer from handler
void *process_request(void *args)
{
  while (1)
  {
    // MUTEX LOCK - SIMILAR TO CLIENT, WAKE BOSS THREAD TO CHECK IF Q EMPTY
    // UNLOCK MUTEX - SEND THE GOODS TO CLIENT
    pthread_mutex_lock(&mutex);

    while (steque_isempty(queue))
    {
      pthread_cond_wait(&cond_worker, &mutex);
    }

    queue_ctx *q_ctx = steque_pop(queue);

    pthread_mutex_unlock(&mutext);

    handler_get(q_ctx->ctx, q_ctx->file_path);

    if (q_ctx->ctx != NULL)
    {
      gfs_abort(q_ctx->ctx);
    }

    free(q_ctx->file_path);
    free(q_ctx);
  }
}

// DO NOT MODIFY
static void _sig_handler(int signo)
{
  if (signo == SIGINT || signo == SIGTERM)
  {
    exit(signo);
  }
}

/* Main ========================================================= */
int main(int argc, char **argv)
{
  int option_char = 0;
  unsigned short port = 8080;
  char *content = "content.txt";
  gfs = NULL;
  nthreads = 1;

  // DO NOT MODIFY
  if (signal(SIGINT, _sig_handler) == SIG_ERR)
  {
    fprintf(stderr, "Can't catch SIGINT...exiting.\n");
    exit(EXIT_FAILURE);
  }

  if (signal(SIGTERM, _sig_handler) == SIG_ERR)
  {
    fprintf(stderr, "Can't catch SIGTERM...exiting.\n");
    exit(EXIT_FAILURE);
  }

  // Parse and set command line arguments
  while ((option_char = getopt_long(argc, argv, "p:t:c:h", gLongOptions, NULL)) != -1)
  {
    switch (option_char)
    {
    case 'p': // listen-port
      port = atoi(optarg);
      break;
    case 't': // nthreads
      nthreads = atoi(optarg);
      break;
    case 'c': // file-path
      content = optarg;
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

  /* not useful, but it ensures the initial code builds without warnings */
  if (nthreads < 1)
  {
    nthreads = 1;
  }

  content_init(content);

  /*Initializing server*/
  gfs = gfserver_create();

  /*Setting options*/
  gfserver_set_port(gfs, port);
  gfserver_set_maxpending(gfs, 100);
  gfserver_set_handler(gfs, handler_setup);
  gfserver_set_handlerarg(gfs, NULL);

  // initialize queue 
  queue = malloc(sizeof(steque_t));
  steque_init(queue);

  // setup worker threads
  worker_thread = malloc(sizeof(pthread_t));

  for (int i = 0; i < nthreads; i++)
  {
    if (pthread_create(&workers[i], NULL, process, NULL) != 0) 
    {
      printf("Failed to create Worker Thread #%d\n", i);
      return -1;
    }
  }

  /*Loops forever*/
  gfserver_serve(gfs);
  
  // Join worker threads with main thread, for good ole thread safety
  for (int i = 0; i < nthreads; i++) 
  {
    if (pthread_join(workers[i], NULL) < 0) 
    {
      printf("Failed to join Worker Thread\n");
    }
  }
  
  steque_destroy(queue);

  free(queue);
  free(workers);
  free(gfs);

  content_destroy();
}
