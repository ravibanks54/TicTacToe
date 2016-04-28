#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include "csapp.h"

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[256];

   if (argc < 3) {
      fprintf(stderr, "usage %s hostname port\n", argv[0]);
      exit(0);
   }

   portno = atoi(argv[2]);

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(argv[1]);

   if (server == NULL) {
      fprintf(stderr, "ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   /* Now ask for a message from the user, this message
      * will be read by server
   */
   while (1) {
      printf("Please enter the message: ");
      bzero(buffer, 256);
      fgets(buffer, 255, stdin);

      /* Send message to the server */
      n = write(sockfd, buffer, strlen(buffer));

      if (n < 0) {
         perror("ERROR writing to socket");
         exit(1);
      }

      /* Now read server response */
      bzero(buffer, 256);
      n = read(sockfd, buffer, 255);

      if (n < 0) {
         perror("ERROR reading from socket");
         exit(1);
      }

      printf("%s\n", buffer);
   }
   return 0;
}

void playgame(int socket, char * buffer, int playerID)
{
   int len, datasocket = socket;

   printf("\nSTARTING GAME\n");

   int i = 0; /* Loop counter */
   int player = 0; /* Player number - 1 or 2 */
   int go = 0; /* Square selection number for turn */
   int row = 0; /* Row index for a square */
   int column = 0; /* Column index for a square */
   int line = 0; /* Row or column index in checking loop */
   int winner = 0; /* The winning player */
   char board[3][3] = { /* The board */
      {'1', '2', '3'}, /* Initial values are reference numbers */
      {'4', '5', '6'}, /* used to select a vacant square for */
      {'7', '8', '9'} /* a turn. */
   };

   /* The main game loop. The game continues for up to 9 turns */
   /* As long as there is no winner */
   for ( i = (0 + turn); i < (9 + turn) && winner == 0; i++)
   {
      /* Display the board */
      printf("\n\n");
      printf(" %c | %c | %c\n", board[0][0], board[0][1], board[0][2]);
      printf("---+---+---\n");
      printf(" %c | %c | %c\n", board[1][0], board[1][1], board[1][2]);
      printf("---+---+---\n");
      printf(" %c | %c | %c\n", board[2][0], board[2][1], board[2][2]);

      player = i % 2 + 1; /* Select player */

      /* Take appropriate actions based on turn. */
      do
      {
         if (player == playerID)
         {
            printf("\n%s, please enter the number of the square "
                   "where you want to place your %c: ", name, (player == 1) ? 'X' : 'O');
            scanf("%d", &go);
            send(datasocket, &go, sizeof(go), 0); /* Send your selection */
         }
         else
         {
            printf("\nWaiting for %s...\n", opponent);
            len = recv(datasocket, &go, MAXRCVLEN, 0); /* Receive peer's selection */
            printf("%s chose %d\n", opponent, go);
         }

         row = --go / 3; /* Get row index of square */
         column = go % 3; /* Get column index of square */
      } while (go < 0 || go > 9 || board[row][column] > '9');

      board[row][column] = (player == 1) ? 'X' : 'O'; /* Insert player symbol */

      /* Check for a winning line - diagonals first */
      if ((board[0][0] == board[1][1] && board[0][0] == board[2][2]) ||
            (board[0][2] == board[1][1] && board[0][2] == board[2][0]))
         winner = player;
      else
         /* Check rows and columns for a winning line */
         for (line = 0; line <= 2; line ++)
            if ((board[line][0] == board[line][1] && board[line][0] == board[line][2]) ||
                  (board[0][line] == board[1][line] && board[0][line] == board[2][line]))
               winner = player;

   }
   /* Game is over so display the final board */
   printf("\n\n");
   printf(" %c | %c | %c\n", board[0][0], board[0][1], board[0][2]);
   printf("---+---+---\n");
   printf(" %c | %c | %c\n", board[1][0], board[1][1], board[1][2]);
   printf("---+---+---\n");
   printf(" %c | %c | %c\n", board[2][0], board[2][1], board[2][2]);

   /* Display result message */
   if (winner == 0)
      printf("\nHow boring, it is a draw.\n");
   else if (winner == playerID)
      printf("\nCongratulations %s, YOU ARE THE WINNER!\n", name);
   else printf("\n%s wins this round...\n", opponent);

   /* Switch first turn */
   if (turn == 0 ) turn++;
   else turn--;

   /* Ask to play another round */
   printf("\nPlay another round? (y/n) ");
   fgetc(stdin);
   fgets(buffer, sizeof buffer, stdin);
   buffer[strlen(buffer) - 1] = '\0';
   printf("\nWating for %s to acknowledge...\n", opponent);

   /* Set while loop flag to true if both players agree to play again otherwise set it to false */
   ingame = 0;
   if (strcmp(buffer, "y") == 0) ingame = 1;
   send(datasocket, buffer, strlen(buffer), 0);
   len = recv(datasocket, buffer, MAXRCVLEN, 0);
   buffer[len] = '\0';
   if (strcmp(buffer, "y") != 0 && ingame == 1)
   {
      printf("\n%s has declined...\n", opponent);
      ingame = 0;
   }
}