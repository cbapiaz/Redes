#ifndef CLIENTITEM_H
#define CLIENTITEM_H

#include<stdio.h>
#include<stdlib.h>
#include <string>

#include "fileHelper.hh"
using namespace std;


typedef struct FileDescriptor FileDescriptor;

typedef struct client client;

/** create a new client */
client * client_create (string ip, string port);

/** get client ip */
string client_getcIp (client *cli);

/** get client port */
string client_getcPort (client *cli);

/** destroy client */
void client_destroy (client *cli); 

void share_file(client *cli,string file);

#endif 
