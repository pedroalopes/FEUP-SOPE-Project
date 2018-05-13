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
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4

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


	printf("** Running process %d (PGID %d) **\n", getpid(), getpgrp());

	if (argc == 4)
		printf("ARGS: %s | %s | %s\n", argv[1], argv[2], argv[3]);
	else {
		printf("USAGE: ./client [time_out] [nr_de_lugares] [lugares]\n");
		exit(1);
	}
	sprintf(dir, "ans%d", getpid());
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
	char toFile[50],aux[50],success[20];
	FILE *f;
	FILE *fp;
	f = fopen("clog.txt","a+");
	fp =fopen("cbook.txt","a+");
	sprintf(toFile,"%05d ",getpid());
	time_t start_t;
	start_t=time(0);
	int i;
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
	strcpy(aux,answer);
	printf(":::%s\n", aux);
	char * split = strtok (aux," ");
	printf("SPLIT::%s\n", split);
	int number = atoi(split);
	if (number==-1){
		strcat(toFile,"MAX\n");
	}
	if (number==-2){
		strcat(toFile,"NST\n");
	}
	if (number==-3){
		strcat(toFile,"IID\n");
	}
	if(number==-5){
		strcat(toFile,"NAV\n");
	}
	if(number==-6){
		strcat(toFile,"FUL\n");
	}
	if (number <0){
		fprintf(f,toFile);
	}
	else{
		for (i=1 ; i <=number;i++){
			split= strtok (NULL, " ");
			int seat = atoi(split);
			sprintf(toFile,"%04d\n",seat);
			fprintf(fp,toFile);
			fprintf(fp,"\n");
			sprintf(toFile,"%05d ",getpid());



			printf("%s\n",split);
			sprintf(success, "0%d.0%d %04d\n",i,number,seat);
			strcat(toFile,success);
			fprintf(f,toFile);
		}
	}
	fclose(f);
	fclose(fp);


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

	write(fifo_escrita,request, sizeof(Request));
	/*else{
		write (fifo_escrita,request, sizeof(Request));
	}*/
}
