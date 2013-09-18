#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h> //inet_addr
#include <unistd.h> //close

#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include <vector>

#include <string>

#include "clientItem.hh"

#include "connect_library.cc"

#include "Util.hh"

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



client * this_is_me;

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

void get_all_buf2(int sock, std::string & inStr, int &totalSize) {
    int n = 1, total = 0, found = 0;
    char c;
    char temp[1024*1024];

    // Keep reading up to a '\n'
    while (!found) {
        n = recv(sock, &temp[total], sizeof(temp) - total - 1, 0);
        if (n == -1) {
            /* Error, check 'errno' for more details */
            break;
        }
        total += n;
        temp[total] = '\0';
        found = (strchr(temp, '\n') != 0);
    }

    totalSize = total-2;
    inStr = temp;
    inStr = inStr.substr(0, inStr.size()-2); //all characters except the line break
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


	//Creo la entrada[2] de my_fds con el socket de la conexión con el tracker
	curr = (struct pollfd*) malloc (sizeof(struct pollfd));
    curr->fd = serv_socket;
    curr->events = POLLIN;
    curr->revents = 0;

    my_fds[2] = *curr;


    //Inicializo la cantidad de fds en 3
    num_fds = 3;

    // buff = (char*)malloc(MAX_BUFF_SIZE);         
     string auxConsole="";

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
         //printf("calling poll (%d sockets)\n\r", num_fds);
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
    					
    					cout << "Agrego el socket de la consola a las posición " <<  num_fds << "\n";					
    					
    					//Add it to the poll call
    					my_fds[num_fds] = *new_conn;
    					console = num_fds;
    					
    					num_fds++;
    			 }


    			 if ((i == 2) && (curr->revents != 0)) {
					printf("Nueva respuesta del tracker\n\r");    		

					char data[MAX_MSG_SIZE];
                	int r_data_size = recv(my_fds[2].fd, data, MAX_MSG_SIZE, 0);
                	data[r_data_size]='\0';
                	cout<<"El tracker responde:"<< data<<"\n";
    			 }
    			 
    				

    			 if ((i == console) && (curr->revents != 0))
    			 { 
                      
	              
	              // para atender a la consola
	              /*string out;              
	              int size;
	    		  get_all_buf2(curr->fd,out,size);*/

	    		  char data[MAX_BUFF_SIZE];
		          int size = recv(curr->fd, data, MAX_BUFF_SIZE, 0);
		          data[size]='\0';
		          string out(data);

		          auxConsole = auxConsole + out;

	              if (size <=0){
	                 printf("Console size<=0, algun error\n");
	                 eliminar = true;
	                 order_fds = true;
	              }
	              else if (strchr(data,'\n')!=0) {
	              		  
						  auxConsole = auxConsole.size() > 2 ? auxConsole.substr(0, auxConsole.size()-2) : auxConsole;

        				  splitstring s(auxConsole);
        				  
        				  /*auxConsole = auxConsole.replace( auxConsole.begin(), auxConsole.end(), '\n', ''); 
        				  auxConsole = auxConsole.replace( auxConsole.begin(), auxConsole.end(), '\r', ''); */

        				  cout <<auxConsole.size()<<"-console msg:$"<< auxConsole<<"$\n";


		                  vector<string> splitV = s.split(' ',1);
		                  //if (splitV.size() > 0) {

			                  string command = splitV[0];
 								

        				  
			                  if (command.find("show") != std::string::npos) { //show command
			                    string param =splitV.size() > 1 ? splitV[1] : "";
			                    cout <<"param:#"<<param<<"#\n";
			                    if (param.size() > 0) {

			                      if (param.find("share") != std::string::npos) { //"show share" muestra todos los archivos compartidos y los bytes transmitidos
			                          print_shared_files(this_is_me);
			                      }
			                      else if (param.find("downloads") != std::string::npos) { //"show downloads" muestra todas las descargas actuales para este cliente, junto con la dir del uploader y bytes descargados
			                          print_downloads(this_is_me);
			                      }
			                      else if (param.find("uploads") != std::string::npos) { //"show uploads" muestra todas las cargas en progreso, junto con la dirección del downloader y los bytes entregados
			                          print_uploads(this_is_me);
			                      }
			                      else perror("Invalid argument for show command");                        


			                    }
			                    else perror("Not enough arguments, need to specify what to show");                        
			                  }

			                  if (command.find("share") != std::string::npos) { //share command			                    
			                    //cout <<"console command:"<<splitV.size()<<"-"<<command<<"\n";
			                    string file =splitV.size() > 1 ? splitV[1] : "";

			                    if (file.size() > 0) {
			                      
			                      cout << "file to share:@"<<file<<"@\n";
			                      share_file(this_is_me,file);                          
			                    }
			                    else perror("Not enough arguments, need to specify file to share");                        
			                  }

			        				  /*cout << "pedido de consola" << "\n"; 
			        				  cout << "comando: " << out <<"\n";                      */

			                  if (auxConsole.compare("quit")==0) {         
			                    //delete this_is_me;   
			                    cout << "Cerrando cliente.. \n";
			                    exit(0);
			                  }
	                                              
	        				  // en out queda gurdado lo que recibo por telnet
	        				  				  
	        				  //envio al servidor lo que me llego por telnet				

	        				  char *toSend = (char*)auxConsole.c_str();
	        				  send(serv_socket, toSend, strlen(toSend), 0);
	              
			                  //recibo la rspuesta del sevidor,por ahora es un numero de puerto
			                  /*buff_sz = recv(serv_socket, &buff2, 254, 0);          
			                  
			                  //me conecto con el cliente
			                  socket_client = connect_socket (HOST,buff2);
			                  
			                  //agrego  el nuevo cliente a my_fds
			                  
			                  sin_size = sizeof their_addr;
			                  new_conn = (struct pollfd*) malloc(sizeof(struct pollfd));
			                  new_conn->fd = socket_client;
			                  new_conn->events = POLLOUT;
			                  new_conn->revents = 0;
			                  
			                  my_fds[num_fds] = *new_conn;
			                  num_fds++;*/

			                  auxConsole = "";
		              	/*} else  {
		              		printf("Invalid Console Command\n");
		              	}*/
	              }

			   }
    			   
			  if ((i != console) && (i > 2) && (curr->revents != 0))
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
			     
            if (eliminar) {
              curr->fd = -1;
            }

            eliminar = false;  
         }
     }
}

