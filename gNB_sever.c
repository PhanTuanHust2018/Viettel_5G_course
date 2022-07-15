#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <string.h>
#include <time.h>
#include <pthread.h>


struct PagingMessage
{
    int Message_type;
    int UE_ID;
    int TAC;
    int CN_Domain;
};
struct PagingMessage NgAP_Paging_AMF;
uint16_t SFN_gNB = 0,queue_number = 0;
int UE_ID = 0;	
uint16_t syn_flag_ID_UE,syn_flag_Qnumber = 0;
struct PagingMessage queue[1024];
void *Socket_sever_AMF(void *threadid) {
	printf("Thread socket sever AMF run!\n");
		int listenfd = -1;
		int connfd = -1;
		struct sockaddr_in server_addr;
		char send_buffer[1024];
		char recv_buffer[1024];
		int NgAP_Paging_buff[4];
		time_t ticks;
 
		memset(send_buffer, 0, sizeof(send_buffer));
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(&server_addr, 0, sizeof(server_addr));
		memset(&NgAP_Paging_buff, 0 ,sizeof(NgAP_Paging_buff));
		memset(&NgAP_Paging_AMF,0,sizeof(NgAP_Paging_AMF));

		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if(listenfd < 0) 
			printf("error create socket");		
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(10000);

		bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		listen(listenfd, 10);

		while(1) {
			connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
			while(syn_flag_ID_UE == 0){
			sleep(2);
			}
			/*-------------Send UE_ID--------------*/
			if(send(connfd, (void*)&UE_ID, sizeof(UE_ID),0) < 0 ) {
    			printf("send failed!\n");
			}

			/*-------------recive NgPaging message--------------*/
			if(recv(connfd,(void*)(&recv_buffer),sizeof(recv_buffer), 0)>=0){
				memcpy((void*)NgAP_Paging_buff,(void*)recv_buffer,sizeof(NgAP_Paging_buff));
				//pthread_mutex_lock(&pagingMsg_mutex);
				NgAP_Paging_AMF.Message_type = NgAP_Paging_buff[0];
				NgAP_Paging_AMF.UE_ID =  NgAP_Paging_buff[1];
				NgAP_Paging_AMF.TAC = NgAP_Paging_buff[2];
				NgAP_Paging_AMF.CN_Domain = NgAP_Paging_buff[3];
				//pthread_mutex_unlock (&pagingMsg_mutex);
				printf("\nNgAP_Paging_buff message receive from AMF: \n");
                printf("Message type: %d\n", NgAP_Paging_AMF.Message_type);
                printf("UE_ID: %d\n", NgAP_Paging_AMF.UE_ID);
                printf("TAC: %d\n", NgAP_Paging_AMF.TAC);
                printf("CN_Domain: %d\n\n", NgAP_Paging_AMF.CN_Domain);			
			}
			if(NgAP_Paging_AMF.Message_type == 100){
                if(NgAP_Paging_AMF.TAC == 100){
                    if(NgAP_Paging_AMF.CN_Domain == 100 || NgAP_Paging_AMF.CN_Domain == 101){
							int k;
							k = (SFN_gNB - NgAP_Paging_AMF.UE_ID)/64 + 1;
							queue_number = (NgAP_Paging_AMF.UE_ID + k*64)%1023;
							syn_flag_Qnumber = 1;
							queue[queue_number] = NgAP_Paging_AMF;
                    }
                }
            }
			close(connfd);
			sleep(1);
		}
		close(listenfd);
   	pthread_exit(NULL);
}
void *Socket_sever_UE(void *threadid) {
	printf("Thread socket sever UE run !\n");
		int listenfd = -1;
		int connfd = -1;
		struct sockaddr_in server_addr;
		char send_buffer[1024];
		char recv_buffer[1024];
		int pagingMsg[4];
		time_t ticks;

		memset(send_buffer, 0, sizeof(send_buffer));
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(&server_addr, 0, sizeof(server_addr));
		memset(&pagingMsg,0,sizeof(pagingMsg));
		memset(&UE_ID,0, sizeof(UE_ID));

		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if(listenfd < 0) 
			printf("error create socket");

		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(10001);

		bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		listen(listenfd, 10);
		while(1) {
			connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
			/*-----------recieve UE_ID from UE-------------*/ 		
			
			if(recv (connfd, (void*)&UE_ID, sizeof(UE_ID), 0) <= 0){
				printf("recive UE_ID fail");
			}
			printf("\nConnection detection: client UE_ID= %d connected\n\n",UE_ID);
			syn_flag_ID_UE = 1;
			/*-----------send RRC paging message-------------*/ 
			while(syn_flag_Qnumber == 0){
				sleep(2);
			}
			printf("\nQueue number of RRC paging message = %d\n\n",queue_number);
			while (1)
			{
				if(SFN_gNB == queue_number){
				pagingMsg[0] = queue[queue_number].Message_type;
				pagingMsg[1] = queue[queue_number].UE_ID;
				pagingMsg[2] = queue[queue_number].TAC;
				pagingMsg[3] = queue[queue_number].CN_Domain;
				printf("\nCopy NgAP_Paging_buff message to RRC Paging message send to UE: \n");
				printf("Message type: %d\n", pagingMsg[0]);
				printf("UE ID: %d\n", pagingMsg[1]);
				printf("TAC: %d\n", pagingMsg[2]);
				printf("CN_Domain: %d\n\n", pagingMsg[3]);
				if( send(connfd, (void*)&pagingMsg, sizeof(pagingMsg),0) < 0 ) {
    				printf("send failed!\n");
					}		
				break;
				}
				sleep(1);
			}
			syn_flag_Qnumber = 0;
			close(connfd);
		}
		close(listenfd);
   	pthread_exit(NULL);
}
void *coutingSFN_gNB(void *threadid){
	printf("Thread couting SFN_gNB run !\n");
	while(1)
	{
		if(SFN_gNB == 1023){
		SFN_gNB = 0;
		}
		else{
			SFN_gNB++;
		}
		printf("SFN_gNB =  %d\n", SFN_gNB);
		sleep(1);
	}
	pthread_exit(NULL);
}
int main(){
	pthread_t thread_AMF, thread_UE,thread_SFN_gNB;
	pthread_create(&thread_SFN_gNB,NULL, coutingSFN_gNB, NULL);
	pthread_create(&thread_UE, NULL, Socket_sever_UE, NULL);
	pthread_create(&thread_AMF, NULL, Socket_sever_AMF, NULL);
	pthread_join(thread_AMF, NULL);
	pthread_join(thread_UE, NULL);
	printf("\nEnd main\n");
	return 0;
}
