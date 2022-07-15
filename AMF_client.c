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
void *Socket_sever_gNB(void *threadid) {
	printf("Hello World! Thread ID");
		int listenfd = -1;
		int connfd = -1;
		struct sockaddr_in server_addr;
		char send_buffer[1024];
		time_t ticks;

		memset(send_buffer, 0, sizeof(send_buffer));
		memset(&server_addr, 0, sizeof(server_addr));

		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(10000);

		bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		listen(listenfd, 10);

		while(1) {
			connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
			ticks = time(NULL);
			sprintf(send_buffer, "Server reply %s", ctime(&ticks));
			write(connfd, send_buffer, strlen(send_buffer));
			close(connfd);
		}
		close(listenfd);
   	pthread_exit(NULL);
}
void *Socket_client_gNB(void *threadid) {
    struct PagingMessage page_msg;
	printf("Thread client AMF create!");
		int sockfd = -1;
        struct sockaddr_in server_addr;
        char recv_buffer[1024];
		char send_buffer[1024];
		int pagingMsg[4];
		int UE_ID;
        time_t ticks;

        memset(recv_buffer, 0, sizeof(recv_buffer));
        memset(&server_addr, 0, sizeof(server_addr));
		memset(&pagingMsg, 0, sizeof(pagingMsg));
		memset(&UE_ID,0,sizeof(UE_ID));

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_port = htons(10000);

		NgAP_Paging_AMF.Message_type = 100;
		NgAP_Paging_AMF.UE_ID = 5;
		NgAP_Paging_AMF.TAC = 100;
		NgAP_Paging_AMF.CN_Domain = 100;


        bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            if(recv ( sockfd, (void*)&UE_ID, sizeof(UE_ID), 0) >= 0){
                printf("\nUE_ID receive =  %d\n", UE_ID);
            }
			NgAP_Paging_AMF.UE_ID = UE_ID;
			pagingMsg[0] = NgAP_Paging_AMF.Message_type;
			pagingMsg[1] = NgAP_Paging_AMF.UE_ID;
			pagingMsg[2] = NgAP_Paging_AMF.TAC;
			pagingMsg[3] = NgAP_Paging_AMF.CN_Domain;
			if( send(sockfd, (void*)&pagingMsg, sizeof(pagingMsg),0) < 0 ) {
    			printf("send failed!\n");
			}
			printf("\nSend NgAp Paging message to gNB: \n");
			printf("\n%d\n", pagingMsg[0]);
			printf("\n%d\n", pagingMsg[1]);
			printf("\n%d\n", pagingMsg[2]);
			printf("\n%d\n", pagingMsg[3]);	
        }
        else 
            printf("error");

        close(sockfd);
   	pthread_exit(NULL);
}
int main(){
	pthread_t thread_gNB;
	pthread_create(&thread_gNB, NULL, Socket_client_gNB, NULL);
	pthread_join(thread_gNB, NULL);
	printf("\nend main\n");
	return 0;
}
