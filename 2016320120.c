#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 47500

#define FLAG_HELLO			((uint8_t) (0x01 << 7))
#define FLAG_INSTRUCTION 	((uint8_t) (0x01 << 6))
#define FLAG_RESPONSE 		((uint8_t) (0x01 << 5))
#define FLAG_TERMINATE 		((uint8_t) (0x01 << 4))

#define OP_ECHO 			((uint8_t)(0x00))
#define OP_INCREMENT 		((uint8_t)(0x01))
#define OP_DECREMENT 		((uint8_t)(0x02))

struct hw_packet{
    uint8_t flag;
    uint8_t operation;
    uint16_t data_len;
    uint32_t seq_num;
    uint8_t data[1024];
};

//function for sending packets
void send_p(int s, uint8_t flag, uint8_t op, uint16_t len, uint32_t seq, uint8_t *data);

int main(){
	FILE *fp;
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
	data_long=&tmp;
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

	uint32_t id = 2016320120;
	uint8_t tmp[4];
	memcpy(tmp, &id, sizeof(uint32_t));
	send_p(s, FLAG_HELLO, OP_ECHO, 4, 0, tmp);

	while(1){
		struct hw_packet rcvd_p;
		recv(s, (char*) &rcvd_p, sizeof(struct hw_packet), 0);

		printf("rcvd flag : %02X\n", rcvd_packet.flag);
		printf("rcvd op   : %02X\n", rcvd_packet.operation);
		printf("rcvd len  : %04X\n", rcvd_packet.data_len);
		printf("rcvd seq  : %08X\n", rcvd_packet.seq_num);
		printf("rcvd data : ");

		int i;
		for(i = 0; i<rcvd_p.data_len;i++){
			printf("%02X", rcvd_p.data[i]);
		}
		printf("\n\n");

		if(rcvd_p.flag == FLAG_INSTRUCTION){
			if(rcvd_p.operation != OP_ECHO){
				uint32_t tmp;
				memcpy(&tmp, rcvd_p.data, sizeof(uint32_t));
				if(rcvd_p.operation == OP_INCREMENT){
					tmp ++;
				}
				memcyp();
				
			}
		}
	}

}