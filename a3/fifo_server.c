//Yunfan Li
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
#include <signal.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <libgen.h>
#include "fifo.h"

#define MAX_DATA 100000
#define MAX_CMD 10000

char fifoName[PATH_MAX] = {'\0'};
int isClient = 0;

void exitFunc(void);
void sigHandler(int sig);

void exitFunc(void)
{
	if (isClient == 0)
		unlink(fifoName);
}

void sigHandler(int sig)
{
	if (isClient == 0)
		killpg(0, SIGINT);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv, char **envp)
{
	char clientRead[PATH_MAX] = {'\0'};
	char clientWrite[PATH_MAX] = {'\0'};
	int fifoFd = 0;
	int clientRdFd = 0;
	int clientWrFd = 0;	
	pid_t pid = 0;
	pid_t pgid = 0;

	isClient = 0;
	
	CREATE_SERVER_FIFO_NAME(fifoName);

	if (mkfifo(fifoName, FIFO_PERMISSIONS) == -1) {
		printf("Create fifo failed!\n");
		exit(0);
	}
	
    (void) atexit(exitFunc);
    (void) signal(SIGINT, sigHandler);
    (void) signal(SIGCHLD, SIG_IGN);

    pgid = getpgid(0);

    while(1) {
	    int tmpFd = 0;

		fifoFd = open(fifoName, O_RDONLY);
		tmpFd = open(fifoName, O_WRONLY);	    
		read(fifoFd, &pid, sizeof(pid));
		close(fifoFd);
		close(tmpFd);
		
		switch(fork()) {
		case -1:
			printf("failed on fork!\n");
			return(EXIT_FAILURE);
			break;
		case 0:
			{				
				char mycom[MAX_CMD] = {'\0'}, myhead[MAX_CMD] = {'\0'}, mydata[MAX_DATA] = {'\0'};
								
				CREATE_CLIENT_READER_NAME(clientRead, pid);
				CREATE_CLIENT_WRITER_NAME(clientWrite, pid);
				
				isClient = 1;
				
				setpgid(getpid(), pgid);				
				
				while (1) {
					int i = 0;
					int tfd = 0;
					
					memset(mycom, '\0', sizeof(mycom));
					memset(myhead, '\0', sizeof(myhead));
					memset(mydata, '\0', sizeof(mydata));

					clientWrFd = open(clientWrite, O_RDONLY);
					tfd = open(clientWrite, O_WRONLY);
					read(clientWrFd, mycom, sizeof(mycom));
					close(clientWrFd);
					close(tfd);
					
					while ((mycom[i] != 32) && (mycom[i] != '\n') && (mycom[i] != '\0')) {
						myhead[i] = mycom[i];
						i++;
					}
					
					if (strcmp(myhead, CMD_EXIT) == 0) {
						exit(EXIT_SUCCESS);
					}//exit
					else if (strcmp(myhead, CMD_REMOTE_CHDIR) == 0) {
						char path[PATH_MAX] = {'\0'};
						
						i++;
						while (mycom[i] != '\n') {
							path[i-strlen(myhead)-1] = mycom[i];
							i++;
						}
						
						if (-1 == chdir(path)) {
							clientRdFd = open(clientRead, O_WRONLY);
							write(clientRdFd, RETURN_ERROR, strlen(RETURN_ERROR));
							close(clientRdFd);
							continue;
						}
						
						getcwd(path, sizeof(path));
						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, path, strlen(path));
						close(clientRdFd);
					}//cd
					else if (strcmp(myhead, CMD_REMOTE_DIR) == 0) {
						FILE *fp;
						char path[PATH_MAX] = {'\0'};
						char res[MAX_DATA] = {'\0'};

						memcpy(path, CMD_LS_POPEN, strlen(CMD_LS_POPEN));
						if ((fp = popen(path, "r")) == NULL) {
							clientRdFd = open(clientRead, O_WRONLY);
							write(clientRdFd, RETURN_ERROR, strlen(RETURN_ERROR));
							close(clientRdFd);
							continue;
						}
						while (fgets(res, sizeof(res), fp) != NULL)
							strcat(mydata, res);
						pclose(fp);
						
						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, mydata, strlen(mydata));
						close(clientRdFd);
					}//dir
					else if (strcmp(myhead, CMD_REMOTE_PWD) == 0) {
						getcwd(mydata, sizeof(mydata));

						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, mydata, strlen(mydata));
						close(clientRdFd);
					}//pwd
					else if (strcmp(myhead, CMD_REMOTE_HOME) == 0) {
						if (-1 == chdir(getenv("HOME"))) {
							clientRdFd = open(clientRead, O_WRONLY);
							write(clientRdFd, RETURN_ERROR, strlen(RETURN_ERROR));
							close(clientRdFd);
							continue;
						}
						
						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, getenv("HOME"), strlen(getenv("HOME")));
						close(clientRdFd);
					}//home
					else if (strcmp(myhead, CMD_PUT) == 0) {
						int tmp = 1;
						char *filename = (char *)malloc(sizeof(char));
						
						printf("Put file to remote server\n");
						filename = basename(&mycom[4]);
						filename[strlen(filename)-1] = '\0';
						
						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, &tmp, sizeof(tmp));
						close(clientRdFd);

						clientWrFd = open(clientWrite, O_RDONLY);
						tfd = open(clientWrite, O_WRONLY);
						read(clientWrFd, mydata, sizeof(mydata));
						close(clientWrFd);
						close(tfd);
						
						if (strcmp(mydata, RETURN_ERROR) == 0) {
							continue;
						}
						
						if ((tfd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)) == -1) {
							printf("yerp: cannot open file to send: Permission denied\n");
						}
						write(tfd, mydata, strlen(mydata));
						close(tfd);
					}//put
					else if (strcmp(myhead, CMD_GET) == 0) {		
						char path[PATH_MAX] = {'\0'};
						
						printf("Get file from remote server\n");
						strncpy(path, &mycom[4], strlen(mycom)-strlen(myhead)-2);
						if ((tfd = open(path, O_RDONLY)) == -1) {
							printf("yerp: cannot open file to send: No such file or directory\n");
							clientRdFd = open(clientRead, O_WRONLY);
							write(clientRdFd, RETURN_ERROR, strlen(RETURN_ERROR));
							close(clientRdFd);
							continue;
						}
						read(tfd, mydata, sizeof(mydata));
						clientRdFd = open(clientRead, O_WRONLY);
						write(clientRdFd, mydata, strlen(mydata));
						close(clientRdFd);
					}//get
				}//while 1
			}//case 0			
			break;
		default:
			isClient = 0;
 			break;
		}
	}
    //return 0;
}
