//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project

#include "client.h"






int main(int argc, char *args[])
{

//Variables to store the strings 
    string command = args[1];    //Storing the first arguement passed in - refers to the desired functon.
    string target = args[2];     //Storing the second argument from command line - refers to the input.
    vector<string> sendBuffer;
    vector<string> receiveBuffer;


//Server address information 
    const int awsPort = 25195;
    const char * awsIP = "127.0.0.1";
    sockaddr_in awsAddr; //the strcut of type sockaddr_in that holds the address of the server
//Client socket
    int clientSocket; //The socket at the client side.

//Filling in the server address information.
    awsAddr = getAddressInfo(awsPort, awsIP, awsAddr);

//System calls 

//Creating Socket or exits the program and prints an error message if the socket couldnt be created.
    clientSocket = createSocket(AF_INET, SOCK_STREAM);
   



//Connecting to the server
    //The client does not need the bind() step because the port is assigned dynamically.
   if(connect(clientSocket, (sockaddr *) &awsAddr, sizeof(awsAddr)) < 0)
    {
        cout << strerror(errno) << endl; //printing an error message if the connection failed 
        exit(EXIT_FAILURE); //exiting the program.
    }

    
    cout << "The Client is up and Running." << endl;


    sendBuffer.push_back(command); //pusing command into the sendBuffer. Has to be done before pushing the target.
    sendBuffer.push_back(target); //pushing the target into the sendBuffer.

    sendTCP(clientSocket, sendBuffer);  //sending the elements in the sendBuffer.

    cout << "The Client sent <"<< target <<"> and <" << command <<"> to AWS" << endl;  

    receiveBuffer = receiveTCP(clientSocket); //Receiving the results from the AWS server.

    displayResults(command, target, receiveBuffer);


    close(clientSocket); //closing the TCP connection and destroying the socket.
}
//End of 


///Functions for string operations and displays.

/*
    Uses a received vector of strings to display the results of on the terminal depending on the operation
    specified in the command line. Supports 3 operations: search, prefix and suffix. 

    *PRECONDITIONS FOR Search operation: 
       SEARCH: the zeroth index of receivedBuf should contain the word and the the next element its definition.*

       @param operation - the specified operation.
       @param input - the requested input.
       @param receivedBuf - a vector of strings that contains the received data from AWS server.
*/
void displayResults(string operation, string input, vector<string> receivedBuf)
{
    //Strings that hold the available operations of the server.
    string searchKey = "search"; 
    string prefixKey = "prefix";
    string suffixKey = "suffix";
    if (receivedBuf.size() > 2)
    {
        if (operation == searchKey) //perform search operation
        { 
            string definition = receivedBuf.at(2);
            if(definition != "\0")  //Printing the message for the search operation 
            {
                 cout <<"Found match for <" << input << "> " << endl;
                cout << "<" << definition  << ">" << endl; //There was no match for definition.
            } 
             else
            {
                cout <<"No match found for <" << input << "> " << endl;
            } 
        }
        else if (operation == prefixKey || operation == suffixKey) //display results from prefix operation.
        {
            cout << "Found  <" << receivedBuf.size()- 2 << "> matches for input <" << input << ">." <<endl;
            for (int i = 2; i < receivedBuf.size(); i++)
            {
                cout << "<" << receivedBuf.at(i) << ">" << endl;
            }
        }
        else
        {
            cout << "Operation Not Supported: Please enter operation from {search, prefix, suffix}." << endl;
        }
    }    
}


//Function helping with setting up the connection to the server.

/*
    Populates the server address struct passed in and returns a reverance to the populated copy.
    The IP address is chosen to be the localHost address as specified in the project description.

    @param portNum - the port number to be associated with the socket.
    @param sockaddr_in - the address that needs to be filled.
    @return - a referance to a filled out address struct.
*/
sockaddr_in getAddressInfo(int portNum, const char* serverIP, sockaddr_in serverInfo)
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
    int fd = socket(family,type, 0); //creating the socket.
    if (fd < 0)
    {       //checking for errors and printing the errno string repres.
        cout << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    return fd;
}





//Functions that aid with sending and receiving data.

/*
    Sends all strings in a vector of strings. The method also sends a delimitor sequence delim (**END**),
    which should signal the recepient to stop receiving.
    This method is designed to work in tandem with a receiving method receiveTCP(). 
    The method returns the number of strings it has sent.

    @param sock - the file descriptor that identifies the socket to send data into.
    @param sendBuffer - a vector of strings that need to be sent.
    @return - the number of strings sent.
*/
int sendTCP(int sock, vector<string> sendBuffer)
{
    string delim = "**END**"; //delimiter sequence 
    int numOfStringsSent = 0; //the Number of strings sent.
        for(int i = 0; i < sendBuffer.size(); i++)
        { //Sending all strings from the vector.
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

    while(temp != delim) {//checking for a delimiter sequence and receiving until it is encountered.
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

        sizeSent = sendTCPString(sock, message);
        sizeReceived = recv(sock, &sizeReceived, sizeof(sizeReceived),0);

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
    }while (sizeSent < 0);

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
