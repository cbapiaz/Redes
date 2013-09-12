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

#include <map>

#include <string>
#include <iostream>
#include <sstream>

#include <openssl/md5.h>

using namespace std;

#include "clientItem.hh"

#define PORT "3490"
#define HOST "localhost"

#define PORT_ACCEPT 5555
#define PORT_CONSOLE 6666

#define MAXLEN 1024

#define MAX_CONN 10

#define MAX_LISTEN 10

#define SECOND 1000
#define TIMEOUT (30 * SECOND)

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void print_clients (map<string, client*> m){
	  cout << "Los clientes registrados son \n";
    int i =0;
    for(std::map<string, client*>::const_iterator it = m.begin(); it != m.end(); it++)
    {
    	cout << "Client Nr "<<i<<": "<< client_getcIp(it->second) << "@" << client_getcPort(it->second) <<  "\n"; ;
    	//Do something
      i++;
    }
}


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


void pollserver(int port_accept) {   
   
   bool order_fds;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i, j;                                //for loops
   char buff[255], buff2[255];              //for sending and recieving text
   struct sockaddr_in my_addr, their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;
   string ip;                             //size of data recieved
   string port; 
   order_fds = false;    
   map<string, client*> clients;  
   
    
    //Creo entrada[0] de my_fds con el socket de la conexión con el cliente
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_accept);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;


    //Inicializo la cantidad de fds en 2
    num_fds = 1;

     while (num_fds < MAX_CONN)
     {
     
         //reset all event flag
         for (i = 1; i < num_fds; i++)
         {
            curr = &my_fds[i];
            curr->events = POLLIN | POLLPRI;
            //printf("%i: fd %i\n\r", i, curr->fd);
            curr->revents = 0;
            //send(curr->fd, "Enter some text:\n", 18, 0);
         }         
         //put all this into poll and wait for something magical to happen
         printf("calling poll (%d sockets)\n\r", num_fds);
         if (poll(my_fds, num_fds, -1) == -1)
         {
            perror("poll");
            exit(0);
         }
          //printf("poll returned!\n");
        for (i = 0; i < num_fds; i++)
        {
			 curr = &my_fds[i];	
 		 
			 if ((i == 0) && (curr->revents != 0))
			 {
				printf("Hay nuevo cliente\n\r");

				//Accept the connection
				sin_size = sizeof their_addr;
				new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
				new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
				new_conn->events = POLLIN;
				new_conn->revents = 0;

				

				//Add it to the poll call
				my_fds[num_fds] = *new_conn;
				num_fds++;	
			
				ip = inet_ntoa(their_addr.sin_addr);
				
				std::stringstream out;
				out << ntohs(their_addr.sin_port);
				port = out.str();				
				
				clients[port + ip] = client_create(ip,port);
				//ver donde copiar
				//ver el concat que copia
				print_clients(clients);
				
			 }

			 else if ((i > 0) && (curr->revents != 0))
			   {
				  printf("Recibo pedido cliente\n"); 
				  buff_sz = recv(curr->fd, &buff, 254, 0);
				  buff[buff_sz] = '\0';
				  printf("Envio respuesta cliente\n");
				  send(curr->fd, buff, strlen(buff) + 1, 0);
				  printf("\n\r");
			   }
			
         }
     }
}


int main(int argc, char *argv[])
{
   char buff[255], buff2[255];              //for sending and recieving text
   struct sockaddr_in my_addr, their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;                             //size of data recieved	
	
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;

     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);     
     
     pollserver(portno);
         
}
