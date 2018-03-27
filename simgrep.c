#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

void verify_options();
int main(int argc, char *argv[]){

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
			if (letter_type==0){
				if (strstr(buf,pattern)!=NULL){
					strcat(cout, line_n);
					strcat(cout,buf);
					totalLines++;
				}
			}
			else if (strcasestr(buf,pattern)!=NULL){
				strcat(cout, line_n);
				strcat(cout,buf);
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
