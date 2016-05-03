#include "csapp.h"
#include <pthread.h>

#define DEBUG

// testing
int open_clientfd(char *hostname, int port);
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);

int threadCount = 0;

char board[3][3] = { // The board 
        {'1', '2', '3'}, // Initial values are reference numbers 
        {'4', '5', '6'}, // used to select a vacant square for 
        {'7', '8', '9'} // a turn. 
};
int turn = 0;
int currentPlayer = -1;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void* handleConnection(void* args);

typedef struct arguments_t {

    int connfd;
    struct sockaddr_in clientaddr;
    int playerID;

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
    int pid;

    //int request_count = 0;    /* Number of requests received so far */

    //Used to fix a bug
    int error = 0;                  /* Used to detect error in reading requests */



    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    signal(SIGPIPE, SIG_IGN);

    /* Create a listening descriptor */
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);
    while (1) {
        error = 0;   //Used to fix a bug
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        //if (pid == fork())
        args = malloc(sizeof(arguments));
        args->connfd = connfd;
        args->clientaddr = clientaddr;
        args->playerID = threadCount;
        if (pthread_create(&thread, NULL, &handleConnection, args)) {
            //free(args);
            perror("Thread creation failed!");
            continue;
        }

        if (threadCount == 2) {
            pid = fork();
            if (pid != 0) {
                //playGame
                //wait here
                printf("In Parent process, created child: %d\n", pid);

            } else {
                threadCount = 0;
                //continue while loop
            }
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
    threadCount++;

    struct sockaddr_in clientaddr;  /* Clinet address structure*/
    int connfd;                     /* socket desciptor for talkign wiht client*/
    int serverfd;                   /* Socket descriptor for talking with end server */
    int n;                       /* General index and counting variables */
    int playerID;

    rio_t rio;                      /* Rio buffer for calls to buffered rio_readlineb routine */
    int* selection;              /* General I/O buffer */
    int error = 0;                  /* Used to detect error in reading requests */

    arguments* args = (arguments*)argsVoid;
    clientaddr = args->clientaddr;
    connfd = args->connfd;
    playerID = args->playerID;



    printf("Thread Number/Player ID: %d\n", threadCount);
    while(threadCount != 2){
        sleep(10);
    }
    Rio_readinitb(&rio, args->connfd);

    while (1) {

        if (turn > 8){
            printf("Draw!\n");
            //send to both players
        }
        if (turn % 2 == 0 && playerID == 0){ //Player 1's turn
            if ((n = Rio_readlineb(&rio, selection, MAXLINE)) <= 0) {   //Read input
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                //free(request);
                break;
            }
            //place input into grid (X)
            printf("request: %d\n", selection);

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            turn++; //Increment turn
        }else if (turn % 2 == 1)

        //print to both ppl
        Rio_writen(args->connfd, selection, n);
    }
    close(args->connfd);
    close(serverfd);
    return NULL;
}
