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
#include <semaphore.h>

#define MAX_SEATS 9999
#define MAX_CLI_SEATS 5
#define NEW_REQUEST 1
#define TAKEN_REQUEST 0
#define SHARED 0
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
sem_t *new_client;


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

	int i;
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
	f=fopen("slog.txt","ab+");

	printf("SERVER::CREATED SERVER\n");

	info=malloc(sizeof(Info));
	info->num_room_seats=atoi(argv[1]);
	info->num_ticket_offices=atoi(argv[2]);
	info->open_time=atoi(argv[3]);

	start_t = time(0);
	if((new_client = sem_open("/new_client", O_CREAT, 0777, 0)) == SEM_FAILED)
		printf("ERROR::Could not create semaphore\n");

	create_ticket_offices();
	create_fifo_requests();

	while(time(0) - start_t < info->open_time) {
		if(new_request_flag == TAKEN_REQUEST) {

			sleep(1);
			printf("before read\n");

			sem_wait(new_client);

			printf("after sem\n");

			while ((fifo_leitura = open("requests", O_RDONLY)) == -1) {
				printf("SERVER: Waiting for REQUESTS'...\n");
			}

			printf("after while\n");
			if(read(fifo_leitura, buf, sizeof(Request)) > 0){
				printf("inside request\n");
				new_request_flag = NEW_REQUEST;
				pthread_cond_signal(&cvar);
			}
		}
	}

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
	free(req);
	free(buf);
	fclose(f);
	pthread_cond_destroy(&cvar);
	sem_close(new_client);
	sem_unlink("new_client");
	close(fifo_leitura);
	unlink("requests");
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
	char dir[30], aux[30],finally[30],toFile[50];
	Request *request = malloc(sizeof(Request));
	strcpy(aux, buf->seats);
	sprintf(toFile, "CL%d-%d:",buf->id,buf->num_seats);
	strcat(toFile,buf->seats);
	if(buf->num_seats>MAX_CLI_SEATS){
		i =-1;
		sprintf(finally, "%d ",i);
		write(fifo_escrita, finally,30);
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

			write(fifo_escrita, finally,30);
			strcat(toFile,"-IID");
			fprintf(f,toFile);

			return;
		}
		if (seats[seat].isFree!=0){
			i =-5;
			sprintf(finally, "%d ",i);
			write(fifo_escrita, finally,30);
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
		write(fifo_escrita, finally,30);
		strcat(toFile,"-NST");
		fprintf(f,toFile);
		return ;
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
		write(fifo_escrita, finally,30);
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
		sprintf(finally, "%d ",seat);
		strcat(success, finally);
		strcat (toFile, success);

		count_seats++;
		split= strtok (NULL, " ");
	}
	fprintf(f, toFile);
	printf("before answer\n");
	sprintf(dir, "ans%d",buf->id);
	printf("%s\n", dir);

	sleep(1);

	if(send_answer(success, dir) == 1){
		printf("Could not send message\n");
		return;
	}
	printf("after answer\n");

}

int send_answer(char* answer, char* dir) {

	if((fifo_escrita=open(dir, O_WRONLY | O_NONBLOCK)) == -1)
		return 1;

	write(fifo_escrita, answer, 30);
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
		pthread_create(&threads[i],NULL,check_buffer,"1");
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

void *check_buffer(void *nr){

	while(1){
		pthread_mutex_lock(&mut);
		while (new_request_flag == TAKEN_REQUEST){
			pthread_cond_wait(&cvar,&mut);
		}
		printf("in check buffer\n");
		new_request_flag = TAKEN_REQUEST;
		open_requests();
		pthread_mutex_unlock(&mut);
	}
	pthread_exit(NULL);

}
