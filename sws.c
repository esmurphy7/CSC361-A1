#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <time.h>  
#include <sys/types.h>  
#include <sys/socket.h> 
#include <sys/time.h> 
#include <netinet/in.h>  
#include <arpa/inet.h>  

#include "responseManager.h"
  
	// Run this program by providing a port number and root directory
	//		>> ./sws 9090 /home/<user>/<dir>

/* 
* This function reports the error and 
* exits back to the shell: 
*/  
static void displayError(const char *on_what)
{  
	fputs(strerror(errno),stderr);  
	fputs(": ",stderr);  
	fputs(on_what,stderr);  
	fputc('\n',stderr);  
	exit(1);  
}
/*
* Wait for user input from stdin and socket "sockfd"
*/
int waitForUserQuit(int sockfd)
{
	int MAXBUFLEN = 1024;
	//struct timeval tv = {3, 0};
    while (1){
        char read_buffer[MAXBUFLEN];
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
		//printf("selecting...\n");
        int retval = select(sockfd+1, &readfds, NULL, NULL, NULL);
        if(retval <=0)
            return retval;
        else
        {
            if(FD_ISSET(STDIN_FILENO, &readfds) &&
               (fgets(read_buffer, MAXBUFLEN, stdin) != NULL) &&
               strchr(read_buffer, 'q') != NULL)
                return 1;
            else if(FD_ISSET(sockfd, &readfds))
                return 2;
        }
    }
    return -1;
}
  
int main(int argc,char **argv) {  
	int z;  
	struct sockaddr_in adr_inet;
	struct sockaddr_in adr_clnt; 
	int len_inet; 
	int s;  
	char dgram[512];             // Recv buffer  
	char dtfmt[512];			  // Date/Time Result  
	time_t td;                   // Current Time and Date  
	struct tm tm;                // Date time values  

	char* response;	  									// Response line to print out: timestamp; client_address:port responseLine; filepath
	char* requestLine;	  									// Request line formatted from datagram packet
	char* port_asStr = (char*)malloc(sizeof(char*)*256); 	// Port number as a string

	int port;					  // User specified port number
	char* root;				  // User specified root directory
	 
 /* 
  * Set the port and root directory fromm the command line
  */  
     if ( argc == 3 ) {
		port = atoi(argv[1]);
		root = argv[2];
		mount_onroot(root);	// mount server on root dir
     }  
     else {  
        printf("Please specify a port number followed by a root directory\n"); 
		exit(1);
     }  
  
 /* 
  * Create a UDP socket to use: 
  */  
     s = socket(AF_INET,SOCK_DGRAM,0);  
     if ( s == -1 ) {  
        displayError("socket()");  
     }  
 /* 
  * Create a socket address, for use 
  * with bind(2): 
  */  
     memset(&adr_inet,0,sizeof adr_inet);  
     adr_inet.sin_family = AF_INET;  
     adr_inet.sin_port = htons(port);  
     adr_inet.sin_addr.s_addr = htonl(INADDR_ANY);
	
     if ( adr_inet.sin_addr.s_addr == INADDR_NONE ) {  
        displayError("bad address.");  
     }  
     len_inet = sizeof adr_inet;  
  
 /* 
  * Bind an address to our socket, so that 
  * client programs can contact this 
  * server: 
  */  
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  
     z = bind(s, (struct sockaddr *)&adr_inet, len_inet);  
     if ( z == -1 ) {  
        displayError("bind()");  
     }  
	 printf("sws is running on UDP port %d and serving %s\n",port,root);
	 
 /* 
  * Now wait for requests: 
  */  
	for (;;) {  
		/*
		* Check if user wants to quit the server
		*/
		int retval = waitForUserQuit(s);
		printf("retval: %d\n",retval);
		if(retval == 1)
		{
			printf("quitting...\n");
			exit(1);
		}
		/* 
		* Block until the program receives a 
		* datagram at the address and port: 
		*/  
		len_inet = sizeof adr_clnt;  
		z = recvfrom(s,            // Socket  
					 dgram,        // Receiving buffer  
					 sizeof dgram, // Max recv buf size  
					 0,            // Flags: no options  
					 (struct sockaddr *)&adr_clnt, // Addr  
					 &len_inet);  // Addr len, in & out  
		if ( z < 0 ) {  
		   displayError("recvfrom(2)");  
		}  
		dgram[z] = '\0'; 

		
		/* 
		* Get the current date and time: 
		*/  
		time(&td);           
		tm = *localtime(&td); 
		/* 
		* Format the timestamp 
		*/  
		char* fmt = "%m %e %H:%M:%S";
        strftime(dtfmt,       
                 sizeof dtfmt,
                 fmt,  
                 &tm); 
		char* timestamp = format_timestamp(dtfmt);
		/*
		* Copy the packet's data for later use
		*/
		requestLine = (char*)malloc(sizeof(char*)*256);
		int i=0;
		while(dgram[i] != '\0')
		{
			if(ispunct(dgram[i]) || isalnum(dgram[i]) || dgram[i] == ' ')
				requestLine = charAppend(requestLine, dgram[i]);
			i++;
		}
		/* 
		* Parse then execute the request and store output
		*/
		char* responseLine;
		if(isBadRequest(requestLine))
			responseLine = responseSet._400;
		else
			responseLine = get_responseline(dgram);
		char* filePath = get_filePath();
		char* fileContents = get_fileContents();
		free_file();
		//printf("responseLine: %s\nfilePath: %s\nfileContents: %s\n",responseLine, filePath, fileContents);
		/*
		* Create response string
		*/
		response = (char*)malloc(sizeof(response)*1000);
		strcpy(response, timestamp);
		response = charAppend(response, ' ');
		//printf("response: %s\n",response);
		
		strcat(response, inet_ntoa(*(struct in_addr *)&adr_clnt.sin_addr.s_addr));
		response = charAppend(response, ':');
		//printf("response: %s\n",response);
		
		sprintf(port_asStr, "%d", ntohs(adr_clnt.sin_port));
		strcat(response, port_asStr);
		response = charAppend(response, ' ');
		//printf("response: %s\n",response);
		
		strcat(response, requestLine);
		strcat(response, "; ");
		//printf("response: %s\n",response);
		
		strcat(response, responseLine);
		strcat(response, "; ");
		//printf("response: %s\n",response);
		
		if(strcmp(responseLine, responseSet._200)==0)
			strcat(response, filePath);
		printf("%s\n",response);
		
		char* to_client = fileContents;
		if(strcmp(responseLine, responseSet._200) != 0)
			to_client = responseLine;
	
		/* 
		* Send the response or file back to the 
		* client program: 
		*/  
        z = sendto(s,
                   to_client, 
                   strlen(to_client), 
                   0, 
                   (struct sockaddr *)&adr_clnt,// addr  
                   len_inet);
		if ( z < 0 ) {  
		  displayError("sendto(2)");  
		}  
    }  
	/*
	*	Free memory
	*/
	free(response);
	free(requestLine);
	free(port_asStr);

	/* 
	* Close the socket and exit: 
	*/  
	 close(s);  
	 return 0;  
}  
