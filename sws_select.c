/* A simple server */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFLEN 256
int checkReadyForRead(int sockfd)
{
    while (1){
        char read_buffer[MAXBUFLEN];
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        int retval = select(sockfd+1, &readfds, NULL, NULL, NULL);
        if(retval <=0) //error or timeout
            return retval;
        else
        {
            if(FD_ISSET(STDIN_FILENO, &readfds) &&
               (fgets(read_buffer, MAXBUFLEN, stdin) != NULL) &&
               strchr(read_buffer, 'q') != NULL)  // 'q' pressed
                return 1;
            else if(FD_ISSET(sockfd, &readfds))   // recv buffer ready
                return 2;
        }
    }
    return -1;
}

int main(int argc, char *argv[]){
    //Using socket() function for communication
    //int socket(int domain, int type, int protocol);
    //domain is PF_INET, type is SOCK_DGRAM for UDP, and protocol can be set to 0 to choose the proper protocol!
    int sockfd, portno;
    socklen_t cli_len;
    char buffer[MAXBUFLEN];	//data buffer
    struct sockaddr_in serv_addr, cli_addr;	//we need these structures to store spcket info
    int numbytes;
    
    if (argc < 2) {
        printf( "Usage: %s <port>\n", argv[0] );
        fprintf(stderr,"ERROR, no port provided\n");
        return -1;;
    }
    
    //The first step: creating a socket of type of UDP
    //error checking for every function call is necessary!
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("sws: error on socket()");
        return -1;
    }
    
    /* prepare the socket address information for server side:
     (IPv4 only--see struct sockaddr_in6 for IPv6)
     
     struct sockaddr_in {
     short int          sin_family;  // Address family, AF_INET
     unsigned short int sin_port;    // Port number
     struct in_addr     sin_addr;    // Internet address
     unsigned char      sin_zero[8]; // Same size as struct sockaddr
     };
     */
    /*Clear the used structure by using either memset():
     memset(&serv_addr, 0, sizeof(struct sockaddr_in));	     or bzero(): */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);  //read the port number value from stdin
    serv_addr.sin_family = AF_INET;	//Address family, for us always: AF_INET
    //serv_addr.sin_addr.s_addr = inet_addr("142.104.69.255");
    serv_addr.sin_addr.s_addr = inet_addr("10.10.1.100"); //INADDR_ANY;  //Listen on any ip address I have
    serv_addr.sin_port = htons(portno);  //byte order again
    
    //Bind the socket with the address information:
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0){
        close(sockfd);
        perror("sws: error on binding!");
        return -1;
    }
    
    while(1){
        printf("sws: waiting to recvfrom...\n");
        cli_len = sizeof(cli_addr);
        int ret = checkReadyForRead(sockfd);
        if(ret == 1)
            exit(1);
        //the sender address information goes to cli_addr
        if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
                                 (struct sockaddr *)&cli_addr, &cli_len)) == -1) {
            perror("sws: error on recvfrom()!");
            return -1;
        }
        /*
         printf("sws: got packet from %s\n",
         inet_ntop(their_addr.ss_family,
         get_in_addr((struct sockaddr *)&their_addr),
         s, sizeof s));
         */
        
        printf("sws: received packet from IP: %s and Port: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        printf("listener: received packet is %d bytes long\n", numbytes);
        buffer[numbytes] = '\0';
        printf("listener: packet contains \"%s\" \n", buffer);
        
        if ((numbytes = sendto(sockfd, buffer, strlen(buffer), 0,
                               (struct sockaddr *)&cli_addr, cli_len)) == -1) {
            perror("sws: error in sendto()");
            return -1;
        }
    }
    
    close(sockfd);
    
    //freeing memory!
    return 0;
}
