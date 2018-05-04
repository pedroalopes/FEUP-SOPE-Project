#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

 char argv2[20];
char argv3[20];
int main(int argc, char *argv[]) {
  printf("** Running process %d (PGID %d) **\n", getpid(), getpgrp());

  if (argc == 4)
    printf("ARGS: %s | %s | %s\n", argv[1], argv[2], argv[3]);

  strcpy(argv3, argv[3]);
  strcpy(argv2, argv[2]);
  create_fifos();

  sleep(10);

  return 0;
}

void create_fifos()
{
	int fds[2];
	char str[20];
	sprintf(str, "/Projeto-2/ans%d", getpid());
	char *ans=str;
	char *req= "Projeto-2/requests";
	mkfifo(ans,0666);
	fds[0]= open(ans,O_RDONLY);
	fds[1]= open(req, O_WRONLY);
	char client_info[200];
	sprintf(client_info, "%d;%s;%s;", getpid(),argv2, argv3);
	write (fds[1],&client_info, sizeof(client_info));
	perror("Could not write\n");
}
