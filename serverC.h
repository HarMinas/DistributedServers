//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project
#ifndef serverC
#define serverC

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
#include <fstream>

using namespace std;




//Functions that help with performing server specific operations.
vector<string> prefixOrSuffix(string, ifstream&, string, string);
vector<string> searchForSimilar(ifstream&, string, string);
string searchForDefin(ifstream&, string, string);

void performOperation(string, string, vector<string>* , ifstream&, string);


//converting to lower case.
void converToLower(string&);

//functions to help with sending over UDP.
int sendUDP(int, sockaddr*, vector<string>);
bool sendUDPA(int, sockaddr*, string);
int sendUDPString(int, sockaddr*, string);

//functions to help with receiving over UDP.
vector<string> receiveUDP(int, sockaddr*);
string receiveUDPA(int, sockaddr*);
string receiveUDPString(int, sockaddr*);


//functions to help with creating a UDP socket.
sockaddr_in getAddressInfo(int portNum, const char * serverIP, sockaddr_in serverInfo);
void bindSocket(int, sockaddr_in);
int createSocket(int, int);





 #endif