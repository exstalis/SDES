#include "Utility.h"

using namespace Server_Client_Framework;
	int main(int argc, char *argv[])
	{
		/** variables for communication **/
		int sockfd, portno, n;
		struct sockaddr_in serv_addr;
		struct hostent *server;    
		char server_port[10], server_ip[50];
		unsigned char buffer[BUFFER_LIMIT];	
		struct message_s recvd;
		unsigned int pack_len;
		bool connected = false;
		bool key_acquired = false;
		SDES o;
		
	
		char cmd[200];
		while(1){
			fgets(cmd, 199, stdin);
			char* tokens[MAX_TOKENS];	int tokenCount = 0;
			getTokens(cmd, tokens, tokenCount);
			if(tokenCount == 0)	continue;

			if(!strcmp(tokens[0], "open")){
				//log("in");
				if(tokenCount < 3){
					printf("usage : open server_ip port\n");
					continue;
				}
				strcpy(server_ip, tokens[1]);
				strcpy(server_port, tokens[2]);
				portno = atoi(server_port);

				// OPEN A SOCKET WITH INTERNET_DOMAIN_ADDRESS AND CONTINUOUS STREAMING
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if (sockfd < 0) 
					fatal_error("ERROR opening socket");

				// TRY TO GET HOSTNAME			
				server = gethostbyname(server_ip);
				if (server == NULL) {
					fprintf(stderr,"ERROR, no such host\n");
					close(sockfd);
					continue;
				}
			
				bzero((char *) &serv_addr, sizeof(serv_addr));	// Initialise serv_addr to zero
			
				// SET serv_addr struct attributes
				serv_addr.sin_family = AF_INET;
				bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
				serv_addr.sin_port = htons(portno);
			
				// TRY TO CONNECT
				if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
					error("ERROR connecting");
					close(sockfd);
					continue;
				}
				else
					connected = true;
			}
			else if(!strcmp(tokens[0], "getkey")){
				if(!connected){
					printf("ERROR: Not Connected to Server\n");
					continue;
				}
				
				if(!_write(sockfd, KEY_REQUEST.pack(pack_len), BUFFER_LIMIT-1))
					continue;

				bzero(buffer,recvd.length+10);
				if(!_read(sockfd, buffer, BUFFER_LIMIT-1))
					continue;
				recvd.unpack(buffer);
				
				if(recvd.type == KEY_REPLY.type){
					o.set_key(recvd.key);
					key_acquired = true;
				}
				else{
					printf("Unexpected packet received from Server; packet is not KEY_REPLY\n");
					close(sockfd);
				}
			}
			else if(!strcmp(tokens[0], "send")){
				if(!key_acquired){
					printf("ERROR: key not acquired by Server\n");
					continue;
				}
				
				char msg[1024];	msg[0] = 0;
				for(int i = 1; i < tokenCount; ++i){
					strcat(msg, tokens[i]);
					if(i != tokenCount-1)
						strcat(msg, " ");
				}
				o.Encrypt(msg);
				MSG.payload = msg;
				MSG.length = HEADER_LENGTH + strlen(MSG.payload) + 1;
				if(!_write(sockfd, MSG.pack(pack_len), BUFFER_LIMIT-1))
					continue;
			}
			else if(!strcmp(tokens[0], "quit")){
				if(!_write(sockfd, QUIT_REQUEST.pack(pack_len), BUFFER_LIMIT-1))
					continue;
				bzero(buffer, BUFFER_LIMIT);
				if(!_read(sockfd, buffer, BUFFER_LIMIT-1))
					continue;
				recvd.unpack(buffer);
				if(recvd.type == QUIT_REPLY.type){
					close(sockfd);
					break;
				}
				else{
					printf("Unexpected packet received from Server; packet is not QUIT_REPLY\n");
				}
			}
			else{
				printf("usage : \n");
				printf("open <ip> <port>\n");
				printf("getkey\n");
				printf("send <message>\n");
				printf("quit\n");
			}
		}
	return 0;
	}
