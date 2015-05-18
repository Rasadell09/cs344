//Yunfan Li(Voldy)
//liyunf@onid.oregonstate.edu
//CS344-001
//Homework #4
//No References
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <linux/limits.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <libgen.h>
#include "posixmsg.h"

#define MAX_DATA 1000000
#define MAX_CMD 10000
#define MAX_LEN sizeof(message_t)+1

char mqRd[PATH_MAX] = {'\0'};
char mqWr[PATH_MAX] = {'\0'};

void exitFunc(void);
void sigHandler(int sig);
void sigChldHandler(int sig);

void exitFunc(void)
{
	mq_unlink(mqRd);
  	mq_unlink(mqWr);
}

void sigHandler(int sig)
{
	printf("SIGINT handler.\n");
	exit(EXIT_SUCCESS);
}

void sigChldHandler(int sig)
{
  	printf("SIGCHLD handler.\n");
}

int main(int argc, char **argv, char **envp)
{
	char serMq[PATH_MAX] = {'\0'};
	pid_t pid = getpid();
	struct mq_attr attr;
	mqd_t smq, crd, cwr;
	message_t pids;
	int ij;

	CREATE_SERVER_QUEUE_NAME(serMq);
	CREATE_CLIENT_READER_NAME(mqRd, pid);
	CREATE_CLIENT_WRITER_NAME(mqWr, pid);

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(message_t);
	attr.mq_curmsgs = 0;
	
	umask(0);

	if ((crd = mq_open(mqRd, O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
		printf("Create read queue failed!\n");
		exit(EXIT_SUCCESS);
	}
	mq_close(crd);

	if ((cwr = mq_open(mqWr, O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
		printf("Create write queue failed!\n");
		exit(EXIT_SUCCESS);
	}
	mq_close(cwr);

	if ((smq = mq_open(serMq, O_WRONLY)) == -1) {
		printf("Cannot find server queue!\n");
		exit(EXIT_SUCCESS);
	}
	memset(&pids, '\0', sizeof(pids));
	sprintf(pids.command, "%d", (int)pid);
	ij = mq_send(smq, (char *) &pids, sizeof(pids), 1);
	if (ij == -1) {
		printf("Send pid failed!\n");
		exit(EXIT_SUCCESS);
	}
	mq_close(smq);
	
	(void) atexit(exitFunc);
	(void) signal(SIGINT, sigHandler);
	(void) signal(SIGCHLD, sigChldHandler);

	while(1) {
		char head[PATH_MAX] = {'\0'};
		char pay[PATH_MAX] = {'\0'};
		char result[PATH_MAX] = {'\0'};
		char command[PATH_MAX] = {'\0'};
		message_t data;
		int i = 0;
		
		memset(command, '\0', sizeof(command));
		memset(result, '\0', sizeof(result));
		memset(head, '\0', sizeof(head));
		memset(&data, '\0', sizeof(data));
		
		printf("%s", PROMPT);
		fgets(command, sizeof(command), stdin);

		while ((command[i] != 32) && (command[i] != '\n') && (command[i] != '\0')) {
			head[i] = command[i];
			i++;
		}
		i++;
		while ((command[i] != '\n') && (command[i] != '\0')) {
			pay[i-strlen(head)-1] = command[i];
			i++;
		}
		memcpy(data.command, head, strlen(head));
		memcpy(data.payload, pay, strlen(pay));

		if (strcmp(head, CMD_EXIT) == 0) {
			printf("\n\n        Sayonara...\n\n\n");
			
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				exit(EXIT_SUCCESS);
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			//			exit(EXIT_SUCCESS);
			sigHandler(SIGINT);
		}//exit
		else if (strcmp(head, CMD_REMOTE_CHDIR) == 0) {
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			memset(&data, '\0', sizeof(data));
			
			if ((crd = mq_open(mqRd, O_RDONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_receive(crd, (char *)&data, sizeof(data), NULL);
			mq_close(crd);
			
			printf("The working directory for the server is:\n");
			printf("        %s\n", data.payload);
		}//cd
		else if (strcmp(head, CMD_LOCAL_CHDIR) == 0) {
			char path[PATH_MAX] = {'\0'};
			
			if (chdir(data.payload) == -1) {
				printf("Change directory failed!\n");
				continue;
			}

			getcwd(path, sizeof(path));
			printf("The working directory for the client is :\n");
			printf("        %s\n", path);
		}//lcd
		else if (strcmp(head, CMD_REMOTE_DIR) == 0) {
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			memset(&data, '\0', sizeof(data));
			if ((crd = mq_open(mqRd, O_RDONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			while ((mq_receive(crd, (char *)&data, sizeof(data), NULL)) && (data.message_type != MESSAGE_TYPE_DIR_END)) {
				printf("%s", data.payload);
				memset(&data, '\0', sizeof(data));
			}
			mq_close(crd);
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
			
			while (fgets(resl, sizeof(resl), fp) != NULL)
				printf("%s", resl);

			pclose(fp);
		}//ldir
		else if (strcmp(head, CMD_REMOTE_PWD) == 0) {
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			memset(&data, '\0', sizeof(data));
			if ((crd = mq_open(mqRd, O_RDONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_receive(crd, (char *)&data, sizeof(data), NULL);
			mq_close(crd);
			
			printf("The working directory for the server is:\n");
			printf("        %s\n", data.payload);
		}//pwd
		else if (strcmp(head, CMD_LOCAL_PWD) == 0) {
			getcwd(result, sizeof(result));

			printf("The working directory for the client is:\n");
			printf("        %s\n", result);
		}//lpwd
		else if (strcmp(head, CMD_REMOTE_HOME) == 0) {
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);

			memset(&data, '\0', sizeof(data));
			
			if ((crd = mq_open(mqRd, O_RDONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_receive(crd, (char *)&data, sizeof(data), NULL);
			mq_close(crd);

			printf("The working directory for the server is:\n");
			printf("        %s\n", data.payload);
		}//home
		else if (strcmp(head, CMD_LOCAL_HOME) == 0) {
			if (chdir(getenv("HOME")) == -1) {
				printf("failed to change direcotry!\n");
				continue;
			}
			
			printf("The working directory for the client is:\n");
			printf("        %s\n", getenv("HOME"));
		}//lhome
		else if (strcmp(head, CMD_PUT) == 0) {
			int fd;
			int len;
			
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			fd = open(data.payload, O_RDONLY);
			memset(&data, '\0', sizeof(data));
			while ((len = read(fd, data.payload, sizeof(data.payload))) != 0) {
				data.num_bytes = len;
				if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
					printf("Cannot open write queue\n");
					continue;
				}
				data.message_type = MESSAGE_TYPE_SEND;
				mq_send(cwr, (char *)&data, sizeof(data), 0);
				mq_close(cwr);
				memset(&data, '\0', sizeof(data));
 			}
			close(fd);
			memset(&data, '\0', sizeof(data));
			data.message_type = MESSAGE_TYPE_SEND_END;
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
		}//put
		else if (strcmp(head, CMD_GET) == 0) {
			int fd = 0;
			
			if ((cwr = mq_open(mqWr, O_WRONLY)) == -1) {
				printf("Cannot open write queue\n");
				continue;
			}
			mq_send(cwr, (char *)&data, sizeof(data), 0);
			mq_close(cwr);
			
			fd = open(basename(data.payload), SEND_FILE_FLAGS, SEND_FILE_PERMISSIONS);
			memset(&data, '\0', sizeof(data));
			if ((crd = mq_open(mqRd, O_RDONLY)) == -1) {
				printf("Open read queue failed\n");
				continue;
			}
			while ((mq_receive(crd, (char *)&data, sizeof(data), NULL)) && (data.message_type != MESSAGE_TYPE_SEND_END)) {
				write(fd, data.payload, data.num_bytes);
				memset(&data, '\0', sizeof(data));
			}
			close(fd);
		}//get
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
		else {
			printf("Client: Command not recognized.\n");
			printf("        This is my surprised face.  :-()\n");
		}
	}

	//return 0;
}
