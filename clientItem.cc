#include<stdio.h>
#include<stdlib.h>
#include <string>

#include "clientItem.hh"

using namespace std;

typedef struct client
{
        	string ip;
			string port;
}client;

/** create a new client */
client * client_create (string ip, string port)
{
	client* cli = new client;
	cli->ip = ip;
	cli->port = port; 
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

