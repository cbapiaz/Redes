#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> //close

#include <iostream>

#define PORT 3490
#define MY_IP "127.0.0.1"
#define MAX_QUEUE 10
#define MAX_MSG_SIZE 1024


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc , char *argv[])
{
   //primitiva SOCKET
   int server_socket = socket(AF_INET, SOCK_STREAM, 0);
   
   //primitiva BIND
   struct sockaddr_in server_addr;
   socklen_t server_addr_size = sizeof server_addr;
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(PORT);
   server_addr.sin_addr.s_addr = inet_addr(MY_IP);
   bind(
      server_socket, 
      (struct sockaddr*)&server_addr, server_addr_size
   );

   //primitiva LISTEN
   listen(server_socket, MAX_QUEUE);
   
   char saddr[INET6_ADDRSTRLEN];


   while (1) {
      //primitiva ACCEPT
      struct sockaddr_in client_addr;
      socklen_t client_addr_size = sizeof client_addr;
      int new_fd = accept(
	     server_socket, 
		 (struct sockaddr *)&client_addr, &client_addr_size
      );
   
      if (new_fd == -1) {
            perror("accept");
            continue;
      }

      inet_ntop(client_addr.sin_family,
            get_in_addr((struct sockaddr *)&client_addr),
            saddr, sizeof saddr);
      printf("Server: got connection from %s\n", saddr);

                       
      if (!fork()) { // this is the child process
         close(server_socket); // child doesn't need the listener
         
         char* data = (char*)malloc(MAX_MSG_SIZE);         
         while ( strcmp(data, "QUIT") !=0 ) {
            //primitiva RECEIVE
            
            int data_size = MAX_MSG_SIZE;
            int received_data_size = recv(new_fd, data, data_size, 0);
                                 
            int i;
            for (i = 0; i < received_data_size; i++) {
               data[i] = toupper(data[i]);
            }                  
            for (i = received_data_size; i < MAX_MSG_SIZE; i++) {
               data[i] = '\0';
            }

            printf("Recibido del cliente (%d bytes): %s\n", received_data_size, data);

            if (strcmp(data, "QUIT") !=0) {
               //primitiva SEND      
               int sent_data_size = send(new_fd, data, received_data_size, 0);
               if (sent_data_size == -1) {
                  perror("send");
               }
               printf("Enviado al cliente (%d bytes): %s\n", sent_data_size, data);
            }
         }

         for (int i = 0; i < MAX_MSG_SIZE; i++) {
            data[i] = '\0';
         }

         close(new_fd);
         exit(0);
      }
      //primitiva CLOSE
      close(new_fd);// parent doesn't need this
   }

   //CLOSE del socket que espera conexiones
   close(server_socket);
}
