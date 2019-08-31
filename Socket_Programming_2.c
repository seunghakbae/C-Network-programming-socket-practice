#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

#define PORTNUM 47500
#define FLAG_HELLO ((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION ((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE ((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE ((unsigned char)(0x01 <<4))

#define OP_ECHO ((unsigned char)(0x00))
#define OP_INCREMENT ((unsigned char)(0x01))
#define OP_DECREMENT ((unsigned char)(0x02))


struct hw_packet{
 unsigned char flag;
 unsigned char operation;
 unsigned short data_len;
 unsigned int seq_num;
 char data[1024];
};

int main(void){

 int sd;
 int len;
 struct sockaddr_in sin;
 struct hw_packet buf_struct;
 struct hw_packet buf_struct_rcv;
 struct hw_packet buf_struct_resend;

 if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
  perror("socket");
  exit(1);
 }

 memset((char *)&sin, '\0', sizeof(sin));
 sin.sin_family = AF_INET;
 sin.sin_port = htons(PORTNUM);
 sin.sin_addr.s_addr = inet_addr("127.0.0.1");
 
 if(connect(sd, (struct sockaddr *)&sin, sizeof(sin))){
  perror("connect");
  exit(1);
 }

 unsigned int value;
 buf_struct.flag = FLAG_HELLO;
 buf_struct.operation = OP_ECHO;
 buf_struct.data_len = 4;
 buf_struct.seq_num = 0;
 value = 2013210069;
 memcpy(buf_struct.data, &value, sizeof(unsigned int));

 if(send(sd, &buf_struct, sizeof(buf_struct), 0) == -1){
  perror("send");
  exit(1);
 }

 printf("sending first hello msg...\n");

 recv(sd, &buf_struct_rcv, sizeof(struct hw_packet), 0);

 switch(buf_struct_rcv.flag){
  case FLAG_HELLO:
   printf("received hello message from the server!\n");
   break;

  case FLAG_INSTRUCTION:
   printf("Instruction\n");
   break;
 
  case FLAG_RESPONSE:
   printf("Response\n");
   break;

  case FLAG_TERMINATE:
   printf("TErminate\n");
   break;
 }
 
 printf("waiting for the first instruction message...\n");
 printf("\n"); 

 while(true){

  recv(sd, &buf_struct_rcv, sizeof(struct hw_packet), 0);
  
  unsigned char flag = buf_struct_rcv.flag;

  if(flag == FLAG_INSTRUCTION){
    printf("received instrucion message!\n");
    printf("received data len : %d\n", buf_struct_rcv.data_len);
    unsigned char operation = buf_struct_rcv.operation;
    char *data2;
    unsigned int value;    

    switch(operation){
     case OP_ECHO:
      printf("operation type is ECHO.\n");
      data2 = buf_struct_rcv.data;
      printf("echo : %s\n", data2);
      
      buf_struct_resend.flag = FLAG_RESPONSE;
      buf_struct_resend.operation = buf_struct_rcv.operation;
      buf_struct_resend.data_len = buf_struct_rcv.data_len;
      buf_struct_resend.seq_num = buf_struct_rcv.seq_num;
      strncpy(buf_struct_resend.data, data2, sizeof(buf_struct_resend.data)-1);
      
      if(send(sd, &buf_struct_resend, sizeof(buf_struct), 0) == -1){
       perror("send");
       exit(1);
      }
     
      printf("send response msg with seq.num. %d to server\n", buf_struct_resend.seq_num);
      printf("\n");
     
      break;

     case OP_INCREMENT:
      printf("operation type is increment.\n");
      memcpy(&value, buf_struct_rcv.data, sizeof(unsigned int));
      printf("increment : %d\n", ++value);
      
      buf_struct_resend.flag = FLAG_RESPONSE;
      buf_struct_resend.operation = ((unsigned char)(0x00));
      buf_struct_resend.data_len = buf_struct_rcv.data_len;
      buf_struct_resend.seq_num = buf_struct_rcv.seq_num;
      memcpy(buf_struct_resend.data, &value, sizeof(unsigned int));

      if(send(sd, &buf_struct_resend, sizeof(buf_struct), 0) == -1){
       perror("send");
       exit(1);
      }
     
      printf("send response msg with seq.num. %d to server\n", buf_struct_resend.seq_num);
      printf("\n");
     
      break;
    
     case OP_DECREMENT:
      printf("operation type is decrement.\n");
      memcpy(&value, buf_struct_rcv.data, sizeof(unsigned int));
      printf("decrement : %d\n", --value);
          
      buf_struct_resend.flag = FLAG_RESPONSE;
      buf_struct_resend.operation = ((unsigned char)(0x00));
      buf_struct_resend.data_len = buf_struct_rcv.data_len;
      buf_struct_resend.seq_num = buf_struct_rcv.seq_num;
      memcpy(buf_struct_resend.data, &value, sizeof(unsigned int));
      
      if(send(sd, &buf_struct_resend, sizeof(buf_struct), 0) == -1){
       perror("send");
       exit(1);
      }
     
      printf("send response msg with seq.num. %d to server\n", buf_struct_resend.seq_num);
      printf("\n");
     
      break;
  }
 }else if(flag == FLAG_TERMINATE){
  printf("received terminate msg! terminating...\n");
  break;
  close(sd);
  }
 }
 
 return 0;
}
