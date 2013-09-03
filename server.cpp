#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT 3490
#define MY_IP "127.0.0.1"
#define MAX_QUEUE 10
#define MAX_MSG_SIZE 1024

int main(void)
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
   
   while (1) {
      //primitiva ACCEPT
      struct sockaddr_in client_addr;
      socklen_t client_addr_size = sizeof client_addr;
      int socket_to_client = accept(
	     server_socket, 
		 (struct sockaddr *)&client_addr, &client_addr_size
      );
   
      //primitiva RECEIVE
      char* data = malloc(MAX_MSG_SIZE);
      int data_size = MAX_MSG_SIZE;
      int received_data_size = recv(socket_to_client, data, data_size, 0);
      
      printf("Recibido del cliente (%d bytes): %s\n", received_data_size, data);
      
      int i;
      for (i = 0; i < received_data_size; i++) {
         data[i] = toupper(data[i]);
      }
      
      //primitiva SEND
      int sent_data_size = send(socket_to_client, data, received_data_size, 0);
      printf("Enviado al cliente (%d bytes): %s\n", sent_data_size, data);
      
      //primitiva CLOSE
      close(socket_to_client);
   }

   //CLOSE del socket que espera conexiones
   close(server_socket);
}
