
//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project

#include "aws.h"

int main()
{

//Port numbers for different connections.
const int serverAPort = 21195;
const int serverBPort = 22195;
const int serverCPort = 23195;
const int udpPort = 24195;
const int clientTCPPort = 25195;
const int monTCPPort = 26195;

int UDPSocket, sockForClient, sockForMon;
   
//The structures to contain the addresses of different ports.
sockaddr_in serverAAddr, serverBAddr, serverCAddr, clientAddr, monitorAddr,  monTCPAddr, clientTCPAddr, awsUDPAddr;

  


//Storages for various messages
string command; //will hold the operation.
string  input; //will hold the target
vector<string> request; //Holds data to send to the 3 back end servers.
vector<string> rbServerA; //Holds data received from the serverA
vector<string> rbServerB; //Holds data received from the serverB
vector<string> rbServerC; //Holds data received from the serverC
vector<string> sendToMon; //Holds data to send to the monitor.
vector<string> sendToClient; //Holds data to send to the client.



//Creating the sockets for the AWS server
UDPSocket = createSocket(AF_INET, SOCK_DGRAM);
sockForClient = createSocket(AF_INET, SOCK_STREAM);
sockForMon = createSocket(AF_INET, SOCK_STREAM);

 

//Filling out the address structures 
serverAAddr = getAddressInfo(serverAPort, serverAAddr);
serverBAddr = getAddressInfo(serverBPort, serverBAddr);
serverCAddr = getAddressInfo(serverCPort, serverCAddr);
awsUDPAddr = getAddressInfo(udpPort, awsUDPAddr);
clientTCPAddr = getAddressInfo(clientTCPPort, clientTCPAddr);
monTCPAddr = getAddressInfo(monTCPPort, monTCPAddr);



//Binding the address to the socket. Doing error checking.
bindSocket(UDPSocket,awsUDPAddr);
bindSocket(sockForClient, clientTCPAddr);
bindSocket(sockForMon, monTCPAddr);


cout << "aws is up and running" << endl;


int monitorSocket; //socket for the monitor

///listening for client and monitor
listen(sockForMon, 2);
listen(sockForClient, 5);

socklen_t size = sizeof(clientAddr);
socklen_t size1 = sizeof(monitorAddr);

monitorSocket = accept(sockForMon, (sockaddr*) &monitorAddr, &size1);

    while(true) //An infinite loop to keep getting commands and replying to them.
    {

        int clientSocket; // Socket for the client

        clientSocket = accept(sockForClient, (sockaddr*)&clientAddr, &size); // receive the client call.

        int clientPort = 5;//ntohs(clientAddr.sin_port);
//Receiving the request from the client.
        request = receiveTCP(clientSocket); //Receiving data from the 

        command = request.at(0); //extracting the command
        input = request.at(1); //extracting the request.

cout << "The AWS received input=<" << input << "> and function <" << command << "> from client using TCP over port " << clientPort << endl;



////Sending the request to the backend servers and receiving the results.
        sendUDP(UDPSocket, (sockaddr*)&serverAAddr, request);
cout << "The AWS sent <"<< input << "> and <" << command << "> to Backend-Server A" << endl;
rbServerA = receiveUDP(UDPSocket, (sockaddr*)&serverAAddr);


        sendUDP(UDPSocket, (sockaddr*)&serverBAddr, request);
cout << "The AWS sent <"<< input << "> and <" << command << "> to Backend-Server B" << endl;
rbServerB = receiveUDP(UDPSocket, (sockaddr*)&serverBAddr);


        sendUDP(UDPSocket, (sockaddr*)&serverCAddr, request);
cout << "The AWS sent <"<< input << "> and <" << command << "> to Backend-Server C" << endl;
rbServerC = receiveUDP(UDPSocket, (sockaddr*)&serverCAddr);






///Printing the message based on results.
//printReceiptMessage(command, "B", serverBPort, rbServerB);
printReceiptMessage(command, "A", serverAPort, rbServerA);
printReceiptMessage(command, "B", serverBPort, rbServerB);
printReceiptMessage(command, "C", serverCPort, rbServerC);

//Sending the results to the monitor.
sendToMon = resutlsForMonitor(command, rbServerA, rbServerB, rbServerC);

sendTCP(monitorSocket, sendToMon);

//Sending the results to the client.
sendToClient = resutlsForClient(command, rbServerA, rbServerB, rbServerC);

sendTCP(clientSocket, sendToClient);

//clearning all the storage containers for subsequent receives.
sendToClient.clear();
sendToMon.clear();
rbServerC.clear();
rbServerB.clear();
rbServerA.clear();
request.clear();




        close(clientSocket); //Closing the client socket
   }


    
    close(sockForClient);
    close(UDPSocket);
    close(sockForMon);

}
//Functions to assist with result analysis.

