#include "csapp.h"
#include <pthread.h>
#include <unistd.h>

#define DEBUG
/*
TODO: Block input (write to client initially, read, strcmp)
Check for Win COndition
Print Draw/win/lose to clients



*/
// testing
int open_clientfd(char *hostname, int port);
//ssize_t read(int fd, void *ptr, size_t nbytes);
//ssize_t read(rio_t *rp, void *usrbuf, size_t maxlen);
//void Rio_writen_w(int fd, void *usrbuf, size_t n);

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

        if (threadCount == 1) {
            pid = fork();
            if (pid != 0) {
                printf("In Parent process, created child: %d\n", pid);
                while(1){
                    sleep(1000000000000000000);
                }
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
    int connfd;                     /* socket desciptor for talking wiht client*/
    int n;                       /* General index and counting variables */
    int playerID;
    char buf[256];

 //   rio_t rio;                      /* Rio buffer for calls to buffered rio_readlineb routine */
    int selection;              /* General I/O buffer */
    int error = 0;                  /* Used to detect error in reading requests */
    //bzero()
    int retval=-1;
    arguments* args = (arguments*)argsVoid;
    clientaddr = args->clientaddr;
    connfd = args->connfd;
    playerID = args->playerID;



    printf("Thread Number/Player ID: %d\n", threadCount);
    while(threadCount != 2){
        sleep(1);
    }
    //Rio_readinitb(&rio, args->connfd);

    while (1) {

        if (turn > 8){
            printf("Draw!\n");
            //send to both players
        }
        if (turn % 2 == 0 && playerID == 0){ //Player 1's turn
            bzero(buf, 256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            printf("Retval1 = %d\n", retval);
            //if ((n = Rio_readnb(&rio, selection, MAXLINE)) <= 0) {   //Read input
            if ((n = read(args->connfd, &selection, MAXLINE)) <= 0){
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                //free(request);
                break;
            }
            int pos = selection;
            //place input into grid (X)
            printf("pos: %d\n", pos);

            int row = --pos/3;
            int column = pos%3;

            if(board[row][column] == 'X' || board[row][column] == 'O'){
                //buf = "Error, move already made!\n";
                retval = write(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                printf("Retval2 = %d\n", retval);
                continue;
            }

            if (pos <0 || pos > 8){
                printf("\nPlease enter a proper value.\n");
                continue;
            }

            board[row][column] = 'X';

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            bzero(buf,256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            printf("Retval3 = %d\n", retval);
            
            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            //printboard
            turn++; //Increment turn
        }else if (turn % 2 == 1 && playerID == 1){
            bzero(buf, 256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            printf("Retval4 = %d\n", retval);
            bzero(buf, 256);
            //if ((n = Rio_readnb(&rio, selection, MAXLINE)) <= 0) {   //Read input
            if ((n = read(args->connfd, &selection, MAXLINE)) <= 0){
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                //free(request);
                break;
            }
            int pos = selection;
            //place input into grid (X)
            printf("pos: %d\n", pos);

            int row = --pos/3;
            int column = pos%3;

            if(board[row][column] == 'X' || board[row][column] == 'O'){
                //buf = "Error, move already made!\n";
                retval = write(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                printf("Retval5 = %d\n", retval);
                continue;
            }

            if (pos <0 || pos > 8){
                printf("\nPlease enter a proper value.\n");
                continue;
            }

            board[row][column] = 'O';

            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            printf("Right before write\n");
            bzero(buf,256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            printf("Retval6 = %d\n", retval);
            
            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            //printboard
            turn++; //Increment turn
        }else{
            sleep(1);
            continue;
        }

    }
    close(args->connfd);
    return NULL;
}
