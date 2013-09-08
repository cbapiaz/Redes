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
    exit(0);
}

static int connect_socket(const char* host, char* port)
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    return sockfd;
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

void order(struct pollfd fds[MAX_CONN], int nfds)
{
  int i,j;
  for (i = 0; i < nfds; i++)
  {
	if (fds[i].fd == -1)
	{
	  for(j = i; j < nfds; j++)
	  {
		fds[j].fd = fds[j+1].fd;
	  }
	  nfds--;
	}
  }
}


void processPeerToPeer(int port_accept,int port_console,int serv_socket) {
   
   bool order_fds;
   int console;
   struct pollfd my_fds[MAX_CONN];

   struct pollfd *curr, *new_conn;          //so I can loop through   
   int num_fds=0;                             //count of how many are being used
   int i;                                //for loops
   char buff[255], buff2[255], buff3[255];              //for sending and recieving text
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

     while (num_fds < MAX_CONN)
     {
     
         //reset all event flag
         for (i = 2; i < num_fds; i++)
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
					
					cout << "Agrego el socket de la consola a las posición " <<  num_fds << "\n";					
					
					//Add it to the poll call
					my_fds[num_fds] = *new_conn;
					console = num_fds;
					
					num_fds++;

				 }

			 if ((i == console) && (curr->revents != 0))
			   { 

				  buff_sz = recv(curr->fd, &buff, 254, 0);
				  buff[buff_sz] = '\0';
				  
				  cout << "pedido de consola" << "\n"; 
				  cout << "comando: " << buff << "\n";
				  // en buff queda gurdado lo que recibo por telnet
				  
				  
				  //envio al servidor lo que me llego por telnet
				  
				  send(serv_socket, buff, strlen(buff) + 1, 0);
				  
				  //recibo la rspuesta del sevidor,por ahora es un numero de puerto
				  buff_sz = recv(serv_socket, &buff2, 254, 0);				  
				  
				  //me conecto con el cliente
				  socket_client = connect_socket (HOST,buff2);
				  
				  //me comunico con el cliente
				  cout << "me conecte al cliente del puerto " << buff2 << "\n";		  
				  send(socket_client, buff, strlen(buff) + 1, 0);
			   }
			   
			   if ((i != console) && (i > 1) && (curr->revents != 0))
			   {
				  //Recibo mi propio puerto de otro cliente
				  buff_sz = recv(curr->fd, &buff3, 254, 0);
				  buff[buff_sz] = '\0';
				  				  
				  cout << "Recibí un pedido de otro cliente" << "\n";
				  
				  cout << "Si mi puerto de escucha es :" << buff3 << "está bien\n";	  
				   			   
			   }
			
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