void printReceiptMessage(string operation, string serverName, int port, vector<string> results)
{   
    string searchKey = "search";
    string prefixKey = "prefix";
    string suffixKey = "suffix";
    string operationNotFound = "**OPERATION NOT SUPPORTED**";

    if(operation == searchKey)
    {   
        cout<< "The AWS received <"<< results.size() - 3 << 
        "> similar words from Backend-Server <" << serverName << 
        "> using UDP over port <"<< port << ">" << endl;
    }
    if(operation == prefixKey || operation == suffixKey)
    {
        cout<< "The AWS received <"<< results.size() - 2 << 
        "> matches from Backend-Server <" << serverName << 
        "> using UDP over port <"<< port << ">" << endl;
    }
    else if (operation != searchKey && operation != prefixKey && operation != suffixKey)
    {
    	cout << "Backend-Server " << serverName << " does not support operation <" << operation << ">" << endl;
    }
}

/*
    creates a vector with results to send to the client. The vector created depends on the operation.
    If the operation is search, the output vector will only contain the operation at index 0, the input at index 1,
    and the definition at index 3 if definition was found from the backend servers. If definition was not found,
    null string will be contained in the 3rd index. 
    if the operation was prefix or suffix, operation and input will still occupy the first two indexes of the results
    vector, but matches from the 3 backend servers will be joined in subsequent indices.

    @param operation - the operation to requested.
    @param resA, resB, resC - the vectors of results returned from the 3 backend servers.
    @return - a vector ready to send to the client containing all information.
*/
vector<string> resutlsForClient(string operation, vector<string> resA, vector<string> resB, vector<string> resC)
{
    string searchKey = "search";
    string prefixKey = "prefix";
    string suffixKey = "suffix";
    string null = "\0";
    vector<string> results;

    results.push_back(operation); //putting the operation in the results for the client.
    results.push_back(resA.at(1)); //putting the input in the results for the client.
    if(operation == searchKey)
    {      //starting from the vector received from serverA, checkin if it contains definition for the input.
        if(resA.at(2) != null) //If does not contain, we check the resutls from the other 2 servers.
        {
            results.push_back(resA.at(2));
        }
        else if(resB.at(2) != null)
        {
            results.push_back(resB.at(2));
        }
        else if(resC.at(2) != null)
        {
            results.push_back(resC.at(2));
        }
        else
        {
            results.push_back(null);
        }
        if (results.at(2) != null) 
        {
            cout << "The AWS sent <1> match to the client." << endl;
        }
        else 
        {
            cout << "The AWS sent <0> match to the client." << endl;
        }
    }
    if(operation == prefixKey || operation == suffixKey) //operation is either prefix or suffix.
    {
        if(resA.size() > 2) //getting the matches from resA into results if any.
        {
            for(int i = 2; i < resA.size(); i++)
            {
                results.push_back(resA.at(i));
            }
        }
        if(resB.size() > 2)//getting the matches from resB into results if any.
        {
            for(int i = 2; i < resB.size(); i++)
            {
                results.push_back(resB.at(i));
            }
        }
      	if(resC.size() > 2)//getting the matches from resC into results if any.
        {
            for(int i = 2; i < resC.size(); i++)
            {
                results.push_back(resC.at(i));
            }
        }
        cout << "The AWS sent <" << results.size()-2 <<"> to the client." << endl;
    }
    return results;
}


