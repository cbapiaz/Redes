#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>


#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <unistd.h> //close

#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include <string>
#include <iostream>
using namespace std;

#define PORT "3490"
#define HOST "localhost"

#define PORT_ACCEPT 3491

#define MAXLEN 1024

#define MAX_CONN 10

#define MAX_LISTEN 10

#define MILLISECONDS 1000
#define TIMEOUT (30 * MILLISECONDS)

#define MAX_BUFF_SIZE 255

static int listen_socket(int port)
{
    struct sockaddr_in a;
    int s;
    int yes;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    yes = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
            (char *) &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(s);
        return -1;
    }
    memset(&a, 0, sizeof(a));
    a.sin_port = htons(port);
    a.sin_family = AF_INET;
    if (bind(s, (struct sockaddr *) &a, sizeof(a)) < 0) {
        perror("bind");
        close(s);
        return -1;
    }
    printf("Accepting connections on port %d\n", port);
    listen(s, MAX_LISTEN);    

    return s;
}

void processPeerToPeer(int port) {
   //struct pollfd **my_fds;                  //array of pollfd structures for poll()
   //struct pollfd **my_fds;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i, j;                                //for loops
   char buff[MAX_BUFF_SIZE], buff2[MAX_BUFF_SIZE];              //for sending and recieving text
   struct sockaddr_in my_addr, their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;                             //size of data recieved

   printf("Peer Started \n");
   

    //I call listen_socket() which creates a socket to listen to
    //this is anchored into my_fds array at element 0.
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;


    //num_fds, the count of items in the array is set to 1
    //because the listen socket is already present
    num_fds = 1;

     while (num_fds < MAX_CONN)
     {
     
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
         printf("calling poll (%d sockets)\n\r", num_fds);
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
            printf("We have a new connection.\n\rAccept goes here...\n\r");

            //Accept the connection
            sin_size = sizeof their_addr;
            new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
            new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
            new_conn->events = POLLIN;
            new_conn->revents = 0;

            printf("Connection from %s\n\r", inet_ntoa(their_addr.sin_addr));
            sprintf(buff, "Your %i\n\r", num_fds);
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
                  printf("Recieved: %s", buff);

                  //send the message to everyone else
                  for (j = 1; j < num_fds; j++)
                  {
                     printf("i = %i, j = %i\n", i, j);
                     if (j != i)
                     {
                        new_conn = &my_fds[j];
                        sprintf(buff2, "%i sent you %i: %s", i, j, buff);
                        send(new_conn->fd, buff2, strlen(buff2) + 1, 0);
                     }
                  }
                  printf("\n\r");
               }
            }
         }
     }
}

int main(int argc , char *argv[])
{

   char* host = (char*)HOST;
   char* port = (char*)PORT;      

   int port_accept = PORT_ACCEPT;
   if (argc > 1) {
      port_accept = atoi(argv[1]);       
      /*host = argv[0];
      if (argc > 1) {         
         port=argv[1];
      }*/
   }



   if (!fork()) { // process to attend client to client connection, peer to peer
        //printf("port inner %i", port_accept);
        processPeerToPeer(port_accept);
   }


   //primitiva SOCKET
   int client_socket = socket(AF_INET, SOCK_STREAM, 0);
   
   //obtenemos la direccion con getaddrinfo
   struct addrinfo hints, *res;
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   getaddrinfo(host, port, &hints, &res);
   
   //primitiva CONNECT
   connect(client_socket, res->ai_addr, res->ai_addrlen);
      
   string msg="";   


   while (msg.compare("quit") != 0) {      

      getline (cin, msg);

      int msg_size = strlen((char*)msg.c_str());
      int sent_msg_size = send(client_socket, (char*)msg.c_str(), msg_size, 0);
      
      printf("Enviado al servidor (%d bytes): %s\n", sent_msg_size, (char*)msg.c_str());
      
      //primitiva RECEIVE
      char* data = (char*)malloc(MAXLEN);
      int data_size = MAXLEN;
      int received_data_size = recv(client_socket, data, data_size, 0);
      
      printf("Recibido del servidor (%d bytes): %s\n", received_data_size, data);


      
   }
   //primitiva CLOSE
   close(client_socket);
   
   freeaddrinfo(res);
}
