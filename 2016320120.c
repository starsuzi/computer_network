#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#define SERVER_PORT 47500
#define MAX_LINE 256
#define MAX_FILE_SIZE 12288

#define FLAG_HELLO		    ((uint8_t)(0x01 << 7))
#define FLAG_INSTRUCTION	((uint8_t)(0x01 << 6))
#define FLAG_RESPONSE		((uint8_t)(0x01 << 5))
#define FLAG_TERMINATE		((uint8_t)(0x01 << 4))

#define OP_ECHO		       	((uint8_t)(0x00))
#define OP_INCREMENT		((uint8_t)(0x01))
#define OP_DECREMENT		((uint8_t)(0x02))
#define OP_PUSH			    ((uint8_t)(0x03))
#define OP_DIGEST		    ((uint8_t)(0x04))

struct hw_packet {
  uint8_t  flag;
  uint8_t  operation;
  uint16_t data_len;
  uint32_t seq_num;
  uint8_t  data[1024];
};

void send_packet(int s, uint8_t flag, uint8_t op, uint16_t len, uint32_t seq, uint8_t *data);

int main(void) {
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host = "localhost";
    int s, file_index = 0;
    char file[MAX_FILE_SIZE];

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }

    printf("*** starting ***\n\n");

    uint32_t student_id = 2016320120;
    uint8_t tmp[4];
    
    struct hw_packet rcvd_packet;

    memcpy(tmp, &student_id, sizeof(uint32_t));
    printf("sending first hello msg...\n");

    send_packet(s, FLAG_HELLO, OP_ECHO, 4, 0, tmp);

    //recv
    recv(s, (char*) &rcvd_packet, sizeof(struct hw_packet), 0);
    printf("received hello message from the server!\n");
	printf("waiting for the first instruction message...\n");
    
    while(1) {
        
        recv(s, (char*) &rcvd_packet, sizeof(struct hw_packet), 0);

        if(rcvd_packet.flag == FLAG_INSTRUCTION) {
            if(rcvd_packet.operation == OP_ECHO){
                uint32_t tmp;
                memcpy(&tmp, rcvd_packet.data, sizeof(uint32_t));
                printf("operation type is echo.\n");
            }

            if(rcvd_packet.operation == OP_INCREMENT || rcvd_packet.operation == OP_DECREMENT) {
                uint32_t tmp;
                memcpy(&tmp, rcvd_packet.data, sizeof(uint32_t));
                tmp += (rcvd_packet.operation == OP_INCREMENT) ? 1 : -1;
                memcpy(rcvd_packet.data, &tmp, sizeof(uint32_t));
                
                send_packet(s, FLAG_RESPONSE, OP_ECHO, rcvd_packet.data_len, rcvd_packet.seq_num, rcvd_packet.data);

            } 
            else if(rcvd_packet.operation == OP_PUSH){
                
                memcpy(&(file[file_index]), rcvd_packet.data, rcvd_packet.data_len);

                printf("received push instruction!!\n");
                printf("received seq_num : %d\n", rcvd_packet.seq_num);
                printf("received data_len : %d\n", rcvd_packet.data_len);
                printf("saved bytes from index %d to %d\n", rcvd_packet.seq_num, (rcvd_packet.seq_num+rcvd_packet.data_len-1));
                printf("saved byte stream (character representation) : %s\n", rcvd_packet.data);
                printf("current file size is : %d!\n\n", rcvd_packet.seq_num + rcvd_packet.data_len);

                file_index += rcvd_packet.data_len;               
                send_packet(s, FLAG_RESPONSE, OP_PUSH, 0, 0, NULL);

            }
            else if(rcvd_packet.operation == OP_DIGEST){

                unsigned char hash_out[20];

                SHA1(hash_out, file, file_index);
                printf("received digest instruction!!\n");
                printf("********** calculated digest **********\n");
                int i;
                for (i = 0; i < 20; i++) {
                    printf("%02x", hash_out[i]);
                    if (i % 2 == 1)
                        printf(" ");
                    if (i % 8 == 7)
                        printf("\n");
            }
            
            printf("\n***************************************\n");
            printf("send digest message to server!\n");                
            send_packet(s, FLAG_RESPONSE, OP_DIGEST, 20, 0, hash_out);

            }
        }

        if(rcvd_packet.flag == FLAG_TERMINATE){
            printf("received terminate msg! terminating...");
            break;
        }
    }

    return 0;
}

void send_packet(int s, uint8_t flag, uint8_t op, uint16_t len, uint32_t seq, uint8_t *data) {
    struct hw_packet send_packet;
    send_packet.flag = flag;
    send_packet.operation = op;
    send_packet.data_len = len;
    send_packet.seq_num = seq;
    if(data != NULL)
    memcpy(send_packet.data, data, len);

    send(s, (char*) &send_packet, sizeof(struct hw_packet), 0);
}