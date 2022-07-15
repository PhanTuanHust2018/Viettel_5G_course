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
struct PagingMessage RRC_paging_UE;
int UE_ID;
void *Socket_client_gNB(void *threadid) {
	printf("Thread client UE create!");
		int sockfd = -1;
        struct sockaddr_in server_addr;
        char recv_buffer[1024];
        int pagingMsg[4];

        memset(recv_buffer, 0, sizeof(recv_buffer));
        memset(&server_addr, 0, sizeof(server_addr));
        memset(&pagingMsg,0,sizeof(pagingMsg));
        memset(&RRC_paging_UE,0,sizeof(RRC_paging_UE));

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_port = htons(10001);
        bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            /*-----------send UE_ID-------------*/     

            if(send(sockfd, (void*)&UE_ID, sizeof(UE_ID),0) < 0 ) {
    			printf("send failed!\n");
			}
            printf("\nSend [UE_ID = random(0,60) = %d] to gNB\n", UE_ID);
            /*-----------receive Paging message-------------*/   
            
            while(recv ( sockfd, (void*)&recv_buffer, sizeof(recv_buffer), 0) <= 0){
                sleep(5);
            }
            memcpy((void*)pagingMsg,(void*)recv_buffer,sizeof(pagingMsg));
            printf("\nMessage receive from gNB: \n");
            printf("Message type: %d\n", pagingMsg[0]);
            printf("UE ID: %d\n", pagingMsg[1]);
            printf("TAC: %d\n", pagingMsg[2]);
            printf("CN_Domain: %d\n", pagingMsg[3]);

            RRC_paging_UE.Message_type = pagingMsg[0];
            RRC_paging_UE.UE_ID = pagingMsg[1];
            RRC_paging_UE.TAC = pagingMsg[2];
            RRC_paging_UE.CN_Domain = pagingMsg[3];

            if(RRC_paging_UE.Message_type == 100){
                if(RRC_paging_UE.UE_ID == UE_ID){
                    printf("\nRecieve Paging message succesfully!\n");
                }   
            }
        }
        else 
            printf("\nconnect socket error\n");
        close(sockfd);
   	pthread_exit(NULL);
}
int random_(int a, int b){
    time_t t;
    int random_number;
	srand((unsigned) time(&t));
    random_number = rand() % 60;
    return random_number;
}
int main(){
	pthread_t thread_gNB;
    UE_ID = random_(0,60);
	pthread_create(&thread_gNB, NULL, Socket_client_gNB, NULL);
	pthread_join(thread_gNB, NULL);
	printf("\nEnd main\n");
	return 0;
}
