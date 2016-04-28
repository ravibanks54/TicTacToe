#include "csapp.h"
#include <pthread.h>

#define DEBUG

// testing git commit
int open_clientfd(char *hostname, int port);
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void* handleConnection(void* args);

typedef struct arguments_t {

    int connfd;
    struct sockaddr_in clientaddr;

} arguments;

int main(int argc, char **argv)
{
    int listenfd;             /* The proxy's listening descriptor */
    int port;                 /* The port the proxy is listening on */
    int clientlen;            /* Size in bytes of the client socket address */
    struct sockaddr_in clientaddr;
    int connfd;
    arguments* args;
    pthread_t thread;

    //int request_count = 0;    /* Number of requests received so far */

    //Used to fix a bug
    int error = 0;                  /* Used to detect error in reading requests */



    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    /*
     * Ignore any SIGPIPE signals elicited by writing to a connection
     * that has already been closed by the peer process.
     */
    signal(SIGPIPE, SIG_IGN);

    /* Create a listening descriptor */
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);

    /* Inititialize */
    
    //int pid;
    /* Wait for and process client connections */
    while (1) {
        error = 0;   //Used to fix a bug
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        //if (pid == fork())
        args = malloc(sizeof(arguments));
        args->connfd = connfd;
        args->clientaddr = clientaddr;
        if (pthread_create(&thread, NULL, &handleConnection, args)) {
            //free(args);
            perror("Thread creation failed!");
            continue;
        }

    }

    /* Control never reaches here */
    exit(0);
}



/*
 * Read the entire HTTP request into the request buffer, one line
 * at a time.
 */

void* handleConnection(void* argsVoid) {
    struct sockaddr_in clientaddr;  /* Clinet address structure*/
    int connfd;                     /* socket desciptor for talkign wiht client*/
    int serverfd;                   /* Socket descriptor for talking with end server */
    //char *request;                  /* HTTP request from client */
    //char *request_uri;              /* Start of URI in first HTTP request header line */
    //char *request_uri_end;          /* End of URI in first HTTP request header line */
    //char *rest_of_request;          /* Beginning of second HTTP request header line */
    //int request_len;                /* Total size of HTTP request */
    //int response_len;               /* Total size in bytes of response from end server */
    int n;                       /* General index and counting variables */
    //int realloc_factor;             /* Used to increase size of request buffer if necessary */

    //char hostname[MAXLINE];         /* Hostname extracted from request URI */
    //char pathname[MAXLINE];         /* Content pathname extracted from request URI */
    //char request[MAXLINE];
    //int serverport;                 /* Port number extracted from request URI (default 80) */
    //char log_entry[MAXLINE];        /* Formatted log entry */

    rio_t rio;                      /* Rio buffer for calls to buffered rio_readlineb routine */
    char buf[MAXLINE];              /* General I/O buffer */

    //Used to fix a bug
    int error = 0;                  /* Used to detect error in reading requests */

    /*if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n Mutex init failed\n");
        return NULL;
    }*/
    arguments* args = (arguments*)argsVoid;
    clientaddr = args->clientaddr;
    connfd = args->connfd;

    
    Rio_readinitb(&rio, args->connfd);
    while(1){
        if ((n = Rio_readlineb(&rio, buf, MAXLINE)) <= 0) {
            error = 1;  //Used to fix a bug
            printf("process_request: client issued a bad request (1).\n");
            close(args->connfd);
            //free(request);
            break;
        }
        printf("request: %s\n", buf);
        if (error) {
            close(connfd);
            pthread_exit(NULL);
        }
        //pthread_mutex_lock(&lock);
        //pthread_mutex_unlock(&lock);
        /*
         * Receive reply from server and forward on to client
         */
    	Rio_writen(args->connfd, buf, n);
    }
    close(args->connfd);
    close(serverfd);
    //free(request);
    //pthread_mutex_destroy(&lock);
    return NULL;
}
