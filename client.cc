#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>


#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <unistd.h> //close

#include <string>
#include <iostream>
using namespace std;

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
   string msg; /*const  = "Mensaje de prueba"*/   
//   cin>> msg;
   getline (cin, msg);

   int msg_size = strlen((char*)msg.c_str());
   int sent_msg_size = send(client_socket, (char*)msg.c_str(), msg_size, 0);
   
   printf("Enviado al servidor (%d bytes): %s\n", sent_msg_size, (char*)msg.c_str());
   
   //primitiva RECEIVE
   char* data = (char*)malloc(MAXLEN);
   int data_size = MAXLEN;
   int received_data_size = recv(client_socket, data, data_size, 0);
   
   printf("Recibido del servidor (%d bytes): %s\n", received_data_size, data);
   
   //primitiva CLOSE
   close(client_socket);
   
   freeaddrinfo(res);
}
