//Author: Harutyun Minasyan
//Student ID: 5639-3401-95
//EE450 Socket Programmin project
#include "serverC.h"






int main()
{

	string fileName = "backendC.txt"; //The file of from which the server will pull data.
	ifstream dictionary;	//The input file stream that will get data from the file.

	int serverSocket; //the socket file descriptor.
	const int serverPort = 23195; //port number of the server
	const char * serverIP = "172.0.0.1"; //the ip of the server machine

	sockaddr_in serverAddr, awsAddr; //creating the address structs of the server and the client.

	
//Container for operations
	string command; //The string that will stored the requested operation.
	string input;   //The string against which the operation is to be performed.
	vector<string> request; //A Vector of string that will contain the request.
	vector<string> results; // A Vector of strings that will contain the results.

				///////////Starting the Server /////////////////////

//Creating the socket.
	serverSocket = createSocket(AF_INET, SOCK_DGRAM); 

//Filling out the address of the server.
	serverAddr = getAddressInfo(serverPort, serverIP, serverAddr); //Filling in the address of the server

//Binding the socket to the server address.
	bindSocket(serverSocket, serverAddr);
	cout << "Server C is up and running using UDP on port " << serverPort << endl;


/*An infinite loop that receives a request through a server UDP socket, performs that requested operations
   and sends the results to the AWS server.*/
	while(true){

		request = receiveUDP(serverSocket, (sockaddr*)&awsAddr); //receiving the request.

		command = request.at(0); //command is always contained in the zeroth index of the request vector.
		input = request.at(1);   //input is at the first index of the request vector.

		//making sure the command and the input are both lowercase 
		converToLower(command);
		converToLower(input); 

		cout << "Server C received input <" << input << "> and operation <" << command << ">" << endl;

		performOperation(command, input, &results, dictionary, fileName); //perform the operation and store the results in a vector.

		if(results.empty())
		{					//If the resutls were empty, we add a signaling string ( the null string).
			results.push_back("\0");
		}

		sendUDP(serverSocket, (sockaddr*)&awsAddr, results); //send the results to the AWS server.

		cout << "The Server C finished sending the output to the AWS." << endl;

		//Emptying the two vectors for subsequnt runs.
		request.clear(); 
		results.clear();

	}

	close(serverSocket); //closing the socket.

}//END OF MAIN





/*
	Performs the specified operation and stores the results in the results string vector passed by referance.
	The function supports 3 operations: search, prefix, and suffix. For the search operation, the zeroth index
	of the results vector is designated for the definition of the target. If no def. found, null char is stored here.
	prefix and suffix operations return a load of words into the vector. 

	@param operation - the operation to perform.
	@param input - the string agains which the dictionary should be operated on.
	@param results - a pointer to a vector of strings where the results will be stored.
	@param dict - an input file stream object that will open the file containing the dictionary data.
	@param fileName - a string specifing the name of the dictionary file.
*/
void performOperation(string operation, string input, vector<string> * results, ifstream& dict, string fileName)
{
	//Strings that hold the available operations of the server.
	string searchKey = "search"; 
	string prefixKey = "prefix";
	string suffixKey = "suffix";
	string definition;
	vector<string> temp; //A vector to temporarly hold the list of similar words outputed by seachForSimilar().
	results->push_back(operation);
	results->push_back(input);
	if (operation == searchKey) //perform search operation
	{ 
		definition = searchForDefin(dict, input, fileName); //get the definition
		results->push_back(definition);  
		temp = searchForSimilar(dict, input, fileName); //getting similar words.

		for(int i = 0; i < temp.size(); i++) //load the 
		{							
			results->push_back(temp.at(i));  
		}

		if(definition != "\0")  //Printing the message for the search operation 
		{
			cout <<"The Server C has found <" << 1 << "> match and <" << temp.size() << "> similar words." << endl; //there is a match for definition.
		}else
		{	
			cout <<"The Server C has found <" << 0 << "> match and <" << temp.size() << "> similar words." << endl;  //There was no match for definition.
		}
	}
	else if (operation == prefixKey || operation == suffixKey) //perform prefix operation
	{
		temp = prefixOrSuffix(operation, dict, input, fileName); //computing the prefixed words that match target.
		for(int i = 0; i < temp.size(); i++) //load the 
		{							
			results->push_back(temp.at(i));  
		}
		cout << "The Server C has found <" << results->size() - 2 <<"> matches" <<endl;
	}

	else
	{
		cout << "The server does not support <" << operation << "> operation." << endl; //operation not supported
	}

}
	

