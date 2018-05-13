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
#define NEW_REQUEST 1
#define TAKEN_REQUEST 0
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar = PTHREAD_COND_INITIALIZER;

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

typedef struct {
	int isFree;
	int clientId;
} Seat;

Info *info;
pthread_t threads[20];
int fifo_escrita;
int fifo_leitura;
int pid_ans;
time_t start_t;
FILE *f;

int new_request_flag = TAKEN_REQUEST;

Seat seats[MAX_SEATS];
Request * buf;

void create_ticket_offices();
void *check_buffer(void *nr);
void create_fifo_requests();
void open_requests();
int send_answer(char* answer, char* dir);
Request* req;

int main (int argc, char * argv[]) {

	char toFile[40];

	if (argc != 4){
		printf("Wrong number of arguments! Usage: %s [num_room_seats] [num_ticket_offices] [open_time] \n", argv[0]);

		exit(1);
	}
	else if (atoi(argv[2]) > 20){
		printf("SERVER::Cant have that many ticket offices\n");
		exit(1);
	}
	else if(atoi(argv[3]) == 0) {
		printf("SERVER:: open_time must be a positive number\n");
		exit(1);
 	}
	printf("open\n");
 	f=fopen("slog.txt","w");
 	fprintf(f,"AAAAA");

	buf=malloc(sizeof(Request));
	info=malloc(sizeof(Info));

	info->num_room_seats=atoi(argv[1]);
	info->num_ticket_offices=atoi(argv[2]);
	info->open_time=atoi(argv[3]);

	start_t = time(0);

	create_fifo_requests();
	create_ticket_offices();


	while(1) {
		if(read(fifo_leitura, buf, sizeof(Request)) > 0) {
			pthread_mutex_lock(&mut);
			new_request_flag = NEW_REQUEST;
			pthread_cond_signal(&cvar);
			pthread_mutex_unlock(&mut);
		}

	}

	int i;
	for (i = 0; i< info->num_ticket_offices;i++){
		pthread_join(threads[i],NULL);
	}

	for (i = 0; i< info->num_ticket_offices;i++){
 		if (i<10){
 			sprintf(toFile, "0%d-CLOSE",i);
 		}
 		else
 			sprintf(toFile,"%d-CLOSE",i);
 		fprintf(f,toFile);
 		pthread_cancel(threads[i]);
 	}

	free(info);
	free(buf);
	pthread_cond_destroy(&cvar);
	pthread_mutex_destroy(&mut);
	close(fifo_leitura);
	remove("requests");
	return 0;
}

void create_fifo_requests(){

	if (mkfifo("requests", 0660) != 0) {
		if (errno == EEXIST)
			printf("SERVER: FIFO REQUESTS already exists\n");
		else
			printf("SERVER: CAN'T CREATE FIFO REQUESTS\n");
	}


	while ((fifo_leitura = open("requests", O_RDONLY | O_NONBLOCK)) == -1) {
		printf("SERVER: Waiting for REQUESTS'...\n");
	}
	return;

}
void open_requests(){

	int i ;
	char dir[30], aux[30],finally[30], toFile[30];
	sprintf(dir, "ans%d",buf->id);
	strcpy(aux, buf->seats);

	if(buf->num_seats>MAX_CLI_SEATS){
		i =-1;
		sprintf(finally, "%d ",i);
		send_answer(finally, dir);
		strcat(toFile,"-MAX");
		fprintf(f,toFile);
		return;
	}
	char * split = strtok (aux," ");
	int count_seats =0;
	int seat;
	while (split != NULL)
	{
		seat=atoi(split);
		if (seat>9999 || seat<0){
			i =-3;
			sprintf(finally, "%d ",i);
			send_answer(finally, dir);
			strcat(toFile,"-IID");
			fprintf(f,toFile);
			return;
		}
		if (seats[seat].isFree!=0){
			i =-5;
			sprintf(finally, "%d ",i);
			send_answer(finally, dir);
			strcat(toFile,"-NAV");
			fprintf(f,toFile);
			return;
		}
		count_seats++;
		split= strtok (NULL, " ");
	}
	if (count_seats>MAX_CLI_SEATS || count_seats < buf->num_seats){
		i =-2;
		sprintf(finally, "%d ",i);
		send_answer(finally, dir);
		strcat(toFile,"-NST");
		fprintf(f,toFile);
		return;
	}
	int j;
	int count=0;
	for (j =0;j<MAX_SEATS;j++){
		if (seats[j].isFree==1)
		{
			count++;
		}
	}
	if (count==MAX_SEATS){
		i=-6;
		sprintf(finally, "%d ",i);
		send_answer(finally, dir);
		strcat(toFile,"-FUL");
		fprintf(f,toFile);
		return;
	}
	char success[30];
	strcpy(aux, buf->seats);

	sprintf(finally, "%d ", buf->num_seats);
	strcpy(success, finally);
	split = strtok (aux," ");
	count_seats=0;
	while (split != NULL && count_seats<buf->num_seats)
	{
		seat=atoi(split);
		seats[seat].isFree=1;
		printf("%d\n",buf->num_seats);
		sprintf(finally, "%d ",seat);
		strcat(success, finally);
		strcat (toFile, success);
		count_seats++;
		split= strtok (NULL, " ");
	}

	fprintf(f, toFile);

	if(send_answer(success, dir) == 1){
		printf("Could not send message\n");
		return;
	}
}

int send_answer(char* answer, char* dir) {

	sleep(4);
	if((fifo_escrita=open(dir, O_WRONLY | O_NONBLOCK)) == -1)
		return 1;

	write(fifo_escrita, answer, 30);
	printf("Answer nr %s handled with %s\n", dir, answer);
	return 0;

}

void create_ticket_offices(){

	int i;
	char toFile[20];

	for (i = 0; i< info->num_ticket_offices;i++){
 		if (i<10){
 			sprintf(toFile, "0%d-OPEN",i);
 		}
 		else
 			sprintf(toFile,"%d-OPEN",i);
 		fprintf(f,toFile);
		pthread_create(&threads[i],NULL,check_buffer, (void *) &threads[i]);
 		printf("Created thread\n");
 	}


}
int isSeatFree(Seat *seat, int seatNum){
	if (seatNum<0 || seatNum>MAX_SEATS)
		return -1;
	if (seats[seatNum].isFree==0)
		return seatNum;
	else return -2;

}
void bookSeat(Seat *seats, int seatNum, int clientId){
	if (seatNum<0 || seatNum>MAX_SEATS)
		return;
	if (seats[seatNum].isFree==0)
	{
		seats[seatNum].clientId=clientId;
		seats[seatNum].isFree=1;
		return;
	}


}
void freeSeat(Seat *seats, int seatNum){
	if (seatNum<0 || seatNum>MAX_SEATS)
		return;
	if (seats[seatNum].isFree==1)
	{

		seats[seatNum].clientId=-1;
		seats[seatNum].isFree=0;
	}

}

void *check_buffer(void * nr){

	while(1){
		pthread_mutex_lock(&mut);

		while (new_request_flag == TAKEN_REQUEST){
			printf("wait:%lu\n", * (long unsigned int *) nr);
			pthread_cond_wait(&cvar,&mut);
		}
		printf("Thread no %lu working\n", * (long unsigned int *) nr);
		printf("CondVar = %d\n", new_request_flag);
		new_request_flag = TAKEN_REQUEST;
		open_requests();
		pthread_mutex_unlock(&mut);
	}
	pthread_exit(NULL);

}
