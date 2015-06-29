#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include "SDES.h"
using namespace std;
using namespace SDES_Project;

namespace Server_Client_Framework{
	
	const int MAX_TOKENS = 10;
	const int MAX_TOKEN_SIZE = 30;
	void getTokens(char* str, char** tokens, int& cnt){
		int len = strlen(str);
		int i = 0, pos = 0;
	
		while(str[i]){
			while(str[i] && str[i] == ' ')	i++;
			if(!str[i] || str[i] == '\n' || str[i] == '\r')	break;
		
			tokens[pos] = new char[MAX_TOKEN_SIZE];
			int j = 0;
			while(str[i] && str[i] != ' ' && str[i] != '\n' && str[i] != '\r')
				tokens[pos][j++] = str[i++];
			tokens[pos][j] = '\0';
			pos++;

			while(str[i] && str[i] == ' ')	i++;
		}
		cnt = pos;
	}

	void log(const char* msg){
		printf("LOG: %s\n", msg);
	}

	void error(const char *msg)
	{
	    perror(msg);
	}

	void fatal_error(const char* msg)
	{
	    perror(msg);
	    exit(0);
	}

	bool _write(int sockfd, unsigned char* buffer, size_t len){
		int n = write(sockfd, buffer, len);
		if (n < 0){
			error("ERROR writing to socket");		
			return false;
		}
		return true;
	}

	bool _read(int sockfd, unsigned char* buffer, size_t len){
		int n = read(sockfd, buffer, len);
		if (n < 0){
			error("ERROR reading from socket");		
			return false;
		}
		return true;
	}

	const unsigned int HEADER_LENGTH = 2*sizeof(unsigned int) + sizeof(unsigned char);

	struct message_s {
		unsigned char type; /* type (1 byte) */
		unsigned int length;/* length (header + payload) (4 bytes) */
		unsigned int key;
		char* payload;
		unsigned char* pack(unsigned int& len);
		void unpack(unsigned char* buffer);
		void init(){
			this->length = HEADER_LENGTH;
			this->payload = NULL;	
		}
		message_s(){	/* constructor */
			 init();
		}
		message_s(unsigned char type){
			init();
			this->type = type;
			this->key = 0;
		}

	}__attribute__ ((packed));


	/** SOME PACKET DEFINITIONS NECESSARY FOR SERVER CLIENT INTERACTION **/
	struct message_s KEY_REQUEST = message_s(0xA1),
			 KEY_REPLY   = message_s(0xA2),

			 QUIT_REQUEST	   = message_s(0xA3),
			 QUIT_REPLY	   = message_s(0xA4),

			 MSG	   = message_s(0xFF);


	unsigned char* toChar(unsigned int x){
		unsigned char* ch = new unsigned char[sizeof(x)];
		memcpy( ch, (unsigned char*)&x, sizeof(x) );
		return ch;
	}

	unsigned int toInt(unsigned char* ch){
		unsigned int* x = new unsigned int;
		memcpy( x, (unsigned int*)ch, sizeof(ch) );
		return *x;
	}

	const unsigned int BUFFER_LIMIT  = 1024;
	unsigned char* buffer = new unsigned char[BUFFER_LIMIT];

	unsigned char char2UnsignedChar(char c){
		return ( (unsigned char)((int)c + 128) );
	}

	char unsignedChar2Char(unsigned char c){
		return ((char) ((int)c - 128) );
	}

	unsigned char* message_s::pack(unsigned int& len){
	
		int pos = 0;
		buffer[pos++] = type;
		unsigned char* tmp = toChar(length);
		for(int i = 0; i < sizeof(length); ++i)
			buffer[pos++] = tmp[i];
		tmp = toChar(key);
		for(int i = 0; i < sizeof(key); ++i)
			buffer[pos++] = tmp[i];

		if(payload != NULL)
		for(int  i = 0; i < strlen(payload); ++i)
			buffer[pos++] = (unsigned char)payload[i];
		buffer[pos++] = '\0';
		len = pos;
		return buffer;
	}

	void message_s::unpack(unsigned char* ubuffer){
		int pos = 0;
		type = ubuffer[pos++];
		unsigned char tmp[sizeof(unsigned int)];
		for(int i = 0; i < sizeof(unsigned int); ++i)
			tmp[i] = ubuffer[pos++];
		length = toInt(tmp);

		for(int i = 0; i < sizeof(unsigned int); ++i)
			tmp[i] = ubuffer[pos++];
		key = toInt(tmp);		

		int restBytes = length - (HEADER_LENGTH);
		payload = new char[restBytes];
		for(int i = 0; i < restBytes - 1; ++i)
			payload[i] = (char)ubuffer[pos++];
		payload[restBytes-1] = '\0';
	}

}
