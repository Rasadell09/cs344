//Yunfan Li
//liyunf@onid.oregonstate.edu
//CS344-001
//Homework #4
//No References
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <libgen.h>
#include "posixmsg.h"

#define MAX_DATA 1000000
#define MAX_CMD 10000

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

char serMq[PATH_MAX] = {'\0'};
int clients = 0;

void exitFunc(void);
void sigHandler(int sig);
void sigChldHandler(int sig);
void *thread_func(void *arg);

void exitFunc(void)
{
	mq_unlink(serMq);
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

void *thread_func(void *arg)
{
	char localpath[PATH_MAX] = {'\0'};
	message_t *pids = (message_t *)arg;
	char mqRd[PATH_MAX] = {'\0'};
	char mqWr[PATH_MAX] = {'\0'};
	pid_t pid = (int)strtol(pids->command, NULL, 10);
	int s = 0;

	pthread_detach(pthread_self());
	
	clients++;

	//	printf("%d client connected\n", pid);

	CREATE_CLIENT_READER_NAME(mqRd, (int)pid);
	CREATE_CLIENT_WRITER_NAME(mqWr, (int)pid);

	free(pids);
	
	umask(0);
	
	s = pthread_mutex_lock(&mtx);
	if (s != 0)
		printf("Lock failed\n");

	getcwd(localpath, sizeof(localpath));

	s = pthread_mutex_unlock(&mtx);
	if (s != 0)
		printf("Unlock failed\n");

	while (1) {
		mqd_t cwr, crd;
		message_t data;
		
		memset(&data, '\0', sizeof(data));

		if ((cwr = mq_open(mqWr, O_RDONLY)) == -1) {
			printf("Open write queue failed!\n");
			clients--;
			pthread_exit(EXIT_SUCCESS);
		}
		mq_receive(cwr, (char *)&data, sizeof(data), NULL);
		mq_close(cwr);

		if (strcmp(data.command, CMD_EXIT) == 0) {
			clients--;
			//		printf("%d client exit\n", pid);
			pthread_exit(EXIT_SUCCESS);
		}
		else if (strcmp(data.command, CMD_REMOTE_CHDIR) == 0) {
			int ts;
			char local[PATH_MAX] = {'\0'};
			char res[PATH_MAX] = {'\0'};

			ts = pthread_mutex_lock(&mtx);
			if (ts != 0)
				printf("Lock failed\n");			
			
			getcwd(local, sizeof(local));
			
			if (chdir(localpath) == -1) {
				printf("Cannot change directory\n");
				continue;
			}

			if (chdir(data.payload) == -1)
				memcpy(res, "ERROR\n", strlen("ERROR\n"));
			else 
				getcwd(res, sizeof(res));
			
			memset(&data, '\0', sizeof(data));
			memcpy(data.payload, res, strlen(res));
			if (strcmp(res, "ERROR\n") != 0) {
				memset(localpath, '\0', sizeof(localpath));	
				memcpy(localpath, res, strlen(res));
			}
			if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_send(crd, (char *)&data, sizeof(data), 0);
			mq_close(crd);

			if (chdir(local) == -1) {
				printf("Cannot restore directory\n");
				continue;
			}

			ts = pthread_mutex_unlock(&mtx);
			if (ts != 0)
				printf("Unlock failed\n");
		}//cd
		else if (strcmp(data.command, CMD_REMOTE_DIR) == 0) {
			char cmd[PATH_MAX] = {'\0'};
			FILE *fp;
			
			memcpy(cmd, CMD_LS_POPEN, strlen(CMD_LS_POPEN));
			strcat(cmd, " ");
			strcat(cmd, localpath);

			if ((fp = popen(cmd, "r")) == NULL) {
				if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
					printf("Cannot open read queue\n");
					continue;
				}			
				memcpy(data.command, "ERROR\n", strlen("ERROR\n"));
				mq_send(crd, (char *)&data, sizeof(data), 0);
				mq_close(crd);
			}
			while (fgets(data.payload, sizeof(data.payload), fp) != NULL) {
				if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
					printf("Cannot open read queue\n");
					continue;
				}
				data.message_type = MESSAGE_TYPE_DIR;
				mq_send(crd, (char *)&data, sizeof(data), 0);
				mq_close(crd);
				memset(&data, '\0', sizeof(data));
			}
			memset(&data, '\0', sizeof(data));
			data.message_type = MESSAGE_TYPE_DIR_END;
			if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_send(crd, (char *)&data, sizeof(data), 0);
			mq_close(crd);
		}//dir
		else if (strcmp(data.command, CMD_REMOTE_PWD) == 0) {
			memset(&data, '\0', sizeof(data));
			memcpy(data.payload, localpath, strlen(localpath));
			if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_send(crd, (char *)&data, sizeof(data), 0);
			mq_close(crd);
		}//pwd
		else if (strcmp(data.command, CMD_REMOTE_HOME) == 0) {
			memset(&data, '\0', sizeof(data));
			memset(localpath, '\0', sizeof(localpath));
			memcpy(localpath, getenv("HOME"), strlen(getenv("HOME")));
			memcpy(data.payload, localpath, strlen(localpath));
			if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_send(crd, (char *)&data, sizeof(data), 0);
			mq_close(crd);
		}//home
		else if (strcmp(data.command, CMD_PUT) == 0) {
			int fd = 0;
			int ts = 0;
			char local[PATH_MAX] = {'\0'};
			
			ts = pthread_mutex_lock(&mtx);
			getcwd(local, sizeof(local));
			chdir(localpath);
			fd = open(basename(data.payload), SEND_FILE_FLAGS, SEND_FILE_PERMISSIONS);
			memset(&data, '\0', sizeof(data));
			if ((cwr = mq_open(mqWr, O_RDONLY)) == -1) {
				printf("Open write queue failed!\n");
				continue;
			}
			while((mq_receive(cwr, (char *)&data, sizeof(data), NULL)) && (data.message_type != MESSAGE_TYPE_SEND_END)) {
				write(fd, data.payload, data.num_bytes);
				memset(&data, '\0', sizeof(data));
			}
			close(fd);
			chdir(local);
			ts = pthread_mutex_unlock(&mtx);
		}//put
		else if (strcmp(data.command, CMD_GET) == 0) {
			int fd;
			int len;
			int ts;
			char local[PATH_MAX] = {'\0'};
			
			ts = pthread_mutex_lock(&mtx);
			getcwd(local, sizeof(local));
			chdir(localpath);
			fd = open(data.payload, O_RDONLY);
			memset(&data, '\0', sizeof(data));
			while ((len = read(fd, data.payload, sizeof(data.payload))) != 0) {
				data.num_bytes = len;
				if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
					printf("Cannot open read queue\n");
					continue;
				}
				data.message_type = MESSAGE_TYPE_SEND;
				mq_send(crd, (char *)&data, sizeof(data), 0);
				mq_close(crd);
				memset(&data, '\0', sizeof(data));
			}
			close(fd);
			memset(&data, '\0', sizeof(data));
			data.message_type = MESSAGE_TYPE_SEND_END;
			if ((crd = mq_open(mqRd, O_WRONLY)) == -1) {
				printf("Cannot open read queue\n");
				continue;
			}
			mq_send(crd, (char *)&data, sizeof(data), 0);
			mq_close(crd);
			chdir(local);
			ts = pthread_mutex_unlock(&mtx);
		}//get
	}
}

int main(int argc, char **argv, char **envp)
{
	mqd_t sert;
	struct mq_attr attr;

	CREATE_SERVER_QUEUE_NAME(serMq);

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(message_t);
	attr.mq_curmsgs = 0;
	
	umask(0);

	if ((sert = mq_open(serMq, O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
		printf("Create queue failed!\n");
		exit(0);
	}
	mq_close(sert);

	(void) atexit(exitFunc);
	(void) signal(SIGINT, sigHandler);
	(void) signal(SIGCHLD, sigChldHandler);
		
	while(1) {
		int s = 0;
		pthread_t ptid;
		message_t *data;

		if ( (sert = mq_open(serMq, O_RDONLY)) == (mqd_t)-1 ) {
			printf("Cannot open queue!\n");
			exit(0);
		}
		data = (message_t *)malloc(sizeof(message_t));
		mq_receive(sert, (char *)data, sizeof(*data), NULL);
		mq_close(sert);
		
		if(clients >= MAX_CLIENTS)
			continue;

		s = pthread_create(&ptid, NULL, thread_func, data);
		if (s != 0) {
			printf("Create thread failed!\n");
			continue;
		}		
	}

	//return 0;
}
