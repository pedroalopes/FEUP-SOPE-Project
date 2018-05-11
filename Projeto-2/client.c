#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
int fifo_leitura;
int fifo_escrita;
int time_out;
sem_t *new_client;

typedef struct {
	int id; //numero do pedido
	int num_seats;
	char seats[10];

} Request;

Request *request;

void create_fifo_ans();
void open_fifo_requests();

int main(int argc, char *argv[]) {

	request = malloc(sizeof(Request));

	char dir[30];
	sprintf(dir, "ans%d", getpid());

	printf("** Running process %d (PGID %d) **\n", getpid(), getpgrp());

	if (argc == 4)
		printf("ARGS: %s | %s | %s\n", argv[1], argv[2], argv[3]);
	else {
		printf("USAGE: ./client [time_out] [nr_de_lugares] [lugares]\n");
		exit(1);
	}

	request->id=getpid();
	request->num_seats=atoi(argv[2]);
	strcpy(request->seats,argv[3]);

	time_out = atoi(argv[1]);
	new_client = sem_open("/new_client", O_CREAT);

	open_fifo_requests();
	create_fifo_ans(dir);

	close(fifo_leitura);
	unlink(dir);
	return 0;
}
void create_fifo_ans(char* dir)
{
	time_t start_t;
	start_t=time(0);
	char answer[30] = "NULL";

	if (mkfifo(dir, 0660) != 0) {
		if (errno == EEXIST){
			printf("FIFO WITH %d pid already exists\n",getpid());
		}
		else {
			printf("CAN'T CREATE FIFO WITH %d pid \n", getpid());
		}
	}
	while((fifo_leitura = open(dir, O_RDONLY)) == -1) {
		printf("Could not open FIFO\n");
	}

	printf("before read\n");
	read(fifo_leitura, answer, 30);

	printf("%s\n", answer);

	/*while (read(fifo_leitura, success, sizeof(success)) == -1) {
		if ((double)(time(0)-start_t)>=time_out){
			printf("END OF TIME\n");
			return;
		}
		printf("CLIENT: Waiting for SERVER'...\n");
	}

	printf("%s\n", success);*/

	return;
}
void open_fifo_requests(){
	while ((fifo_escrita= open("requests",O_WRONLY)) == -1){
		sleep(1);
		printf("CLIENT: ERROR WHEN FIFO REQUESTS WAS OPENED\n");
	}

	printf("before post\n");
	sem_post(new_client);
	write(fifo_escrita,request, sizeof(Request));
	/*else{
		write (fifo_escrita,request, sizeof(Request));
	}*/
}
