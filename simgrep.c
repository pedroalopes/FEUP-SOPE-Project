#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <time.h>
#define BUF_LENGTH 256
#define _GNU_SOURCE
char *pattern;
int letter_type=0;
int file_name=0;
int line_number=0;
int total_lines=0;
int word_compare=0;
int directory=0;
clock_t begin=0;
clock_t end;
void clear (void)
{
	while ( getchar() != '\n' );
}

void verify_options(char *filename);
void find_in_directory(char *directory);
int find_complete_word(char *str);
void sigint_handler(int signo)
{
	char input, log_entry[200];

	FILE *logFile=fopen(getenv("LOGFILENAME"),"a");
	sprintf(log_entry,"%ld - %d - received SIGINT signal from user\n",clock(),getpid());

	fprintf(logFile,"%s\n",log_entry);
	fclose(logFile);

	printf("Are you sure that you want to exit?\n");

	input=getchar();
	clear();

	if (input=='Y' || input=='y'){
		FILE *logFile=fopen(getenv("LOGFILENAME"),"a");
		sprintf(log_entry,"%ld - %d - User forced temination! program terminated . . .\n",clock(),getpid());
		sprintf(log_entry,"--------------------------***-------------------------\n");

		fprintf(logFile,"%s\n",log_entry);
		fclose(logFile);
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
	char log_entry[200], log_entry2[200];
	begin=clock();

	FILE *f=fopen("logfile.txt","a");
	sprintf(log_entry,"%ld - %d - COMMAND ",clock(),getpid());
	fprintf(f,"%s\n",log_entry);

	for(int t = 0; t < argc; t++) {
		sprintf(log_entry,"%s ", argv[t]);
		fprintf(f,"%s",log_entry);
	}

	fprintf(f,"\n\n");
	fclose(f);

	putenv("LOGFILENAME=logfile.txt");

	if (sigaction(SIGINT,&action,NULL) < 0)
	{
		fprintf(stderr,"Unable to install SIGINT handler\n");
		exit(1);
	}
	if (argc < 3) {
		printf("Wrong number of arguments! Usage: %s [options] pattern [file/dir]\n", argv[0]);
	}

	char *filename=argv[argc-1];
	pattern= argv[argc-2];
	int i;

	for (i=1;i<argc-1; i++){
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

		if (directory==1){
			find_in_directory(filename);
		}

		else verify_options(filename);
	}

	f=fopen("logfile.txt","a");
	sprintf(log_entry2,"--------------Finished command execution--------------\n");
	fprintf(f,"%s\n",log_entry2);
	sprintf(log_entry2,"--------------------------***-------------------------\n");
	fprintf(f,"%s\n",log_entry2);
	fclose(f);

	exit(0);

}

void verify_options(char *filename){
	FILE *file;
	FILE *logFile;
	char buf[BUF_LENGTH];
	char cout[BUF_LENGTH*40];
	char line_n[10];
	char aux_buf[BUF_LENGTH];
	char log_entry[200];

	int totalLines=0;
	printf("\nFILE:: %s\n", filename);
	if ((file=fopen(filename,"r"))==NULL){
		printf("Non existing file \n");
		exit(0);
	}
	logFile=fopen(getenv("LOGFILENAME"),"a");
	sprintf(log_entry,"%ld - %d - opened file %s\n",clock(),getpid(),filename);

	fprintf(logFile,"%s\n",log_entry);
	fclose(logFile);

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
		return;
	}
	if (file_name==1){
		printf("FILE: %s\n", filename);
		return;
	}
	printf("%s\n",cout);
	cout[0]='\0';
	fclose(file);

	logFile=fopen(getenv("LOGFILENAME"),"a");
	sprintf(log_entry,"%ld - %d - closed file %s\n",clock(),getpid(),filename);

	fprintf(logFile,"%s\n",log_entry);
	fclose(logFile);
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
void find_in_directory(char *directory){

	char log_entry[200];

	int pid, stat;
	pid = fork();

	FILE *logFile=fopen(getenv("LOGFILENAME"),"a");
	sprintf(log_entry,"%ld - %d - created new process for directory %s\n",clock(),getpid(),directory);

	fprintf(logFile,"%s\n",log_entry);
	fclose(logFile);

	if(pid > 0) {
		wait(&stat);
	} else if( pid == 0) {
		struct dirent *de;

		DIR *dr= opendir(directory);
		struct stat stat_buf;
		char file[200];

		if (dr==NULL)
		{
			printf("Could not open current directory\n");
			exit(0);
		}

		while(((de=readdir(dr))!=NULL)){
			sprintf(file, "%s/%s",directory,de->d_name);

			if (lstat(file, &stat_buf)==-1){
				perror("Lstat error \n");
				exit(0);
			}

			if (S_ISREG(stat_buf.st_mode)){
				verify_options(file);
			}

			else if (S_ISDIR(stat_buf.st_mode)){
				if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 ){

					verify_options(file);
					find_in_directory(de->d_name);
				}

			}
			else verify_options(de->d_name);


		}
		closedir(dr);
		return;
	}

}
