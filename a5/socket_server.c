#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <linux/limits.h>
#include "socket_hdr.h"

#define _PORT 11098
#define NOT_IN_USE -1

char truepath[PATH_MAX] = {'\0'};

int main(int argc, char **argv, char **envp)
{
	int i;
	int maxfd, listenfd, connfd, sockfd;
	int nready, nclients;
	int clients[MAX_CLIENTS];
	char localpath[MAX_CLIENTS][PATH_MAX];
	fd_set rset, allset;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	struct sockaddr_in seraddr;
	char tmp;
	int flag = 0;
	char port[INET6_ADDRSTRLEN] = {'\0'};

	getcwd(truepath, sizeof(truepath));
	
	while((tmp = getopt(argc, argv, "p:")) != -1) {
		switch(tmp) {
		case 'p':
			sprintf(port, "%s", optarg);
			flag = 1;
			break;
		}
	}
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&seraddr, 0, sizeof(seraddr));

	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (flag)
		seraddr.sin_port = htons((int)strtol(port, NULL, 10));
	else
		seraddr.sin_port = htons(_PORT);

	if (bind(listenfd, (struct sockaddr *)&seraddr, sizeof(seraddr)) != 0) {
		printf("cannot bind address\n");
		exit(EXIT_FAILURE);
	}

	if (listen(listenfd, LISTENQ) != 0) {
		printf("cannot listen on port address\n");
		exit(EXIT_FAILURE);
	}

	maxfd = listenfd;
	nclients = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		clients[i] = NOT_IN_USE;
		memset(localpath[i], '\0', sizeof(localpath[i]));
	}

	FD_ZERO(&allset);

	FD_SET(listenfd, &allset);

	while(1) {
		rset = allset;

		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (nready == -1) {
			printf("select returned failure\n");
			continue;
		}

		if (FD_ISSET(listenfd, &rset)) {
			nclients++;
			printf("new client connected\n");
			clilen = sizeof(cliaddr);

			connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

			for (i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i] < 0) {
					clients[i] = connfd;
					memcpy(localpath[i], truepath, strlen(truepath));
					break;
				}
			}

			if (i == MAX_CLIENTS) {
				printf("!!!TOO MANY CLIENTS!!!\n");
				continue;
			}

			FD_SET(connfd, &allset);

			if (connfd > maxfd)
				maxfd = connfd;
			
			if (--nready <= 0)
				continue;			
		}//check listenfd

		for (i = 0; i < MAX_CLIENTS; i++) {
			sockfd = clients[i];

			if (sockfd < 0)
				continue;

			if (FD_ISSET(sockfd, &rset)) {
				char command[PATH_MAX+10] = {'\0'}; 

				printf("server: received communication from client\n");

				memset(command, '\0', sizeof(command));

				if (read(sockfd, command, sizeof(command)) == -1)
					perror("read error\n");
				
				if (strncmp(command, CMD_EXIT, strlen(CMD_EXIT)) == 0) {
					close(sockfd);
					FD_CLR(sockfd, &allset);
					clients[i] = NOT_IN_USE;
					memset(localpath[i], '\0', sizeof(localpath[i]));
					printf("server: client closed connection\n");
					nclients--;
				}//exit
				else if (strncmp(command, CMD_REMOTE_CHDIR, strlen(CMD_REMOTE_CHDIR)) == 0) {
					char path[PATH_MAX] = {'\0'};
					
					chdir(localpath[i]);
					
					memset(localpath[i], '\0', sizeof(localpath[i]));
					
					chdir(&command[strlen(CMD_REMOTE_CHDIR)]);

					getcwd(path, sizeof(path));
					memcpy(localpath[i], path, strlen(path));
					
					if(write(sockfd, path, strlen(path)) == -1)
					   perror("send error\n");

					chdir(truepath);
				}//cd
				else if (strncmp(command, CMD_REMOTE_DIR, strlen(CMD_REMOTE_DIR)) == 0) {
					FILE *fp;
					char path[PATH_MAX] = {'\0'};				
					char resl[PATH_MAX] = {'\0'};
					char end[2] = {EOT_CHAR, '\0'};
					
					memcpy(path, CMD_LS_POPEN, strlen(CMD_LS_POPEN));
					strcat(path, " ");
					strcat(path, localpath[i]);
					if ((fp = popen(path, "r")) == NULL) {
						write(sockfd, RETURN_ERROR, strlen(RETURN_ERROR));
						continue;
					}

				   					while(fgets(resl, sizeof(resl), fp) != NULL) {
						if (write(sockfd, resl, strlen(resl)) == -1)
							perror("send error\n");						
						memset(resl, '\0', sizeof(resl));
					}
					if (write(sockfd, end, strlen(end)) == -1)
						perror("send error\n");
				}//dir
				else if (strncmp(command, CMD_REMOTE_HOME, strlen(CMD_REMOTE_HOME)) == 0) {
					memset(localpath[i], '\0', sizeof(localpath[i]));
					memcpy(localpath[i], getenv("HOME"), strlen(getenv("HOME")));

					if (write(sockfd, localpath[i], strlen(localpath[i])) == -1)
						perror("send error\n");
				}//home
				else if (strncmp(command, CMD_REMOTE_PWD, strlen(CMD_REMOTE_PWD)) == 0) {
					write(sockfd, localpath[i], strlen(localpath[i]));
				}//pwd
				else if (strncmp(command, CMD_GET_FROM_SERVER, strlen(CMD_GET_FROM_SERVER)) == 0) {
					int fd;
					ssize_t n;
					char resl[PATH_MAX] = {'\0'};
					char end[2] = {EOT_CHAR, '\0'};

					chdir(localpath[i]);
					
					if ((fd = open(&command[strlen(CMD_GET_FROM_SERVER)], O_RDONLY)) == -1) {
						if (write(sockfd, RETURN_ERROR, strlen(RETURN_ERROR)) == -1)
							perror("send error\n");
						continue;
					}

					while(1) {
						if ((n = read(fd, resl, sizeof(resl))) == 0)
							break;

						if (write(sockfd, resl, n) == -1)
							perror("send error\n");

						memset(resl, '\0', sizeof(resl));
					}
					if (write(sockfd, end, strlen(end)) == -1)
						perror("send error\n");

					chdir(truepath);
				}//get
				else if (strncmp(command, CMD_PUT_TO_SERVER, strlen(CMD_PUT_TO_SERVER)) == 0) {
					int fd;
					ssize_t n;
					char resl[PATH_MAX] = {'\0'};
					char end = EOT_CHAR;
					
					if (write(sockfd, &end, strlen(&end)) == -1)
						perror("send error\n");

					chdir(localpath[i]);
					
					fd = open(basename(&command[strlen(CMD_PUT_TO_SERVER)]), O_CREAT|O_TRUNC|O_WRONLY, SEND_FILE_PERMISSIONS);

					while(1) {
						if ((n = read(sockfd, resl, sizeof(resl))) == -1)
							perror("read error\n");

						if (resl[n-1] == EOT_CHAR) {							
							write(fd, resl, n-1);
							break;
						}

						write(fd, resl, n);
						
						memset(resl, '\0', sizeof(resl));
					}

					chdir(truepath);
				}//put
			}			
		}//for loop check if there is any ready data
	}//while 1
}
