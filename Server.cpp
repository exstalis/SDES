#include "Utility.h"


using namespace Server_Client_Framework;



	void dostuff(int, struct sockaddr_in cli_addr); /* function prototype */

	int main(int argc, char *argv[])
	{
		/** variables for communication **/
		int sockfd, newsockfd, portno, pid;
		socklen_t clilen;
		struct sockaddr_in serv_addr, cli_addr;
	
		// OPEN A SOCKET WITH INTERNET_DOMAIN_ADDRESS AND CONTINUOUS STREAMING
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
			fatal_error("ERROR opening socket");
		bzero((char *) &serv_addr, sizeof(serv_addr));	// Initialise serv_addr to zero
		if(argc < 2){
			fatal_error("No Server Port Provided");	
		}
		portno = atoi(argv[1]);
	
		// SET serv_addr struct attributes
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
	
		// TRY TO BIND WITH PORT
        long ret_status =::bind(sockfd,(struct sockaddr *) &serv_addr,
                              sizeof(serv_addr));
        if ( ret_status < 0)
		      error("ERROR on binding");
	
		// START LISTENING
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		while (1) {
		 newsockfd = accept(sockfd, 
		       (struct sockaddr *) &cli_addr, &clilen);
		 if (newsockfd < 0) 
		     error("ERROR on accept");
	
		 printf("Connected to ");
		 printf("%d.%d.%d.%d\n",
	 		int(cli_addr.sin_addr.s_addr&0xFF),
	  		int((cli_addr.sin_addr.s_addr&0xFF00)>>8),
	  		int((cli_addr.sin_addr.s_addr&0xFF0000)>>16),
	  		int((cli_addr.sin_addr.s_addr&0xFF000000)>>24));
		 // FOR EACH NEW CLIENT START PROCESSING ITS REQUESTS
		 pid = fork();
		 if (pid < 0)
		     fatal_error("ERROR on fork");
		 if (pid == 0)  {
		     close(sockfd);
		     dostuff(newsockfd, cli_addr);
		     exit(0);
		 }
		 else close(newsockfd);
		} /* end of while */
		close(sockfd);
		return 0;
	}
	
	void printClientName(struct sockaddr_in cli_addr){
		printf("%d.%d.%d.%d ",
			 		int(cli_addr.sin_addr.s_addr&0xFF),
			  		int((cli_addr.sin_addr.s_addr&0xFF00)>>8),
			  		int((cli_addr.sin_addr.s_addr&0xFF0000)>>16),
			  		int((cli_addr.sin_addr.s_addr&0xFF000000)>>24));
	}

	/******** DOSTUFF() *********************
	 There is a separate instance of this function 
	 for each connection.  It handles all communication
	 once a connnection has been established.
	 *****************************************/
	void dostuff (int sock, struct sockaddr_in cli_addr)
	{
		unsigned char buffer[BUFFER_LIMIT];	
		struct message_s recvd;	
	   	int n; unsigned int pack_len;
		unsigned int key;
		SDES o;
	
		while(1){
			bzero(buffer, BUFFER_LIMIT);
			// Read the ecnrypted message
			if(!_read(sock, buffer, BUFFER_LIMIT-1))
				continue;
			recvd.unpack(buffer);

			if(recvd.type == KEY_REQUEST.type){
				printf(" Dispatching key to ");
				printClientName(cli_addr);
				printf("\n");
				
				srand(time(NULL));
				KEY_REPLY.key = (key = (rand()%1024));
				
				o.set_key(key);
				if(!_write(sock, KEY_REPLY.pack(pack_len), BUFFER_LIMIT-1))
					continue;
			}
			else if(recvd.type == MSG.type){
				printf("Message Received from Client ");
				printClientName(cli_addr);
				printf(" \nEncypted msg:\t%s\n", recvd.payload);
				
				//Decrypt the message and display on console
				o.Decrypt(recvd.payload);	
				printf("Decypted msg:\t%s\n", recvd.payload);
				
			}
			else if(recvd.type == QUIT_REQUEST.type){
				
				if(!_write(sock, QUIT_REPLY.pack(pack_len), BUFFER_LIMIT-1))
					continue;
				printf("Disconnecting from ");
				printClientName(cli_addr);	
				printf("\n");
				break;
			}
			else{
				error("ERROR: Unknown Message Type! Breaking Connection");
				break;
			}
		}
	}
