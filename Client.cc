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

#include "connect_library.cc"

#define PORT "3490"
#define HOST "localhost"

#define PORT_ACCEPT 5555
#define PORT_CONSOLE 6666

#define MAXLEN 1024

#define MAX_CONN 10

#define MAX_LISTEN 10

#define SECOND 1000
#define TIMEOUT (30 * SECOND)

int new_console_pos (struct pollfd fds[MAX_CONN],int nfds, int fd)
{
  int i;
  int pos = -1;
  bool encontre = false;	
  for (i = 0; (i < nfds) && (!encontre); i++)
  {
	if (fds[i].fd == fd)
	{
		encontre = true;
		pos = i;
	}
  }
  return pos;	
} 

void processPeerToPeer(int port_accept,int port_console,int serv_socket) {
   
   bool eliminar = false;
   bool order_fds = false;
   int console,console_fds;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i;                                //for loops
   char buff[MAX_BUFF_SIZE];
   char buff2[MAX_BUFF_SIZE], buff3[MAX_BUFF_SIZE];              //for sending and recieving text
   struct sockaddr_in their_addr;  // my address information
   socklen_t sin_size;
   int buff_sz;                             //size of data recieved
   int socket_client;
   console = -1;
   order_fds = false;      
   
    
    //Creo entrada[0] de my_fds con el socket de la conexión con el cliente
    curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_accept);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[0] = *curr;
	
	//Creo la entrada[1] de my_fds con el socket de la conexión con la consola
	curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = listen_socket(port_console);
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[1] = *curr;


    //Inicializo la cantidad de fds en 2
    num_fds = 2;

    // buff = (char*)malloc(MAX_BUFF_SIZE);         

     while (num_fds < MAX_CONN)
     {
		 //reordeno my_fds
		 if (order_fds)
		 {
			order_fds = false; 		 
			num_fds = order(my_fds, num_fds);
			cout << "ordeno" << "\n";
			if (console != -1)
				console = new_console_pos(my_fds,num_fds,console_fds);
		 }
         //reset all event flag
         //for (i = 2; i < num_fds; i++)
         //{
         //   curr = &my_fds[i];
         //   curr->events = POLLIN;
         //   curr->revents = 0;
         //}         
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
				//seguir ver receive
				//Accept the connection
				sin_size = sizeof their_addr;
				new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
				new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
				
		          if (new_conn->fd < 0)
		          {
		              perror("  accept() failed");
		              //end_server = TRUE;
		          }			
				
				new_conn->events = POLLIN;
				new_conn->revents = 0;

				//Add it to the poll call
				my_fds[num_fds] = *new_conn;
				num_fds++;
			 }
			 
			 if ((i == 1) && (curr->revents != 0))
				 {
					printf("Inicio consola\n\r");

					//Accept the connection
					sin_size = sizeof their_addr;
					new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
					new_conn->fd = accept(curr->fd, (struct sockaddr *)&their_addr, &sin_size);
					new_conn->events = POLLIN;
					new_conn->revents = 0;

		          if (new_conn->fd < 0)
		          {
		              perror("  accept() failed");
		              //end_server = TRUE;
		          }	
					
					cout << "Agrego el socket de la consola a las posición " <<  num_fds << "\n";					
					
					//Add it to the poll call
					my_fds[num_fds] = *new_conn;
					console = num_fds;
					console_fds = new_conn->fd;
					num_fds++;

				 }

			 if ((i == console) && (curr->revents != 0))
			   { 
				  cout << "revents onsola " << curr->revents << "\n"; 
				  if ((curr->revents != POLLIN)) 
				  {
					  printf("algun error\n");
					  eliminar = true;
					  order_fds = true;
				  }
				  else
				  {                  
                  // para atender a la consola
                  //string out;
                  //while (out.compare("quit")!=0) {
                  //    int size ;
    			 	//  get_all_buf(curr->fd,out,size);                   
                      
                  //}
                  
                  
                      buff_sz = recv(curr->fd, buff, sizeof(buff), 0);
                      if (buff_sz <= 0)

					  {
						  printf("algun error\n");
						  eliminar = true;
						  order_fds = true;
					  }
					  else
					  {                      
	    				  buff[buff_sz] = '\0';
	    				  
	    				  cout << "pedido de consola" << "\n"; 
	    				  cout << "comando: " << buff <<"\n";  
	                  
						  // en buff queda gurdado lo que recibo por telnet
						  
						  
						  //envio al servidor lo que me llego por telnet
						  
		
		
						  send(serv_socket, buff, strlen(buff) + 1, 0);
						  
						  //recibo la rspuesta del sevidor,por ahora es un numero de puerto
						  buff_sz = recv(serv_socket, &buff2, 254, 0);				  
						  
						  //me conecto con el cliente
						  socket_client = connect_socket (HOST,buff2);
						  
						  //agrego  el nuevo cliente a my_fds
						  
						  sin_size = sizeof their_addr;
						  new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
						  new_conn->fd = socket_client;
						  new_conn->events = POLLOUT;
						  new_conn->revents = 0;
						  
					      my_fds[num_fds] = *new_conn;
						  num_fds++;
						  
					  }
			     }
			   }
			   
			   if ((i != console) && (i > 1) && (curr->revents != 0))
			   {
				  cout << "revents cliente " << curr->revents << "\n"; 
				  if ((curr->revents != POLLIN) && (curr->revents != POLLOUT)) 
				  {
					  printf("algun error\n");
					  eliminar = true;
					  order_fds = true;; 
				  }
				  else
				  {
					if (curr->revents == POLLIN) 
					{					  		
						  //Recibo mi propio puerto de otro cliente
						  buff_sz = recv(curr->fd, &buff3, 254, 0);
						  if (buff_sz <= 0)	
						  {
							  printf("algun error\n");
							  eliminar = true;
							  order_fds = true;
						  }
						  else
						  {   		  				   
						  
						  cout << "Tamano buffer " << buff_sz << "\n"; 
						  buff[buff_sz] = '\0';
						  				  
						  cout << "Recibí un pedido de otro cliente" << "\n";
						  
						  cout << "Si mi puerto de escucha es :" << buff3 << "está bien\n";	  
						  }
					  }
					  else
					  {
						  //me comunico con el cliente
						  cout << "me conecte al cliente del puerto " << buff2 << "\n";		  
						  send(socket_client, buff, strlen(buff) + 1, 0);
						  eliminar = true;
						  order_fds = true;
						  close(socket_client);
					  }
				  } 			   
			   }
		 if (eliminar)	   
			   curr->fd = -1;
		 eliminar = false;			
         }
     }
}


int main(int argc, char *argv[])
{
    int sockfd,port_accept,port_console;
    //struct sockaddr_in serv_addr;
    //struct hostent *server;

    if (argc < 5) {
       fprintf(stderr,"usage %s hostname hostport acceptport consoleport\n", argv[0]);
       exit(0);
    }
    //portno = atoi(argv[2]);
    
	printf("Comienza cliente \n");
	printf("Me conecto al servidor \n");
   
    sockfd = connect_socket(HOST,argv[2]) ;    
    printf("Conectado con el servidor \n");
    
    port_accept = atoi(argv[3]);
    port_console = atoi(argv[4]);

	processPeerToPeer(port_accept,port_console,sockfd);

/* //primitiva CLOSE
   close(client_socket);
   freeaddrinfo(res);
   */
   
}
