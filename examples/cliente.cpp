#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

#include <poll.h>

using namespace std;

#define LISTEN_PORT 5555
#define SERVER_PORT "5556"
#define MY_IP "127.0.0.1"
#define HOST "localhost"
#define MAX_QUEUE 10
#define MAX_MSG_SIZE 1024

#define MAX_CONN 10
#define MAX_BUFF_SIZE 255

#define TIMEOUT (30000)

int main(void)
{

//BEGIN-SOCKET TO LISTEN
   printf("CLIENTE: Ingrese el puerto para escuchar: ");
   string in_listen_port="";
   getline (cin, in_listen_port);
   short listen_port = atoi(in_listen_port.c_str());
   int listen_socket = socket(AF_INET, SOCK_STREAM, 0);

   //BIND
   struct sockaddr_in listen_addr;
   socklen_t listen_addr_size = sizeof listen_addr;
   listen_addr.sin_family = AF_INET;
   listen_addr.sin_port = htons(listen_port);
   listen_addr.sin_addr.s_addr = inet_addr(MY_IP);
   bind(
      listen_socket, 
      (struct sockaddr*)&listen_addr, listen_addr_size
   );

   //LISTEN
   listen(listen_socket, MAX_QUEUE);
   printf("Estoy escuchando en el puerto: %d\n", listen_port);
//END-SOCKET TO LISTEN

//BEGIN-SOCKET TO SERVER
   int client_socket = socket(AF_INET, SOCK_STREAM, 0);

   struct addrinfo hints, *res;
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   getaddrinfo(HOST, SERVER_PORT, &hints, &res);

   //CONNECT
   connect(client_socket, res->ai_addr, res->ai_addrlen);

   //SEND
   printf("Ingrese el mensaje a enviar al servidor: ");
   string in_msg="";
   getline (cin, in_msg);
   char *msg = (char*)in_msg.c_str();
   int msg_size = strlen(msg);
   int sent_msg_size = send(client_socket, msg, msg_size, 0);
   printf("Mensaje enviado al servidor (%d bytes): %s\n", sent_msg_size, msg);

   //RECEIVE
   char* data = (char*)malloc(MAX_MSG_SIZE);
   int data_size = MAX_MSG_SIZE;
   int received_data_size = recv(client_socket, data, data_size, 0);
   printf("Mensaje recibido del servidor (%d bytes): %s\n", received_data_size, data);

   //CLOSE
   //close(client_socket);
   //freeaddrinfo(res);
//END-SOCKET TO SERVER

//BEGIN-SOCKET TO par
   int p2p_socket = socket(AF_INET, SOCK_STREAM, 0);

   printf("CLIENTE: Ingrese el puerto del par a transferir: ");
   string in_p2p_port="";
   getline (cin, in_p2p_port);
   char *p2p_port = (char*)in_p2p_port.c_str();
   getaddrinfo(HOST, p2p_port, &hints, &res);

   //CONNECT
   connect(p2p_socket, res->ai_addr, res->ai_addrlen);

   //SEND
   printf("Ingrese el mensaje a transferir al par: ");
   in_msg="";
   getline (cin, in_msg);
   msg = (char*)in_msg.c_str();
   msg_size = strlen(msg);
   sent_msg_size = send(p2p_socket, msg, msg_size, 0);
   printf("Mensaje transferido al par (%d bytes): %s\n", sent_msg_size, msg);
//END-SOCKET TO par




   int port = atoi(p2p_port);
   struct pollfd my_fds[MAX_CONN];
   struct pollfd *curr, *new_conn;
   int num_fds=0;

   char buff[MAX_BUFF_SIZE], buff2[MAX_BUFF_SIZE];
   struct sockaddr_in my_addr, their_addr;
   socklen_t sin_size;
   int buff_sz;
   int i, j;

    //En el elemento 0 socket que escucha por nuevas conexiones
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket;
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;

    num_fds = 1;

    while (num_fds < MAX_CONN) {
         //reset all event flag
         for (i = 1; i < num_fds; i++)
         {
            curr = &my_fds[i];
            curr->events = POLLIN | POLLPRI;
            printf("%i: fd %i\n\r", i, curr->fd);
            curr->revents = 0;
            //send(curr->fd, "Enter some text:\n", 18, 0);
         }
         //put all this into poll and wait for something magical to happen
         printf("Invocando al poll con: (%d sockets)\n\r", num_fds);
         if (poll(my_fds, num_fds, TIMEOUT) == -1)
         {
            perror("poll");
            exit(0);
         }

         //printf("poll returned!\n");

         //First item is the accepting socket....check it independently of the rest!
         curr = &my_fds[0];
         if (curr->revents != 0)
         {
            printf("Nueva conexión desde par\n\rVamos a aceptarla...\n\r");

            //Accept the connection
            sin_size = sizeof their_addr;
            new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
            new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
            new_conn->events = POLLIN;
            new_conn->revents = 0;

            printf("Conexión desde par: %s\n\r", inet_ntoa(their_addr.sin_addr));
            sprintf(buff, "Nfds %i\n\r", num_fds);
            send(new_conn->fd, buff, 7, 0);

            //Add it to the poll call
            my_fds[num_fds] = *new_conn;
            num_fds++;

         }
         else
         {
            //skip first one, we know that's the accepting socket (handled above).
            for (i = 1; i < num_fds; i++)
            {
               curr = &my_fds[i];
               if (curr->revents != 0)
               {
                  buff_sz = recv(curr->fd, &buff, MAX_BUFF_SIZE, 0);
                  buff[buff_sz] = '\0';
                  printf("Mensaje recibido del par: %s", buff);

                  //send the message to everyone else
//                  for (j = 1; j < num_fds; j++)
//                  {
//                     printf("i = %i, j = %i\n", i, j);
//                     if (j != i)
//                     {
//                        new_conn = &my_fds[j];
//                        sprintf(buff2, "%i sent you %i: %s", i, j, buff);
//                        send(new_conn->fd, buff2, strlen(buff2) + 1, 0);
//                     }
//                  }
                  printf("\n\r");
               }
            }
         }
     }
}
