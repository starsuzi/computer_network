#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 47500

#define FLAG_HELLO ((unsigned char) (0x01 << 7))
#define FLAG_INSTRUCTION ((unsigned char) (0x01 << 6))
#define FLAG_RESPONSE ((unsigned char) (0x01 << 5))
#define FLAG_TERMINATE ((unsigned char) (0x01 << 4))

#define OP_ECHO ((unsigned char)(0x00))
#define OP_INCREMENT ((unsigned char)(0x01))
#define OP_DECREMENT ((unsigned char)(0x02))

struct header{
    unsigned char flag;
    unsigned char operation;
    unsigned short data_len;
    unsigned int seq_num;
    char data[1024];
};

int main(){
    struct hostent *hp;
    struct sockaddr_in sin;
    char host[] = "127.0.0.1";
	int s; //socket file descripter

	char buf[1024];
	char buf_get[1024];
	char data_str[1024];
	int data_int;
	long long * data_long;
	long long tmp;
	int len;
	data_log = &tmp;
	int i;

	unsigned char flag_get[2];
	unsigned char operation_get[2];
	unsigned short seq_num_get[2];
	unsigned int data_len_get;

	/* translate host name into peer's IP address */
	//IP 주소를 이용해 hostent의 정보를 구함
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "unknown host: %s\n",host);
		exit(1);
	}
	
	/* build address data structure */
	bzero((char *)&sin, sizeof(sin)); //sin을 0으로 채움
	sin.sin_family = AF_INET; //IPv4 IP를 의미
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length); //hp의 주소를 sin주소에 복사
	sin.sin_port = htons(SERVER_PORT); //port설정
	
	/* active open */
	//socket 생성
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	
	//연결 요청
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect");
		close(s);
		exit(1);
	}

	//hw_packet 생성
	struct header *header_p;
	header_p = (struct header *)malloc(sizeof(struct header));
    
	//HELLO
	header_p -> flag = FLAG_HELLO;
	header_p -> operation = OP_ECHO;
	header_p -> data_len = htonl(4);
	header_p -> seq_num = htons((unsigned short)0); //비트 수 맞춤
	
	*data_long = htonl(2016320120);

	memcpy(buf, header_p, sizeof(struct header)); //copy header
	memcpy(buf+sizeof(struct header), data_long, 4);//copy data

	printf("*** starting ***\n\n");
	printf("sending first hello msg..\n");

	//읽어오기
	while(1){
		memset(buf_get,0,sizeof(buf_get));
		read(s,buf_get,1024);

		memset(flag_get,0,sizeof(unsigned char));
		memcpy(flag_get,buf_get,1);

		memset(operation_get,0,sizeof(unsigned char));
		memcpy(operation_get,buf_get+1,1);

		// Hello
		if(flag_get[0] == FLAG_HELLO){
			printf("received a hello message from the server! \n waiting for the first instruction message...\n");
		}

		// Instruction
		else if(flag_get[0] == FLAG_INSTRUCTION){
			seq_num_get = ntohs(*(unsigned short *)(buf_get+2));
			data_len_get = ntohl(*(int *)(buf_get+4));
			printf("received an instruction message! received data_len : %d bytes\n",data_len_get);

			//ECHO req
			if(operation_get[0] == OP_ECHO){
				//print recieved String
				printf("operation type is echo!\n");

				memset(data_str,0,strlen(data_str));

				for(i=0;i<data_len_get;i++){
					data_str[i] = (buf_get+sizeof(struct header))[i];
				}
				header_p -> flag = FLAG_RESPONSE;
				header_p -> operation = OP_ECHO;
				header_p -> seq_num = htons(seq_num_get);
				header_p -> data_len = htonl(data_len_get);

				memset(buf,0,sizeof(buf));
				memcpy(buf,header_p,sizeof(struct header)); //copy header
				memcpy(buf+sizeof(struct header), data_str, strlen(data_str)+1); //copy data
				len = sizeof(struct header) + data_len_get + 1;
				send(s,buf,len,0);

				printf("sent response message with seqence number %d to server\n\n\n",seq_num_get);

			}

			//Increment req
			else if(operation_get[0] == OP_INCREMENT){
				data_int = ntohl(*(int *)(buf_get+sizeof(struct header)));
				printf("operation type is increment!\n");

				data_int = htonl(++data_int);

				header_p -> flags = FLAG_RESPONSE;
				header_p -> operation = OP_ECHO;
				header_p -> seq_num = htons(seq_num_get); 
				header_p -> data_len = htonl(4);

				memset(buf,0,sizeof(buf));
				memcpy(buf, header_p, sizeof(struct header)); // header 복사 
				memcpy(buf + sizeof(struct header), &data_int, 4); // segmentation fault

				len = sizeof(struct header) + sizeof(data_int);
				send(s,buf,len,0);
				printf("sent response message with seqence number %d to server\n\n\n",seq_num_get);		
		
			}

			//Decrement req
			else {
				data_int = ntohl(*(int *)(buf_get+sizeof(struct header)));
				printf("operation type is decrement!\n");

				data_int = htonl(--data_int);

				header_p -> flags = FLAG_RESPONSE;
				header_p -> operation = OP_ECHO;
				header_p -> seq_num = htons(seq_num_get); 
				header_p -> data_len = htonl(4);

				memset(buf,0,sizeof(buf));
				memcpy(buf, header_p, sizeof(struct header)); // header 복사 
				memcpy(buf + sizeof(struct header), &data_int, 4); // segmentation fault

				len = sizeof(struct header) + sizeof(data_int);
				send(s,buf,len,0);
				printf("sent response message with seqence number %d to server\n\n\n",seq_num_get);		
			}
		}
		
		//Terminate
		else if(flag_get[0] == FLAG_TERMINATE){
			printf("received terminate msg! terminating...\n");
			close(s);
			return 0;
		}
	}
}