string getMyIP() {  
  return "127.0.0.1";
}

int main(int argc, char *argv[])
{
    int sockfd,port_accept,port_console;


    if (argc < 5) {
       fprintf(stderr,"usage %s hostname hostport acceptport consoleport\n", argv[0]);
       exit(0);
    }
    
	  printf("Comienza cliente \n");
	  printf("Me conecto al servidor \n");
    
    this_is_me = client_create(HOST,argv[2]);

    //connect to tracker
    sockfd = connect_socket(HOST,argv[2]) ;    

    char buff[MAX_BUFF_SIZE];    
    sprintf(buff, "NEWCLIENT\n%s:%s\r\n", getMyIP().c_str(),argv[3]);

    cout<<"message to send:"<<buff<<"\n";
    send(sockfd, buff, strlen(buff), 0);    

    printf("Conectado con el servidor %s:%s \n\n",argv[3],argv[4]);
    
    /*port_accept = atoi(argv[3]);
    port_console = atoi(argv[4]);*/
    
    port_accept = (int)strtol(argv[3],NULL,10);
    port_console = (int)strtol(argv[4],NULL,10);

	printf("hola hola si");

    cout<<"Conectado con el servidor2: "<<port_accept<<":"<<port_console<<"\n";


	processPeerToPeer(port_accept,port_console,sockfd);

   
}