vector<string> resutlsForMonitor(string operation, vector<string> resA, vector<string> resB, vector<string> resC)
{
    string searchKey = "search";
    string prefixKey = "prefix";
    string suffixKey = "suffix";
    string null = "\0";
    vector<string> results;

    results.push_back(operation); //putting the operation in the results for the client.
    results.push_back(resA.at(1)); //putting the input in the results for the client.
    if(operation == searchKey)
    {      //starting from the vector received from serverA, checkin if it contains definition for the input.
        if(resA.at(2) != null) //If does not contain, we check the resutls from the other 2 servers.
        {
            results.push_back(resA.at(2));
        }
        else if(resB.at(2) != null)
        {
            results.push_back(resB.at(2));
        }
        else if(resC.at(2) != null)
        {
            results.push_back(resC.at(2));
        }
        else
        {
            results.push_back(null);
        }

        if(resA.size() > 3) //If does not contain, we check the resutls from the other 2 servers.
        {
            results.push_back(resA.at(3));
            results.push_back(resA.at(4));

        }
        else if(resB.size() > 3)
        {
            results.push_back(resA.at(3));
            results.push_back(resA.at(4));

        }
        else if(resC.size() > 3)
        {
            results.push_back(resA.at(3));
            results.push_back(resA.at(4));

        }
        else
        {
            results.push_back(null); //pushing null in the place of the similar word.
        }

        if(results.at(2) != null && results.at(3) != null)
        {
            cout << "The AWS sent <" << results.at(1) << "> and <" << results.at(3) << "> to the monitor via TCP port 26195" << endl;
        }
        else if(results.at(2) != null)
        {
            cout << "The AWS sent <" << results.at(1) << "> to the monitor via TCP port 26195" << endl;
        }
        else if(results.at(3) != null)
        {
            cout << "The AWS sent <" << results.at(3) << "> to the monitor via TCP port 26195" << endl;

        }

        return results; //retruning to not consider the rest of the operations for efficiency.
    }
    if(operation == prefixKey || operation == suffixKey) //operation is either prefix or suffix.
    {
        if(resA.size() > 2) //getting the matches from resA into results if any.
        {
            for(int i = 2; i < resA.size(); i++)
            {
                results.push_back(resA.at(i));
            }
        }
        if(resB.size() > 2)//getting the matches from resB into results if any.
        {
            for(int i = 2; i < resB.size(); i++)
            {
                results.push_back(resB.at(i));
            }
        }
        if(resC.size() > 2)//getting the matches from resC into results if any.
        {
            for(int i = 2; i < resC.size(); i++)
            {
                results.push_back(resC.at(i));
            }
        }

        cout << "The AWS send <" << results.size() - 2 << "> matches to the monitor via TCP port 26195" << endl;
    }

    return results; //return the obtained results.
}



/*
    Populates the server address struct passed in and returns a reverance to the populated copy.
    The IP address is chosen to be the localHost address as specified in the project description.

    @param portNum - the port number to be associated with the socket.
    @param sockaddr_in - the address that needs to be filled.
    @return - a referance to a filled out address struct.
*/
sockaddr_in getAddressInfo(int portNum, sockaddr_in serverInfo)
{
    serverInfo.sin_port = htons(portNum); //adding the server port in network byte order.
    serverInfo.sin_family = AF_INET; //adding family type.
    serverInfo.sin_addr.s_addr = htons(INADDR_ANY); //assigns the ip of the local host.
    return serverInfo;
}

