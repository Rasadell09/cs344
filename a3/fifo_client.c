//Yunfan Li (Voldy)
//liyunf@onid.oregonstate.edu
//CS344-001
//Homework #3
//No References
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/limits.h>
#include <signal.h>
#include "fifo.h"

#define MAX_DATA 1000000
#define MAX_CMD 10000

char fifoRead[PATH_MAX] = {'\0'};
char fifoWrite[PATH_MAX] = {'\0'};

void exitFunc(void);
void sigHandler(int sig);

void exitFunc(void)
{
	unlink(fifoRead);
	unlink(fifoWrite);
}

void sigHandler(int sig)
{
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv, char **envp)
{
	char serverFifo[PATH_MAX] = {'\0'};
	char cmd[MAX_CMD] = {'\0'};
	char res[MAX_DATA] = {'\0'};
	int serverFifoFd = 0;
	int clientRdFd = 0;
	int clientWrFd = 0;
	pid_t pid = getpid();

	CREATE_SERVER_FIFO_NAME(serverFifo);
	CREATE_CLIENT_READER_NAME(fifoRead, pid);
	CREATE_CLIENT_WRITER_NAME(fifoWrite, pid);

	if (mkfifo(fifoRead, FIFO_PERMISSIONS) == -1) {
		printf("Create read fifo failed!\n");
		exit(0);
	}
	
	if (mkfifo(fifoWrite, FIFO_PERMISSIONS) == -1) {
		printf("Create write fifo failed!\n");
		exit(0);
	}

	if ((serverFifoFd = open(serverFifo, O_WRONLY)) == -1) {
		printf("Cannot find server fifo file!\nPlease check whether you opened fifo_server!\n");
		exit(0);
	}
	write(serverFifoFd, &pid, sizeof(pid));
	close(serverFifoFd);

	(void) atexit(exitFunc);
	(void) signal(SIGINT, sigHandler);

	while(1) {
		char head[MAX_CMD] = {'\0'};
		int i = 0;

		memset(cmd, '\0', sizeof(cmd));
		memset(res, '\0', sizeof(res));
		
		printf("%s", CLIENT_PROMPT);
		fgets(cmd, sizeof(cmd), stdin);
		
	   	while ((cmd[i] != 32) && (cmd[i] != '\n') && (cmd[i] != '\0')) {
			head[i] = cmd[i];
	   		i++;
	   	}
	
		if (strcmp(head, CMD_EXIT) == 0) {
			printf("client exiting\n");
	  		clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);
			
			exit(EXIT_SUCCESS);
		}//exit
		else if (strcmp(head, CMD_REMOTE_CHDIR) == 0) {
			int tmpFd = 0;
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);

			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoWrite, O_WRONLY);
			read(clientRdFd, res, sizeof(res));
			close(clientRdFd);
			close(tmpFd);
			
			printf("The working directory for the server is:\n");
			printf("        %s\n", res);
		}//cd
		else if (strcmp(head, CMD_LOCAL_CHDIR) == 0) {
			char path[PATH_MAX] = {'\0'};

			i++;
			while (cmd[i] != '\n') {
				path[i-strlen(head)-1] = cmd[i];
				i++;
			}
			
			if (-1 == chdir(path)) {
				printf("Change directory failed!\n");
				continue;
			}
			
			getcwd(path, sizeof(path));
			printf("The working directory for the client is:\n");
			printf("        %s\n", path);
		}//lcd
		else if (strcmp(head, CMD_REMOTE_DIR) == 0) {
			int tmpFd = 0;
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);

			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoRead, O_WRONLY);
			read(clientRdFd, res, sizeof(res));
			close(clientRdFd);
			close(tmpFd);
			
			printf("%s", res);
		}//dir
		else if (strcmp(head, CMD_LOCAL_DIR) == 0) {
			FILE *fp;
			char path[PATH_MAX] = {'\0'};
			char dirpath[PATH_MAX] = {'\0'};
			char resl[MAX_DATA] = {'\0'};

			memcpy(path, CMD_LS_POPEN, strlen(CMD_LS_POPEN));
			getcwd(dirpath, sizeof(dirpath));
			strcat(path, " ");
			strcat(path, dirpath);
			if ((fp = popen(path, "r")) == NULL) {
				printf("Cannot list this directory!\n");
				continue;
			}
			while(fgets(resl, sizeof(resl), fp) != NULL)
				strcat(res, resl);
			pclose(fp);
			
			printf("%s", res);
		}//ldir
		else if (strcmp(head, CMD_REMOTE_PWD) == 0) {
			int tmpFd = 0;
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);
		   
			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoRead, O_WRONLY);
			read(clientRdFd, res, sizeof(res));
			close(clientRdFd);
			close(tmpFd);
			
			printf("The working directory for the server is:\n");
			printf("        %s\n", res);
		}//pwd
		else if (strcmp(head, CMD_LOCAL_PWD) == 0) {
			getcwd(res, sizeof(res));
			
			printf("The working directory for the client is:\n");
			printf("        %s\n", res);
		}//lpwd
		else if (strcmp(head, CMD_REMOTE_HOME) == 0) {
			int tmpFd = 0;
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);

			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoRead, O_WRONLY);
			read(clientRdFd, res, sizeof(res));
			close(clientRdFd);
			close(tmpFd);
			
			printf("The working directory for the server is:\n");
			printf("        %s\n", res);
		}//home
		else if (strcmp(head, CMD_LOCAL_HOME) == 0) {
			if (-1 == chdir(getenv("HOME"))) {
				printf("failed to change directory!\n");
				continue;
			}
			
			printf("The working directory for the client is:\n");
			printf("        %s\n", getenv("HOME"));
		}//lhome
		else if (strcmp(head, CMD_HELP) == 0) {
			printf("Available client commands:\n");
			printf("         help      : print this help text\n");
			printf("         exit      : exit the client, causing the client server to also exit\n");
			printf("         cd <dir>  : change to directory <dir> on the server side\n");
			printf("         lcd <dir> : change to directory dir> on the client side\n");
			printf("         dir       : show a `ls -lF` of the server side\n");
			printf("         ldir      : show a `ls -lF` on the client side\n");
			printf("         home      : change current directory of the client server to the user's home directory\n");
			printf("         lhome     : change current directory of the client to the user's home directory\n");
			printf("         pwd       : show the current directory from the server side\n");
			printf("         lpwd      : show the current directory on the client side\n");
			printf("         get <file>: send <file> from server to client (extra credit)\n");
			printf("         put <file>: send <file> from client to server (extra credit)\n");
		}//help
		else if (strcmp(head, CMD_PUT) == 0) {
			int tmpFd = 0;
			int tmp = 0;
			char data[MAX_DATA] = {'\0'};
			char filename[PATH_MAX] = {'\0'};
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);

			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoRead, O_WRONLY);
			read(clientRdFd, &tmp, sizeof(tmp));
			close(clientRdFd);
			close(tmpFd);

			i++;
			while (cmd[i] != '\n') {
				filename[i-strlen(head)-1] = cmd[i];
				i++;
			}
			
			if ((tmpFd = open(filename, O_RDONLY)) == -1) {
				printf("yerp 1: Bad file descriptor\n");
				clientWrFd = open(fifoWrite, O_WRONLY);
				write(clientWrFd, RETURN_ERROR, strlen(RETURN_ERROR));
				close(clientWrFd);
				continue;
			}
			read(tmpFd, data, sizeof(data));
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, data, strlen(data));
			close(clientWrFd);
		}//put
		else if (strcmp(head, CMD_GET) == 0) {		
			int tmpFd = 0;
			char *filename = (char *)malloc(sizeof(char));
			
			clientWrFd = open(fifoWrite, O_WRONLY);
			write(clientWrFd, cmd, strlen(cmd));
			close(clientWrFd);

			clientRdFd = open(fifoRead, O_RDONLY);
			tmpFd = open(fifoRead, O_WRONLY);
			read(clientRdFd, res, sizeof(res));
			close(clientRdFd);
			close(tmpFd);
			
			if (strcmp(res, RETURN_ERROR) == 0) 				
				continue;			

			filename = basename(&cmd[4]);
			filename[strlen(filename)-1] = '\0';

			tmpFd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
			write(tmpFd, res, strlen(res));
			close(tmpFd);
		}//get
		else {
			printf("Client: Command not recognized.\n");
			printf("  Boooo.  Hisss.  Rotten tomatoes.\n");
		}
	}//while 1
	//return 0;
}
