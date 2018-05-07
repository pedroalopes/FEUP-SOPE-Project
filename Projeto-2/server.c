#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_SEATS 9999
#define MAX_CLI_SEATS 5
typedef struct {
	int num_room_seats;
	int num_ticket_offices;
	int open_time;
} Info;

typedef struct {
	int id; //numero do pedido
	int num_seats;
	char seats[10];

} Request;
Info *info;
pthread_t threads[20];
int fifo_escrita;
int fifo_leitura;
int pid_ans;
int seats[MAX_SEATS];
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar = PTHREAD_COND_INITIALIZER;
Request * buf;
int a = 0;
void process_request(Request* req);
void create_ticket_offices();
void * f(void * arg);
void *check_buffer(void *nr);
int main(int argc , char * argv[]){
	Request* req= malloc(sizeof(Request));
	buf=malloc(sizeof(Request));
	req->id=0;
	req->num_seats=1;
	strcpy(req->seats,"10");


	if (argc != 4){
		printf("Wrong number of arguments! Usage: %s [num_room_seats] [num_ticket_offices] [open_time] \n", argv[0]);
		exit(0);
	}

	info=malloc(sizeof(Info));
	info->num_room_seats=atoi(argv[1]);
	info->num_ticket_offices=atoi(argv[2]);
	info->open_time=atoi(argv[3]);
	create_ticket_offices();
	pthread_t test;
	pthread_cond_signal(&cvar);
	pthread_create(&test,NULL,f,"1");
	pthread_join(test, NULL);
	buf=req;
	a = 1;
	printf("%d\n", a);
	free(info);


}

void * f(void * arg) {

	pthread_mutex_lock(&mut);
	printf("signal\n" );
	pthread_cond_signal(&cvar);
	pthread_mutex_unlock(&mut);

}
void create_fifo_requests(){

		if (mkfifo("/tmp/requests", 0660) != 0) {
				if (errno == EEXIST)
					printf("SERVER: FIFO REQUESTS already exists\n");
				else
					printf("SERVER: CAN'T CREATE FIFO REQUESTS\n");
			}

			while ((fifo_leitura = open("tmp/requests", O_RDONLY| O_NONBLOCK)) == -1) {
				printf("SERVER: Waiting for REQUESTS'...\n");
			}
			return;

}
void open_requests(){
	int i ;
	char dir[30];
	Request *request=malloc(sizeof(Request));
	read(fifo_leitura,request,sizeof(Request));
	sprintf(dir, "tmp/ans%d",request->id);
	fifo_escrita=open(dir, O_WRONLY | O_NONBLOCK);
	if(request->num_seats>MAX_CLI_SEATS){
		i =-1;
		write(fifo_escrita, &i,sizeof(int));
	}

}

void create_ticket_offices(){
	int i;
	for (i = 0; i< info->num_ticket_offices;i++){
		pthread_create(&threads[i],NULL,check_buffer,"1");
		printf("Created thread\n");

	}
	int j;
	for (j = 0; j< info->num_ticket_offices;j++){
		pthread_join(threads[j],NULL);

	}
}

void process_request(Request* req){

	printf("process\n");
	a = 2;


	//pthread_exit(NULL);
}

void *check_buffer(void *nr){

	while(1){
		pthread_mutex_lock(&mut);
		while (a == 0){
			printf("check buffer \n");
			pthread_cond_wait(&cvar,&mut);
			printf("dentro while\n");
		}
		printf("fora while\n");
		process_request(buf);
		pthread_mutex_unlock(&mut);

	}



}