/*
    creates a socket of specified family and type and returns the file descriptor. Checks for error while creating the socket
    If error occurs during creation, print a Error message and exits the program.
    @param family - the family to which the socket belongs to.
    @param type - the type of the socket (e.g. SOCK_DGRAM).
    @return - the socket file descriptor.
*/
int createSocket(int family, int type)
{
    int fd = socket(family,type, 0);
    if (fd < 0)
    {
        cout << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    return fd;
}

/*  
    Binds the socket specified from the file descriptor to the address information contained in the sockaddr_in struct.
    If the bind fails, an error message is printed to the concole and the program exits.
    @param socket - the socket file descriptor.
    @param address - a sockaddr_in struct that contains the address information of the socket. Note- This struct has to be filled out.
*/
void bindSocket(int socket, sockaddr_in address)
{
    if (bind(socket, (sockaddr*) &address, sizeof(address)) < 0)
    {
        cout << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}




//METHODS TO ASSIST WITH UDP COMMUNICATIONS.


/*
    Sends all strings in a vector of strings. The method also sends a delimitor sequence delim (**END**),
    which should signal the recepient to stop receiving.
    This method is designed to work in tandem with a receiving method receiveUDP(). 
    The method returns the number of strings it has sent.
    Runs over UDP.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param address - the sockaddr_in that holds the address of the destination.
    @param sendBuffer - a vector of strings that need to be sent.
    @return - the number of strings sent.
*/
int sendUDP(int sock, sockaddr* address, vector<string>  sendBuffer)
{
    string delim = "**END**"; //delimiter sequence 
    int numOfStringsSent = 0; //the Number of strings sent.
        for(int i = 0; i < sendBuffer.size(); i++) //Sending all strings from the vector.
        {   
            sendUDPA(sock, address, sendBuffer.at(i));
            numOfStringsSent++;
        }

        sendUDPA(sock, address, delim); //sending the delimitor.
    return numOfStringsSent;
}

/*
    Receives a sequence of strings, puts them in a vector and returns that vector. Designed to work with sendUDP
    method, which has to be the sender of the strings. Uses delimitor sequence delim (**END**) to stop receiving.
    Runs over UDP.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param address - the sockaddr_in that holds the address of the destination.
    return - a vector containing all received strings.
*/
vector<string> receiveUDP(int sock, sockaddr* address)
{
    vector<string> receiveBuffer; //a container for the strings received
    string delim = "**END**"; //the delimiter sequence.
    string temp = ""; // A temporary string to hold incoming strings
    int numOfStrRecd = 0;
    while(temp != delim) //checking for a delimiter sequence and receiving until it is encountered.
    { 
        temp = receiveUDPA(sock, address);
        receiveBuffer.push_back(temp);
    }

    receiveBuffer.pop_back(); //removing the delimitor before returning the vector.
    return receiveBuffer;
}

/*
    Sends a string and receives an acknoledgment from a receipant. The acknoledgment is the size of message 
    received. SendUDPA comares that acknoledgment to the size of the message it sent and if they dont match, 
    sends the message again. desigend to work with the receiveUDPA method, which is the recepient.
    Runs over UDP.
    
    @param message - the string object to be sent.
    @param address - the sockaddr_in that holds the address of the destination.
    @param sock - the file descriptor that identifies the socket from which the message should be sent.
    @return - true if it sent the message successfull, false otherwise.
*/
bool sendUDPA(int sock, sockaddr* address, string message)
{
    int sizeSent; //size of the message actually sent.
    int sizeReceived; //size of the message received by the recepient.
    socklen_t addrSize = sizeof(*address);
        sizeSent = sendUDPString(sock, address, message);
        sizeReceived = recvfrom(sock, &sizeReceived, sizeof(sizeReceived),0, address, &addrSize);
    return (sizeSent == sizeReceived);
}


/*
    Receives a string from a socket sock reliably. Sends an acknoledgement to the sending party that the message
    was accepted. The acknolegment is the number of bytes received. The receiving end has to make sure the 
    right amount was received. returns the received string only if it is larger than 0 in size. 
    Note: this method is desigend to work with a counterpart sendUDPA() method.
    Uses UDP protocol.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param address - the sockaddr_in that holds the address of the destination.
    @return - the string received from the socket.
*/
string receiveUDPA(int sock, sockaddr* address)
{
    string message; //the string to contain the sent message.
    int messageSize; //the size of the message received.
    socklen_t addrSize = sizeof(*address);

        message = receiveUDPString(sock, address);
        messageSize = message.length();
        sendto(sock, &messageSize, sizeof(messageSize),0, address, addrSize);

    return message;
}


/*
    a function that can send a string. The string is first converted into a c string, than a char array, 
    the bytes of which are sent out using the send() method into a socket sock. returns the number of 
    bytes sent. Checks for errors and prints the apporpriate errno message. Runs over UDP.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param address - the sockaddr_in that holds the address of the destination.
    @param s - the string to be sent.
    @return - the number of bytes sent successfully.
*/
int sendUDPString(int sock, sockaddr* address, string s)
{
    string response;
    int sizeSent;                   //will store the number of bites 
    int copySize = s.length() + 1; //getting the size of the char array from the sting object
    char copy[copySize];     //settin the size of the char array to the size of the string + null character.
    strcpy(copy, s.c_str());        //copies the contents of the string object ot a char array
    socklen_t addrSize = sizeof(*address);
    do{  //the loop sends until the number of bytes sent is greater than -1. 
        sizeSent = sendto(sock, copy, sizeof(copy), 0, address, addrSize);
        if(sizeSent < 0)
        {                                       //printing the errno string repres.
            cout << strerror(errno) << endl;
        }
    }while(sizeSent < 0);
    return sizeSent;
}


/*
    Receives a string from the specified socket. receives the bytes in a char array and uses a vector to convert
    the chars into a string. Error checks for recv() function in case it returns -1 and prints the errno string version.
    intended to work over UDP sockets.

    @param socket - the file descriptor that identifies the socket to receive data from.
    @param address - the sockaddr_in that holds the address of the destination.
    @return - string that was sent to the socket.
*/
string receiveUDPString(int socket, sockaddr* address)
{
    char temp[1024]; //char array to store the incoming bytes in.
    int bytesReceived; //number of bytes received
    socklen_t addrSize = sizeof(*address);

    do{ 
        bytesReceived = recvfrom(socket, temp, sizeof(temp), 0, address, &addrSize);
        if(bytesReceived < 0)
        {
            cout << strerror(errno) << endl;
        }

    }while(bytesReceived < 0);

    vector<char> v; //uses a vector to create a string from char array up to a null ('\0') char.
    int i = 0;     //the iterator 

    while(temp[i] != '\0')   //this loop puts all chars from temp[] into vector v up to a null char.
    {
        v.push_back(temp[i]);
        i++;
    }

    string message(v.begin(), v.end()); //the string object that is constructed from a vector of chars.

    return message;
}


//METHODS THAT ASSIST WITH TCP COMMUNICATIONS.


//Functions to aid with sending and receiving data through a TCP port.
/*
    Sends all strings in a vector of strings. The method also sends a delimitor sequence delim (**END**),
    which should signal the recepient to stop receiving.
    This method is designed to work in tandem with a receiving method receiveTCP(). 
    The method returns the number of strings it has sent.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param sendBuffer - a vector of strings that need to be sent.
    @return - the number of strings sent.
*/
int sendTCP(int sock, vector<string>  sendBuffer)
{
    string delim = "**END**"; //delimiter sequence 
    int numOfStringsSent = 0; //the Number of strings sent.

        for(int i = 0; i < sendBuffer.size(); i++) //Sending all strings from the vector.
        {   
            sendTCPA(sock, sendBuffer.at(i));
            numOfStringsSent++;
        }

        sendTCPA(sock, delim); //sending the delimitor.
    return numOfStringsSent;
}

/*
    Receives a sequence of strings, puts them in a vector and returns that vector. Designed to work with sendTCP
    method, which has to be the sender of the strings. Uses delimitor sequence delim (**END**) to stop receiving.

    @param sock - the file descriptor that identifies the socket to send data into.
    return - a vector containing all received strings.
*/
vector<string> receiveTCP(int sock)
{
    vector<string> receiveBuffer; //a container for the strings received
    string delim = "**END**"; //the delimiter sequence.
    string temp = ""; // A temporary string to hold incoming strings
    int numOfStrRecd = 0;

    while(temp != delim) //checking for a delimiter sequence and receiving until it is encountered.
    {
    temp = receiveTCPA(sock);
    receiveBuffer.push_back(temp);
    }

    receiveBuffer.pop_back(); //removing the delimitor before returning the vector.
    return receiveBuffer;
}

/*
    Sends a string and receives an acknoledgment from a receipant. The acknoledgment is the size of message 
    received. SendTCPA comares that acknoledgment to the size of the message it sent and if they dont match, 
    sends the message again. desigend to work with the receiveTCPA method, which is the recepient.
    
    @param message - the string object to be sent.
    @param sock - the file descriptor that identifies the socket from which the message should be sent.
    @return - true if it sent the message successfull, false otherwise.
*/
bool sendTCPA(int sock, string message)
{
    int sizeSent; //size of the message actually sent.
    int sizeReceived; //size of the message received by the recepient.

    do{         //Checks if the sizeSent is equal to received size. if not sends message again.
        sizeSent = sendTCPString(sock, message);
        recv(sock, &sizeReceived, sizeof(sizeReceived),0);
    } while(sizeSent == sizeReceived);

    return (sizeSent == sizeReceived);
}


/*
    Receives a string from a socket sock reliably. Sends an acknoledgement to the sending party that the message
    was accepted. The acknolegment is the number of bytes received. The receiving end has to make sure the 
    right amount was received. returns the received string only if it is larger than 0 in size. 
    Note: this method is desigend to work with a counterpart sendTCPA() method.

    @param sock - the file descriptor that identifies the socket to send data into.
    @return - the string received from the socket.
*/
string receiveTCPA(int sock)
{
    string message; //the string to contain the sent message.
    int messageSize; //the size of the message received.

            //receives a string until the received size is greater than 0 and sends that size to the sender.
        message = receiveTCPString(sock);
        messageSize = message.length();
        send(sock, &messageSize, sizeof(messageSize),0);
    return message;
}


/*
    a function that can send a string. The string is first converted into a c string, than a char array, 
    the bytes of which are sent out using the send() method into a socket sock. returns the number of 
    bytes sent. Checks for errors and prints the apporpriate errno message.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param s - the string to be sent.
    @return - the number of bytes sent successfully.
*/
int sendTCPString(int sock, string s)
{
    string response;
    int sizeSent;                   //will store the number of bites 
    int copySize = s.length() + 1; //getting the size of the char array from the sting object
    char copy[copySize];     //settin the size of the char array to the size of the string + null character.
    strcpy(copy, s.c_str());        //copies the contents of the string object ot a char array

    do{  //the loop sends until the number of bytes sent is greater than -1. 
        sizeSent = send(sock, copy, sizeof(copy), 0);
        if(sizeSent < 0)
        {                                       //printing the errno string repres.
            cout << strerror(errno) << endl;
        }
    }while(sizeSent < 0);

    return sizeSent;
}


/*
    Receives a string from the specified socket. receives the bytes in a char array and uses a vector to convert
    the chars into a string. Error checks for recv() function in case it returns -1 and prints the errno string version.

    @param socket - the file descriptor that identifies the socket to receive data from.
    @return - string that was sent to the socket.
*/
string receiveTCPString(int socket)
{
    char temp[1024]; //char array to store the incoming bytes in.
    int bytesReceived; //number of bytes received
    do{ 
        bytesReceived =recv(socket, temp, sizeof(temp), 0);
        if(bytesReceived < 0)
        {
            cout << strerror(errno) << endl;
        }

    }while(bytesReceived < 0);

    vector<char> v; //uses a vector to create a string from char array up to a null ('\0') char.
    int i = 0;     //the iterator 

    while(temp[i] != '\0')   //this loop puts all chars from temp[] into vector v up to a null char.
    {
        v.push_back(temp[i]);
        i++;
    }

    string message(v.begin(), v.end()); //the string object that is constructed from a vector of chars.

    return message;
}





















