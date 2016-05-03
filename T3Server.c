#include "csapp.h"
#include <pthread.h>
#include <unistd.h>

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
    char buf[256];
    rio_t rio;                      /* Rio buffer for calls to buffered rio_readlineb routine */
    int* selection;              /* General I/O buffer */
    int error = 0;                  /* Used to detect error in reading requests */

    arguments* args = (arguments*)argsVoid;
    clientaddr = args->clientaddr;
    connfd = args->connfd;
    playerID = args->playerID;



    printf("Thread Number/Player ID: %d\n", threadCount);
    while(threadCount != 2){
        sleep(1);
    }
    Rio_readinitb(&rio, args->connfd);

    while (1) {

        if (turn > 8){
            printf("Draw!\n");
            //send to both players
        }
        if (turn % 2 == 0 && playerID == 0){ //Player 1's turn
            //print board
            if ((n = Rio_readlineb(&rio, selection, MAXLINE)) <= 0) {   //Read input
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                //free(request);
                break;
            }
            int pos = *selection;
            //place input into grid (X)
            printf("request: %d\n", pos);

            int row = --pos/3;
            int column = pos%3;

            if(board[row][column] == 'X' || board[row][column] == 'O'){
                //buf = "Error, move already made!\n";
                Rio_writen(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                continue;
            }

            if (pos <1 || pos > 9){
                printf("\nPlease enter a proper value.\n");
                continue;
            }

            board[row][column] = 'X';

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n  %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            Rio_writen(args->connfd, buf, 256);
            
            turn++; //Increment turn  

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            //printboard
            turn++; //Increment turn
        }else if (turn % 2 == 1 && playerID == 1){
            //printboard
            if ((n = Rio_readlineb(&rio, selection, MAXLINE)) <= 0) {   //Read input
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                //free(request);
                break;
            }

            int pos = *selection; 
            //place input into grid (O)
            printf("request: %d\n", pos);

            int row = --pos/3;
            int column = pos%3;

            if(board[row][column] == 'X' || board[row][column] == 'O'){
                //buf = "Error, move already made!\n";
                Rio_writen(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                continue;
            }

            if (pos <1 || pos > 9){
                printf("\nPlease enter a proper value.\n");
                continue;
            }

            board[row][column] = 'O';

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n  %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            Rio_writen(args->connfd, buf, 256);

            turn++; //Increment turn  
        }else{
            sleep(1);
            continue;
        }



        //print to both ppl
        Rio_writen(args->connfd, selection, n);
    }
    close(args->connfd);
    close(serverfd);
    return NULL;
}
