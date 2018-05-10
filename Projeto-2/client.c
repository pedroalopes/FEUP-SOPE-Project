#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
int fifo_leitura;
int fifo_escrita;
int time_out;

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
	printf("** Running process %d (PGID %d) **\n", getpid(), getpgrp());
	if (argc == 4)
		printf("ARGS: %s | %s | %s\n", argv[1], argv[2], argv[3]);

	request->id=getpid();
	request->num_seats=atoi(argv[2]);
	strcpy(request->seats,argv[3]);

	time_out = atoi(argv[1]);
	create_fifo_ans();
	open_fifo_requests();

	//sleep(10);
	return 0;
}
void create_fifo_ans()
{
	time_t start_t;
	start_t=time(0);
	char dir[20], success[30];

	sprintf(dir, "/tmp/ans%d", getpid());

	if (mkfifo(dir, 0660) != 0) {
		if (errno == EEXIST){
			printf("FIFO WITH %d pid already exists\n",getpid());
			return;
		}
		else {
			printf("CAN'T CREATE FIFO WITH %d pid \n", getpid());
		}
	}

	if((fifo_leitura = open(dir, O_RDONLY| O_NONBLOCK)) == -1) {
		printf("Could not open FIFO\n");
		return;
	}

	while (read(fifo_leitura, success, sizeof(success)) == -1) {
		if ((double)(time(0)-start_t)>=time_out){
			printf("END OF TIME\n");
			return;
		}
		printf("CLIENT: Waiting for SERVER'...\n");
	}

	printf("%s\n", success);

	return;
}
void open_fifo_requests(){
	if ((fifo_escrita= open("tmp/requests",O_WRONLY|O_NONBLOCK))==-1)
		printf("CLIENT: ERROR WHEN FIFO REQUESTS WAS OPENED\n");

	else{
		write (fifo_escrita,request, sizeof(Request));
	}


}
