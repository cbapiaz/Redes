#ifndef CLIENTITEM_H
#define CLIENTITEM_H

#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <map>

#include "fileHelper.hh"
using namespace std;


typedef struct fileDescriptor fileDescriptor;

typedef struct client client;

/** create a new client */
client * client_create (string ip, string port);

/** get client ip */
string client_getcIp (client *cli);

/** get client port */
string client_getcPort (client *cli);

/** destroy client */
void client_destroy (client *cli); 

void publish_file(client *cli,string file,string _md5);
string search_file(map<int,client*> &clients,string file);

void share_file(client *cli,string file);

void print_shared_files(client * cli);
void print_downloads(client * cli);
void print_uploads(client * cli);



#endif 
