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
   printf("TRACKER: Ingrese el puerto para escuchar: ");
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

   int port = listen_port;
   struct pollfd my_fds[MAX_CONN];
   struct pollfd *curr, *new_conn;
   int num_fds=0;			//cantidad de sockets

   char buff[MAX_BUFF_SIZE], buff2[MAX_BUFF_SIZE];
   struct sockaddr_in my_addr, their_addr;
   socklen_t sin_size;
   int buff_sz;				//size of data received
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
            printf("Nueva conexión\n\rVamos a aceptarla...\n\r");

            //Accept the connection
            sin_size = sizeof their_addr;
            new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
            new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
            new_conn->events = POLLIN;
            new_conn->revents = 0;

            printf("Conexión desde: %s\n\r", inet_ntoa(their_addr.sin_addr));
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
                  printf("Recibido: %s", buff);

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
