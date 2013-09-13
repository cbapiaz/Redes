#include<stdio.h>
#include<stdlib.h>
#include <string>

#include "clientItem.hh"


using namespace std;



#define MAX_FILES 1500

typedef struct FileDescriptor
{
  	string name;
  	string md5;
  	int fd;
  	int size;
} FileDescriptor;


typedef struct client
{
        	string ip;
			string port;
			FileDescriptor files[MAX_FILES];
			int nr_files;
} client;


#define BASE_DIR "share/"
/** create a new client */
client * client_create (string ip, string port)
{
	client* cli = new client;
	cli->ip = ip;
	cli->port = port; 
	cli->nr_files = 0;
	return cli;
}

/** get client ip */
string client_getcIp (client *cli){
	return cli->ip;
}

/** get client port */
string client_getcPort (client *cli){
	return cli->port;
}

/** destroy client */
void client_destroy (client *cli){
	delete cli;
} 


void share_file(client *cli,string file) {
	if (cli != NULL) {		 
		 int fd; unsigned long size;
		 string _md5 = getMD5(BASE_DIR + file,fd,size);

   	     if (_md5.size() > 0) {
			 cli->files[cli->nr_files].name = file;			
			 cli->files[cli->nr_files].md5 = _md5;
			 cli->files[cli->nr_files].fd = fd;
			 cli->files[cli->nr_files].size = size;

			 cli->nr_files++;
		 }
		 else {
		 	perror("\nCannot share file, error calculating md5\n");
		 }
	}
	else perror ("share_file: client should not be null");
}

