//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project
#ifndef server
#define server

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

//Methods.

int createSocket(int, int);
void bindSocket(int, sockaddr_in);


//returns the struct sockaddr_in that contains the server address.
sockaddr_in getAddressInfo(int, sockaddr_in);


//Assisting in result processing.
void printReceiptMessage(string, string, int, vector<string>);
vector<string> resutlsForMonitor(string, vector<string>, vector<string>, vector<string>);
vector<string> resutlsForClient(string, vector<string>, vector<string>, vector<string>);

//functions to help with sending over UDP.
int sendUDP(int, sockaddr*, vector<string>);
bool sendUDPA(int, sockaddr*, string);
int sendUDPString(int, sockaddr*, string);

//functions to help with receiving over UDP.
vector<string> receiveUDP(int, sockaddr*);
string receiveUDPA(int, sockaddr*);
string receiveUDPString(int, sockaddr*);


//sends a message through a socket over TCP .
int sendTCP(int, vector<string>);
bool sendTCPA(int, string);
int sendTCPString(int, string );
//recieve a message from a socket over TCP
vector<string> receiveTCP(int);
string receiveTCPA(int);
string receiveTCPString(int);


//Print the string thats passed in. 
void print(string);
//prints an int thats passed in.
void print(int);

#endif 