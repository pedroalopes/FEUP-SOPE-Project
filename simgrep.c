#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#define BUF_LENGTH 256
#define _GNU_SOURCE
char *filename;
char *pattern;
int letter_type=0;
int file_name=0;
int line_number=0;
int total_lines=0;
int word_compare=0;
int directory=0;
void clear (void)
{
	while ( getchar() != '\n' );
}

void verify_options();
int find_complete_word(char *str);
void sigint_handler(int signo)
{
	char input;
	printf("Are you sure that you want to exit?\n");

	input=getchar();
	clear();

	if (input=='Y' || input=='y'){
		exit(0);
	}
	else if (input=='N' || input =='n')
		return;
	else
		printf("Invalid input\n");
}

int main(int argc, char *argv[]){
	struct sigaction action;
	action.sa_handler = sigint_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGINT,&action,NULL) < 0)
	{
		fprintf(stderr,"Unable to install SIGINT handler\n");
		exit(1);
	}
	if (argc < 3) {
		printf("Wrong number of arguments! Usage: %s [options] pattern [file/dir]\n", argv[0]);
	}

	filename=argv[argc-2];
	pattern= argv[argc-3];
	int i;
	for (i=1;i<argc-2; i++){
		if (strcmp(argv[i],"-i")==0){
			letter_type=1;
			continue;
		}
		if (strcmp(argv[i],"-l")==0){
			file_name=1;
			continue;
		}
		if (strcmp(argv[i],"-n")==0){
			line_number=1;
			continue;
		}
		if (strcmp(argv[i],"-c")==0){
			total_lines=1;
			continue;
		}
		if (strcmp(argv[i],"-w")==0){
			word_compare=1;
			continue;
		}
		if (strcmp(argv[i],"-r")==0){
			directory=1;
			continue;
		}
		verify_options();
	}
}

void verify_options(){
	FILE *file;
	char buf[BUF_LENGTH];
	char cout[BUF_LENGTH*40];
	char line_n[10];
	char aux_buf[BUF_LENGTH];
	int totalLines=0;
	if ((file=fopen(filename,"r"))==NULL){
		printf("Non existing file \n");
		exit(0);
	}
	int i =1;
	while(fgets(buf, BUF_LENGTH, file)){

		if (line_number==1){
			sprintf(line_n, "%d:",i);

		}
		if (word_compare==1){
			strcpy(aux_buf,buf);
			int j=find_complete_word(buf);

			if (j==1){
			strcat(cout, line_n);
			strcat(cout,aux_buf);
			totalLines++;
			}
		}
		else{
			if (letter_type==0){
				if (strstr(buf,pattern)!=NULL){
					strcat(cout, line_n);
					strcat(cout,buf);
					totalLines++;

				}
			}
			if (letter_type==1){
				if ((strcasestr(buf,pattern))!=NULL){
					strcat(cout, line_n);
					strcat(cout,buf);
					totalLines++;
				}
			}
		}
		i++;
	}
	if (total_lines==1)
	{
		printf("Total lines=%d\n",totalLines);
		exit(0);
	}
	if (file_name==1){
		printf("FILE: %s\n", filename);
		exit(0);
	}
	printf("%s\n",cout);
}

int find_complete_word(char *str){
	const char s[2]=" ";
	char *token;
	token=strtok(str,s);
	while(token !=NULL){
		if (strcmp(token,pattern)==0)
			return 1;
		token = strtok(NULL, s);
	}
	return 0;
}
