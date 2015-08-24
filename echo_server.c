#include <stdio.h> //standard input and output
#include <sys/socket.h> //for socket and socket functions
#include <arpa/inet.h> //for sockaddr_in and inet_ntoa
#include <stdlib.h>	//for atoi() and exit()
#include <string.h> //for memset()
#include <unistd.h> //for close()


#define RCVBUFSIZE 32 //size of the receive buffer
#define MAXPENDING 5 //maximum outstanding connection requests

void handleTCPClient(int clientSocket);
void dieWithError(char*errorMessage);

int main(int argc,char*argv[]){
	int serverSock; //socket descriptor for server
	int clientSock; //socket descriptor for client
	struct sockaddr_in echoServerAddr;  //local address
	struct sockaddr_in echoClientAddr; //client address
	unsigned short echoServerPort;
	unsigned int clientLen; //length of client address data structures

	if(argc !=2){
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
		exit(1);
	}

	echoServerPort = atoi(argv[1]); // first arg local port

	//create socket for incoming connections

	if((serverSock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		dieWithError("socket() failed!");

	//construct local address structure
	memset(&echoServerAddr,0,sizeof(echoServerAddr)); //zero out structure
	echoServerAddr.sin_family=AF_INET;          //internet address family
        echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); //any incoming interface
        echoServerAddr.sin_port = htons(echoServerPort); //local port
        
        
        //bind to the local address
        
        if(bind(serverSock,(struct sockaddr *)&echoServerAddr,sizeof(echoServerAddr))<0)
            dieWithError("bind() failed!");
        
        //mark the socket so it will listen to incoming connections.
        if(listen(serverSock, MAXPENDING)<0)
            dieWithError("listen() failed!");
        
        for(;;){ //run forever
            //set the size of the in out parameter
            clientLen = sizeof(echoClientAddr);
            
            //wait for a client to connect
            if((clientSock = accept(serverSock,(struct sockaddr*)&echoClientAddr, &clientLen))<0)
                dieWithError("accept() failed!");
            
            //clientSock is connected to a client
            printf("Talking with client %s\n",inet_ntoa(echoClientAddr.sin_addr));
            handleTCPClient(clientSock);
            
        }


	return 0;
}
void handleTCPClient(int clientSocket){
    char echoBuffer[RCVBUFSIZE]; //buffer of the echo string
    int recvMessageSize;
    
    //receive message from client
    if((recvMessageSize = recv(clientSocket, echoBuffer, RCVBUFSIZE,0))<0)
        dieWithError("recv() failed!");
    //send received string and receive again until end of transmission
    
    while(recvMessageSize >0){
        //echo the message back to the client
        if(send(clientSocket,echoBuffer,recvMessageSize,0)!=recvMessageSize)
            dieWithError("send() failed!");
        
        //see if there is more data to receive
        if((recvMessageSize = recv(clientSocket,echoBuffer, RCVBUFSIZE,0))<0)
            dieWithError("recv() failed!");
        
        
    }
    close(clientSocket); //close the client socket
    
    
}

void dieWithError(char*errorMessage){
	perror(errorMessage);
	exit(1);
}
