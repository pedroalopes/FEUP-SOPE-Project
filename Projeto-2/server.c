
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

#define DELAY() usleep(50)
#define MAX_SEATS 9999
#define MAX_CLI_SEATS 99
#define NEW_REQUEST 1
#define TAKEN_REQUEST 0

#define QUOTE(str) __QUOTE(str)
#define __QUOTE(str)  #str
#define WIDTH_SEAT 4

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
	char seats[200];

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
int open_ticket_offices = 0;
time_t start_t;


int new_request_flag = TAKEN_REQUEST;

Seat seats[MAX_SEATS];
Request * buf;

void create_ticket_offices();
void *check_buffer(void *nr);
void create_fifo_requests();
void open_requests();
int send_answer(char* answer, char* dir);
void bookSeat(Seat *seats, int seatNum, int clientId);
int isSeatFree(Seat *seat, int seatNum);

Request* req;

int main (int argc, char * argv[]) {
	FILE *f;
	FILE *fp;
	FILE *fc;
	FILE *fcp;
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
	fc=fopen("clog.txt", "w");
	fcp=fopen("cbook.txt", "w");
	fclose(fc);
	fclose(fcp);
	f=fopen("slog.txt","w");
	fclose(f);
	fp=fopen("sbook.txt","w");
	buf=malloc(sizeof(Request));
	info=malloc(sizeof(Info));

	info->num_room_seats=atoi(argv[1]);
	info->num_ticket_offices=atoi(argv[2]);
	info->open_time=atoi(argv[3]);
	start_t = time(0);

	create_fifo_requests();
	create_ticket_offices();

	start_t = time(0);

	while(time(0) - start_t < info->open_time) {
		if(read(fifo_leitura, buf, sizeof(Request)) > 0) {
			pthread_mutex_lock(&mut);
			new_request_flag = NEW_REQUEST;
			pthread_cond_signal(&cvar);
			pthread_mutex_unlock(&mut);
		}
	}

	open_ticket_offices = 0;
	f = fopen("slog.txt", "a");
	int i;
	for (i = 1; i <= info->num_ticket_offices;i++){

		sprintf(toFile,"%02d-CLOSE\n",i);
		fprintf(f,"%s",toFile);

		pthread_mutex_lock(&mut);
		pthread_cond_signal(&cvar);
		pthread_mutex_unlock(&mut);
	}


	for (i = 0; i< info->num_ticket_offices;i++){
		pthread_join(threads[i],NULL);
	}

	fprintf(f, "%s", "SERVER CLOSE\n");

	free(info);
	free(buf);
	fclose(f);
	fclose(fp);
	close(fifo_leitura);
	unlink("requests");
	remove("requests");
	pthread_cond_destroy(&cvar);
	pthread_mutex_destroy(&mut);
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
	FILE *f=fopen("slog.txt","a");
	FILE *fp=fopen("sbook.txt","a");
	int i ;
	char dir[100], aux[10000],finally[10000], toFile[10000],type[20];
	sprintf(dir, "ans%d",buf->id);
	strcpy(aux, buf->seats);


	if(buf->num_seats>MAX_CLI_SEATS){
		i =-1;
		sprintf(toFile,"%d-%d: %d %d ... -",buf->id,buf->num_seats, buf->seats[0], buf->seats[1]);
		strcpy(type,"MAX\n");
		strcat(toFile,type);
		fprintf(f,"%s",toFile);
		sprintf(finally, "%d ",i);
		send_answer(finally, dir);
		fclose(f);
		return;
	}

	sprintf(toFile,"%d-%d: %s -",buf->id,buf->num_seats,buf->seats);

	char * split = strtok (aux," ");
	int count_seats =0;
	int seat, return_value;

	while (split != NULL)
	{
		seat=atoi(split);
		return_value = isSeatFree(seats, seat);
		if (return_value == -1){
			i =-3;
			strcpy(type,"IID\n");
			strcat(toFile,type);
			fprintf(f,"%s",toFile);
			sprintf(finally, "%d ",i);
			send_answer(finally, dir);
			fclose(f);
			return;
		}
		if (return_value == -2){
			i =-5;
			strcat(toFile,"NAV\n");
			fprintf(f,"%s",toFile);
			sprintf(finally, "%d ",i);
			send_answer(finally, dir);
			fclose(f);
			return;
		}
		count_seats++;
		split= strtok (NULL, " ");
	}

	if (count_seats>MAX_CLI_SEATS || count_seats < buf->num_seats){
		i =-2;
		strcat(toFile,"NST\n");
		fprintf(f,"%s",toFile);
		sprintf(finally, "%d ",i);
		send_answer(finally, dir);
		fclose(f);
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
		strcat(toFile,"-FUL\n");
		fprintf(f,"%s",toFile);
		fclose(f);
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
		fprintf(fp,"%0"QUOTE(WIDTH_SEAT)"d",seat);
		fprintf(fp,"\n");
		bookSeat(seats, seat, buf->id);
		sprintf(finally, "%d ",seat);
		strcat(success, finally);
		sprintf(finally,"%04d ",seat);

		strcat (toFile, finally);
		count_seats++;
		split= strtok (NULL, " ");
	}

	DELAY();

	if(send_answer(success, dir) == 1){
		printf("Could not send message\n");
		return;
	}
	strcat(toFile, "\n");
	fprintf(f,"%s",toFile);
	fclose(f);
	fclose(fp);
}

int send_answer(char* answer, char* dir) {
	if((fifo_escrita=open(dir, O_WRONLY | O_NONBLOCK)) == -1)
		return 1;

	write(fifo_escrita, answer, 30);
	printf("Answer nr %s handled with %s\n", dir, answer);
	return 0;

}

void create_ticket_offices(){
	FILE *f=fopen("slog.txt","a");
	int i;
	char toFile[20];
	open_ticket_offices = 1;

	for (i = 1; i <= info->num_ticket_offices;i++){

		sprintf(toFile,"%02d-OPEN\n",i);
		fprintf(f,"%s",toFile);
		pthread_create(&threads[i],NULL,check_buffer, (void *) &threads[i]);
		printf("Created thread\n");

	}

	fclose(f);

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
			printf("thread %lu waiting for client...\n", * (long unsigned int *) nr);
			pthread_cond_wait(&cvar,&mut);

			if(open_ticket_offices == 0){
				pthread_mutex_unlock(&mut);
				pthread_exit(NULL);
			}

		}
		printf("Thread no %lu working...\n", * (long unsigned int *) nr);
		new_request_flag = TAKEN_REQUEST;
		open_requests();
		pthread_mutex_unlock(&mut);
	}

}
