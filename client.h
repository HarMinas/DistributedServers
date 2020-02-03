//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project


#ifndef client
#define client

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <cstdlib>
#include <vector>

using namespace std;


void displayResults(string, string, vector<string>);

//Method headers.

//returns the struct sockaddr_in that contains the server address.
sockaddr_in getAddressInfo(int, const char *, sockaddr_in);
int createSocket(int, int);

//sends a message through a socket .
int sendTCP(int, vector<string>);
bool sendTCPA(int, string);
int sendTCPString(int, string );
//recieve a message from a socket
vector<string> receiveTCP(int);
string receiveTCPA(int);
string receiveTCPString(int);

#endif