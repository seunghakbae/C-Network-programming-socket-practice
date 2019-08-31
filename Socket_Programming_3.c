#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stdio.h>

#define PORTNUM 47500
#define FLAG_HELLO ((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION ((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE ((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE ((unsigned char)(0x01 << 4))

#define OP_ECHO ((unsigned char)(0x00))
#define OP_INCREMENT ((unsigned char)(0x01))
#define OP_DECREMENT ((unsigned char)(0x02))
#define OP_PUSH ((unsigned char)(0x03))
#define OP_DIGEST ((unsigned char)(0x04))

struct hw_packet{
 unsigned char flag;
 unsigned char operation;
 unsigned short data_len;
 unsigned int seq_num;
 char data[1024];
};

int main(void){

 int sd;
 struct sockaddr_in sin; 
 struct hw_packet buf_struct, buf_struct_rcv, buf_struct_reply, buf_struct_digest;
 unsigned char flag;
 unsigned char op;
 unsigned int seq_num;
 unsigned short data_len;
 char* data;
 char buffer[12288];
 memset(buffer, '\0', sizeof(buffer)); 

 if((sd = socket(AF_INET, SOCK_STREAM,0)) == -1){
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
 
 buf_struct.flag = FLAG_HELLO;
 buf_struct.operation = 0;
 buf_struct.data_len = 4;
 buf_struct.seq_num = 0;
 unsigned int value = 2013210069;

 memcpy(buf_struct.data, &value, sizeof(unsigned int));

 if(send(sd, &buf_struct, sizeof(buf_struct), 0) == -1){
  perror("send");
  exit(1);
 }

 printf("sending first hello message...\n");
 
 recv(sd, &buf_struct, sizeof(struct hw_packet), 0);
 
 flag = buf_struct.flag;

 if(flag == FLAG_HELLO)
  printf("received hello message from the server!\n");

 printf("waiting for the first instruction message...\n"); 
 int index = 0;
 int error = 0;

 while(true){
  recv(sd, &buf_struct_rcv, sizeof(struct hw_packet),0);
  op = buf_struct_rcv.operation;
  flag = buf_struct_rcv.flag;

  if(flag == FLAG_TERMINATE){
   printf("received terminate message! terminating...\n");
   break;
  } 
 
  if(op == OP_PUSH){
     printf("received push instruction!!\n");
     data = NULL;

     seq_num = buf_struct_rcv.seq_num;
     data_len = buf_struct_rcv.data_len;
     data = buf_struct_rcv.data;
     
     printf("************************");

     char copy_array[strlen(data)];
     strncpy(copy_array, data, strlen(data));
     
     int j = 0;
     int i;
 
     for(i = 0; i < data_len; i++){
      buffer[index + i] = data[i];
     }

     
     /*for(i = seq_num; i < seq_num + data_len; i++){
      buffer[i] = copy_array[j];
      j++;
     }*/

    /* printf("\n\n");
     printf("%s\n", buffer);
     printf("\n\n");*/
    
     printf("received seq_num : %d\n", seq_num);
     printf("received data_len : %d\n", data_len);
     printf("save bytes from index %d to %d \n", index, index + data_len);
     //printf("saved byte stream (character representation) : %s \n", data);
     printf("save byte stream (character representation) : ");
     int k;
     for(k = 0; k < data_len; k++){
      printf("%c", buffer[index + k]);
     }
     printf("\n");
     index += data_len;
     printf("current file size is : %d !\n\n", seq_num + data_len); 
   

     buf_struct_reply.flag = FLAG_RESPONSE;
     buf_struct_reply.operation = OP_PUSH;
     buf_struct_reply.data_len = 0;
     buf_struct_reply.seq_num = 0;
     memset((char *)&buf_struct_reply.data, '\0', sizeof(buf_struct_reply.data));     

     if(send(sd, &buf_struct_reply, sizeof(buf_struct), 0) == -1){
      perror("send");
      exit(1);
     } 
    
  }else if(op == OP_DIGEST){
    
     printf("received digest instruction!!\n");
     printf("********** calculated digest **********\n\n"); 
    
     char hash_out[20];
       
     SHA1(hash_out, buffer, strlen(buffer));
     
     setlocale("en_US.UTF-8");

    // printf("%ls\n",(wchar_t*)hash_out);
    int i;
     for(i =0; i < 20; i++){
      printf("%2x\n", hash_out[i]);
     }

     printf("***************************************\n\n");
     

     buf_struct_digest.flag = FLAG_RESPONSE;
     buf_struct_digest.operation = OP_DIGEST;
     buf_struct_digest.data_len = 20;
     buf_struct_digest.seq_num = 0;
     memcpy(buf_struct_digest.data, hash_out, seq_num);

     if(send(sd, &buf_struct_digest, sizeof(buf_struct_digest), 0) == -1){
      perror("send");
      exit(1);
     }

     printf("index : %d", index);

     printf("\nsent digest message to server!\n");
  }
 }
}
