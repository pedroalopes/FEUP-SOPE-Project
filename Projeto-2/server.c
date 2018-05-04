#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MAX_SEATS 9999

typedef struct {
	int num_room_seats;
	int num_ticket_offices;
	int open_time;
} Info;
Info *info;
pthread_t threads[20];
void * process_request(void *arg);
void create_ticket_offices();
int main(int argc , char * argv[]){

	if (argc != 4){
		printf("Wrong number of arguments! Usage: %s [num_room_seats] [num_ticket_offices] [open_time] \n", argv[0]);
		exit(0);
	}

	info=malloc(sizeof(Info));
	info->num_room_seats=atoi(argv[1]);
	info->num_ticket_offices=atoi(argv[2]);
	info->open_time=atoi(argv[3]);
	//create_ticket_offices();
	char* buffer;
	char * requests = "/Projeto-2/requests";
	mkfifo(requests, 0666);
	free(info);


}
void create_ticket_offices(){
	int i;

	for (i = 0; i< info->num_ticket_offices;i++){
		pthread_create(&threads[i],NULL,process_request, &info->open_time);
		//pthread_join(threads[i],NULL);
		printf("Created thread %d\n",i);
	}
}

void * process_request(void * arg){

	clock_t start = clock();
	double seconds_since_start= (clock()-start)/CLOCKS_PER_SEC;
	while (seconds_since_start < info->open_time){
		seconds_since_start= (clock()-start)/CLOCKS_PER_SEC;

	}

	//pthread_exit(NULL);
}
