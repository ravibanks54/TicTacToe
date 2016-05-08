#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include "csapp.h"



void playgame(int socket, char * buffer, int playerID);

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   int selection;
   
   char buffer[256];
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);
   
   // Create a socket
   sockfd = Socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = Gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   // Now connect to the server
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   bzero(buffer, 256);
   n = read(sockfd, buffer, 256);
   if(n < 0){
      printf("Error reading!\n");
   }
   printf("%s\n", buffer);

   
   // Now ask for a message from the user, this message will be read by server
   
	while(1){
      bzero(buffer, 256);
      n = read(sockfd, buffer, 256);
      if(n < 0){
         printf("Error reading!\n");
      }

      //Check if you have won, lost or drawn

      printf("%s\n", buffer);
      if(strcmp(buffer, "You win!!!\n") == 0){
         exit(0);
      }else if(strcmp(buffer, "You lose!!!\n") == 0){
         exit(0);
      }else if(strcmp(buffer, "Tie game!!!\n") == 0){
         exit(0);
      }
      
      //Select the square you want to place your symbol in
      printf("\nPlease enter the number of the square:\n");
      scanf("%d", &selection);
      if (selection <1 || selection > 9){
         printf("\nPlease enter a proper value.\n");
         continue;
      }

      // Send message to the server
      n = write(sockfd, &selection, sizeof(selection));      
      if (n < 0) {
         perror("ERROR writing to socket");
         exit(1);
      }
      
      // Now read server response
      bzero(buffer,256);
      n = read(sockfd, buffer, 256);      
      if (n < 0) {
         perror("ERROR reading from socket");
         exit(1);
      }
   	
      printf("%s\n",buffer);
   }
   return 0;
}