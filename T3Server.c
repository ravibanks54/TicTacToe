#include "csapp.h"
#include <pthread.h>
#include <unistd.h>

#define DEBUG
/*
TODO: (Icing on top)
Block input (write to client initially, read, strcmp)
Add play again option
Fix Tie game final board state
Tell players which symbol they are
*/
int open_clientfd(char *hostname, int port);
int threadCount = 0;

char board[3][3] = { // The board 
        {'1', '2', '3'}, // Initial values are reference numbers 
        {'4', '5', '6'}, // used to select a vacant square for 
        {'7', '8', '9'} // a turn. 
};
int turn = 0;
int currentPlayer = -1;
int hodor = 0;

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
        args = malloc(sizeof(arguments));
        args->connfd = connfd;
        args->clientaddr = clientaddr;
        args->playerID = threadCount;
        if (pthread_create(&thread, NULL, &handleConnection, args)) {
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

void* handleConnection(void* argsVoid) {
    threadCount++;
    struct sockaddr_in clientaddr;  /* Clinet address structure*/
    int connfd;                     /* socket desciptor for talking wiht client*/
    int n;                       /* General index and counting variables */
    int playerID;
    char buf[256];
    int line;
    int selection;             
    int error = 0;                  /* Used to detect error in reading requests */
    int retval=-1;
    arguments* args = (arguments*)argsVoid;
    clientaddr = args->clientaddr;
    connfd = args->connfd;
    playerID = args->playerID;

    printf("Thread Number/Player ID: %d\n", threadCount);
    while(threadCount != 2){
        sleep(1);
    }
    while (1) {
        if (turn > 8 && hodor == 0){
            retval = write(args->connfd, "Tie game!!!\n", strlen("Tie game!!!\n"));
            if (retval < 0){
                printf("Error writing!\n");
                pthread_exit(NULL);
            }
            pthread_exit(NULL);
        }
        if (turn % 2 == 0 && playerID == 0){ //Player 1's turn
            if(hodor == 1){
                retval = write(args->connfd, "You lose!!!\n", strlen("You lose!!!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
                pthread_exit(NULL);
            }
            bzero(buf, 256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
            }
            if ((n = read(args->connfd, &selection, MAXLINE)) <= 0){
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                break;
            }
            int pos = selection;
            int row = --pos/3;
            int column = pos%3;
            if(board[row][column] == 'X' || board[row][column] == 'O'){
                retval = write(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
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
            if(retval < 0){
                printf("Error writing!\n");
                pthread_exit(NULL);
            }
            
            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            // Check for a winning line - diagonals first 
            if ((board[0][0] == board[1][1] && board[0][0] == board[2][2]) || (board[0][2] == board[1][1] && board[0][2] == board[2][0])){
                retval = write(args->connfd, "You win!!!\n", strlen("You win!!!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
                hodor = 1;
                turn++;
                pthread_exit(NULL);
            }else{
            // Check rows and columns for a winning line 
                for (line = 0; line <= 2; line ++){
                    if ((board[line][0] == board[line][1] && board[line][0] == board[line][2]) || (board[0][line] == board[1][line] && board[0][line] == board[2][line])){
                        retval = write(args->connfd, "You win!!!\n", strlen("You win!!!\n"));
                        if(retval < 0){
                            printf("Error writing!\n");
                            pthread_exit(NULL);
                        }
                        hodor = 1;
                        turn++;
                        pthread_exit(NULL);
                    }
                }
            }
            turn++; //Increment turn
        }else if (turn % 2 == 1 && playerID == 1){
            if(hodor == 1){
                retval = write(args->connfd, "You lose!!!\n", strlen("You lose!!!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
                pthread_exit(NULL);
            }
            bzero(buf, 256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            if(retval < 0){
                printf("Error writing!\n");
                pthread_exit(NULL);
            }
            bzero(buf, 256);
            if ((n = read(args->connfd, &selection, MAXLINE)) <= 0){
                error = 1;  //Used to fix a bug
                printf("process_request: client issued a bad request (1).\n");
                close(args->connfd);
                break;
            }
            int pos = selection;
            int row = --pos/3;
            int column = pos%3;

            if(board[row][column] == 'X' || board[row][column] == 'O'){
                retval = write(args->connfd, "Error, move already made!\n", strlen("Error, move already made!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
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
            //printf("Right before write\n");
            bzero(buf,256);
            snprintf(buf, 256, "\n\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n ---+---+---\n %c | %c | %c\n", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
            retval = write(args->connfd, buf, 256);
            if(retval < 0){
                printf("Error writing!\n");
                pthread_exit(NULL);
            }
            
            if (error) {
                close(connfd);
                pthread_exit(NULL);
            }
            // Check for a winning line - diagonals first 
            if ((board[0][0] == board[1][1] && board[0][0] == board[2][2]) || (board[0][2] == board[1][1] && board[0][2] == board[2][0])){
                retval = write(args->connfd, "You win!!!\n", strlen("You win!!!\n"));
                if(retval < 0){
                    printf("Error writing!\n");
                    pthread_exit(NULL);
                }
                hodor = 1;
                turn++;
                pthread_exit(NULL);
            }else{
            // Check rows and columns for a winning line 
                for (line = 0; line <= 2; line ++){
                    if ((board[line][0] == board[line][1] && board[line][0] == board[line][2]) || (board[0][line] == board[1][line] && board[0][line] == board[2][line])){
                        retval = write(args->connfd, "You win!!!\n", strlen("You win!!!\n"));
                        if(retval < 0){
                            printf("Error writing!\n");
                            pthread_exit(NULL);
                        }
                        hodor = 1;
                        turn++;
                        pthread_exit(NULL);
                    }
                }
            }
            turn++; //Increment turn
        }else{
            sleep(1);
            continue;
        }
    }
    close(args->connfd);
    return NULL;
}
