#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "3490"
#define HOST "localhost"
#define MAXLEN 1024

int main(void)
{
   //primitiva SOCKET
   int client_socket = socket(AF_INET, SOCK_STREAM, 0);
   
   //obtenemos la direccion con getaddrinfo
   struct addrinfo hints, *res;
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   getaddrinfo(HOST, PORT, &hints, &res);
   
   //primitiva CONNECT
   connect(client_socket, res->ai_addr, res->ai_addrlen);
   
   //primitiva SEND
   char *msg = "Mensaje de prueba";
   int msg_size = strlen(msg);
   int sent_msg_size = send(client_socket, msg, msg_size, 0);
   
   printf("Enviado al servidor (%d bytes): %s\n", sent_msg_size, msg);
   
   //primitiva RECEIVE
   char* data = malloc(MAXLEN);
   int data_size = MAXLEN;
   int received_data_size = recv(client_socket, data, data_size, 0);
   
   printf("Recibido del servidor (%d bytes): %s\n", received_data_size, data);
   
   //primitiva CLOSE
   close(client_socket);
   
   freeaddrinfo(res);
}