//Functions that perform server operations "search, prefix and suffix".

/*
	The function that finds a word that has only one letter changed compared to the target. 
	Returns "\0" string if no match was found.
	
	@param dict - a referance to the ifstream object.
	@param target - a string which will be used to find a similar word.
	@param fileName - a string which holds the name of the text file where the data needs to be found from.
	@return - a string containing the similar word found or the null character if nothing is found.

*/
string searchForDefin(ifstream& dict, string target,string fileName)
{
	string lineOfText; //storage for a line of text from the file being read. 
	string key;       //This will hold the key. It can be one key at a time.
	string delim = "::"; // The delimiter symbol that separates the keys from definitions.
	string definition; //The string that will hold the definition of a matched word to target.
	int indexOfDelim; //this will record the index at which the delimiter "::" appears in the string.

	dict.open(fileName.c_str()); //opening the file using the ifstream opject.

	if(!dict.is_open())
	{	//Checking if the file is opened properly. If not, exits the program and print an error message.
		cout << "ERROR: Failed to open the file named " << fileName << endl;
		exit(1);
	}

	while (!dict.eof())
	{ // The engine of the search. Iterates over every line until finds a match or no end of file is reached.
		getline(dict, lineOfText); //getting the next line from the file.
		if(lineOfText.length() < 1)
		{ //exit since the end of file is reached.
			dict.close(); //closing the opened file.
			return "\0"; //returning the null character since no match was found and end of file is reached.
		}

		indexOfDelim = lineOfText.find(delim); //checking agains the delimitor
		key = lineOfText.substr(0, indexOfDelim - 1);  //getting the key.
		converToLower(key); //turning the key into lowercase.

		if( key == target)
		{ //If the key is the same as the target, returns the definition of the key.
			dict.close(); //closing the opened file.
			definition = lineOfText.substr(indexOfDelim + 2); //getting the definition.
			return definition; //return the definition.
		}
	} 
	dict.close(); //closing the opened file.
	return "\0"; //return the definition.
}


/*
	The function that finds all words that has only one letter changed compared to the target. All such words
	are put in a vector of strings along with their definitions, which is than returned. definitions are right 
	next to the keys in the list.
	
	@param dict - a referance to the ifstream object.
	@param target - a string which will be used to find a similar word.
	@param fileName - a string which holds the name of the text file where the data needs to be found from.
	@return - a string vector containing all similar words with their definitions.
*/
vector<string> searchForSimilar(ifstream& dict, string target, string fileName)
{
	int lettersOff = 0; //This will record the number of letters switched
	string lineOfText; //storage for a line of text from the file being read. 
	int indexOfDelim; //this will record the index at which the delimiter "::" appears in the string.
	string key;       //This will hold the key. It can be one key at a time.
	string delim = "::"; // The delimiter symbol that separates the keys from definitions.
	string definition; //The string that will hold the definition of a matched word to target.

	vector<string> similarWords;
	dict.open(fileName.c_str()); //opening the file using the ifstream opject.

	if(!dict.is_open())
	{	//Checking if the file is opened properly. If not, exits the program and print an error message.
		cout << "ERROR: Failed to open the file named " << fileName << endl;
		exit(1);
	}

	while (!dict.eof())
	{//The engine of the search. Iterates over every key until it finds a similar key to target or end of file is reached.
		getline(dict, lineOfText);   //getting a line of text.
		indexOfDelim = lineOfText.find(delim);
		key = lineOfText.substr(0, indexOfDelim - 1);
		converToLower(key); //turning the key to lowercase.

		if(lineOfText.length() < 1)
		{ //exit since the end of file is reached.
			dict.close(); //closing the opened file.
			return similarWords; //returning the null character since no match was found and end of file is reached.
		}

		if (key.length() == target.length())
		{ //If the target and the key are the same size, we check all their characters for equality.
			for (int i = 0; i < key.length(); i ++)
			{
				if(key.at(i) != target.at(i))
				{ //Checking how many characters differ between the key and the target.
					lettersOff++;
				}
			}

			if(lettersOff == 1)
			{ //Only one letter was off between the key and the target, so it returns that key and close the file.
				dict.close();
				definition = lineOfText.substr(indexOfDelim + 2); //getting the definition of the key.
				//storing th key and the definition in the list right next to each other.
				similarWords.push_back(key);  
				similarWords.push_back(definition);
			}
		}
		lettersOff = 0; //reseting for the next key.
	}//End While
	dict.close(); //closing the opened file.
	return similarWords; //returning the null character since no match was found and end of file is reached.
}


/*
	Performs an operation on a dictionary using a file reader and finds all keys that have a prefix or a suffix matching the 
	prefix/suffix that is the target. The method is case insensetive for both the dictionary and the target. Dictionary keys must be separated
	from the definitions by "::" delimitor. The key/definition pair can occupy not more than 1 line and 1 line can 
	only hold 1 key/def. pair for the method to work correctly. 
	
	@param operation - a string that can be either "prefix" or "suffix".
	@param dict - the file reader ifstream object.
	@param target - the prefix itself.
	@param fileName - the name of the dictionary file.
	@return - a vector of strings loaded with the keys from dictionary that have the prefix
*/
vector<string> prefixOrSuffix(string operation, ifstream& dict, string target, string fileName)
{	
	vector<string> list; //will hold the matched keys
	int sizeOfTarget = target.length(); //the size of the prefix.
	string begining; //will store the begining of the keys if the operation is prefix.
	string ending; //will store the ending of the keys if the operation is suffix.
	string delim = "::";//the delimitor sequence.
	string prefixKey = "prefix";
	string suffixKey = "suffix";
	string lineOfText; //will hold a line of text.
	string key; //will hold the keys.

	dict.open(fileName.c_str()); //opening the file using the ifstream opject.

	if(!dict.is_open())
	{	//Checking if the file is opened properly. If not, exits the program and print an error message.
		cout << "ERROR: Failed to open the file named " << fileName << endl;
		exit(1);
	}

	while (!dict.eof())
	{//The engine of the search. Iterates over every key until it finds a similar key to target or end of file is reached.
		getline(dict, lineOfText);  //getting a line of text.
		int indexOfDelim = lineOfText.find(delim); //chekcing agains the delimitor.
		key = lineOfText.substr(0, indexOfDelim - 1); //getting  the key.
		converToLower(key); //making sure the key is lower case

		if(lineOfText.length() < 1)
		{ //exit since the end of file is reached.
			dict.close(); //closing the opened file.
			return list; //returning the null character since no match was found and end of file is reached.
		}
		if (operation == prefixKey) //performing the prefix operation
		{
			if (key.length() >= sizeOfTarget)
			{
				begining = key.substr(0, sizeOfTarget); //getting the begining of the key
			if (target == begining)
				{	//if the begining matches, we push that key into the list.
					list.push_back(key);
				}
			}
		}
		if(operation == suffixKey){ //Perforing the suffix operaton
			if (key.length() >= sizeOfTarget)
			{
				ending = key.substr(key.length() - (sizeOfTarget));
				//cout << "begining is " << begining << endl;
				if (target == ending)
				{
					list.push_back(key);
				}
			}
		}	
	}
	return list; //retuning the completed list.
}




/*
	Converts a passed in string to all lowerCase.

	@param word - a referance to a string to be converted.
*/
void converToLower(string& word)
{
	for(int i = 0; i < word.length(); i++)
	{
		word.at(i) = tolower(word.at(i));
	}
}


///ASSIST WITH ADDRESS LOADING, SOCKET CREATION AND BINDING.

/*
	Takes a socket address struct sockaddr_in and populates the fields with the information of the server.
	@param portNum - the port number of the server.
	@param serverIP - a string literal representation of the server IP address.
	@param serverInto - a structure to contain the server address information
*/
sockaddr_in getAddressInfo(int portNum, const char * serverIP, sockaddr_in serverInfo)
{
    serverInfo.sin_port = htons(portNum); //adding the server port in network byte order.
    serverInfo.sin_family = AF_INET; //adding family type.
    serverInfo.sin_addr.s_addr = htons(INADDR_ANY); //adding the server IP.
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


/*
	Binds the socket to the address in the address sockaddr_in structure.
	if the binding fails, prints the appropriate errno string message. 
	
	@param socket - the file desctiptor associated with the socket.
	@param address - the internet address containig sockaddr_in structure.
*/
void bindSocket(int socket, sockaddr_in address)
{
    if (bind(socket, (sockaddr*) &address, sizeof(address)) < 0)
    {
        cout << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}





///Functions that assist with transmissions over a networdk using the UDP.

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
