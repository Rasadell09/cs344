#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <linux/limits.h>
#include "socket_hdr.h"

#define _PORT 11098

void sigHandler(int sig);
void exitFunc(void);

int sockfd;

void sigHandler(int sig)
{
	write(sockfd, CMD_EXIT, strlen(CMD_EXIT));
	exit(EXIT_SUCCESS);
}

void exitFunc(void)
{
	close(sockfd);
}

int main(int argc, char **argv, char **envp)
{
	struct sockaddr_in seraddr;
	int flag_p=0, flag_i=0, flag_h=0;
	char addr[INET6_ADDRSTRLEN] = {'\0'};
	char port[PATH_MAX] = {'\0'};
	char name[PATH_MAX] = {'\0'};
	char tmp;

	while((tmp = getopt(argc, argv, "i:p:H:")) != -1) {
		switch(tmp) {
		case 'i':
			flag_i = 1;
			sprintf(addr, "%s", optarg);
			break;
		case 'p':
			flag_p = 1;
			sprintf(port, "%s", optarg);
			break;
		case 'H':
			flag_h = 1;
			sprintf(name, "%s", optarg);
			break;
		}
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&seraddr, 0, sizeof(seraddr));

	seraddr.sin_family = AF_INET;
	if (flag_p)		
		seraddr.sin_port = htons((int)strtol(port, NULL, 10));
	else {
		seraddr.sin_port = htons(_PORT);
		sprintf(port, "%d", _PORT);
	}
		
	if (flag_i)
		inet_pton(AF_INET, addr, &seraddr.sin_addr.s_addr);
	else 
		seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr)) != 0) {
		printf("could not connect\n");
		exit(EXIT_FAILURE);
	}

	(void) signal(SIGINT, sigHandler);
	(void) atexit(exitFunc);
	
	while(1) {
		char command[PATH_MAX] = {'\0'};

		printf("%s", PROMPT);
		fgets(command, sizeof(command), stdin);
		command[strlen(command)-1] = '\0';
		if (strncmp(command, CMD_EXIT, strlen(CMD_EXIT)) == 0) {
			printf("\n\n\tSayonara...\n\n\n");

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");

			exit(EXIT_SUCCESS);
		}//exit
		else if (strncmp(command, CMD_HELP, strlen(CMD_HELP)) == 0) {
			char *ip = (char *)malloc(sizeof(char));
			
			printf("Available client commands:\n");
			printf("  help      : print this help text\n");
			printf("  exit      : exit the client, causing the client server to also exit\n");
			printf("  cd <dir>  : change to directory <dir> on the server side\n");
			printf("  lcd <dir> : change to directory dir> on the client side\n");
			printf("  dir       : show a `ls -lF` of the server side\n");
			printf("  ldir      : show a `ls -lF` on the client side\n");
			printf("  home      : change current directory of the client server to the user's home directory\n");
			printf("  lhome     : change current directory of the client to the user's home directory\n");
			printf("  pwd       : show the current directory from the server side\n");
			printf("  lpwd      : show the current directory on the client side\n");
			printf("  get <file>: send <file> from server to client (extra credit)\n");
			printf("  put <file>: send <file> from client to server (extra credit)\n");

			ip = inet_ntoa(seraddr.sin_addr);
			printf("\nYou are connected to a server at IPv4 address %s\n", ip);
			printf("You are connected over port %s\n", port);
		}//help
		else if (strncmp(command, CMD_LOCAL_CHDIR, strlen(CMD_LOCAL_CHDIR)) == 0) {
			char path[PATH_MAX] = {'\0'};
			
			if (chdir(&command[strlen(CMD_LOCAL_CHDIR)]) == -1) {
				printf("Change directory failed!\n");
				continue;
			}

			getcwd(path, sizeof(path));

			printf("The working directory for the client is :\n");
			printf("\t%s\n", path);
		}//lcd
		else if (strncmp(command, CMD_REMOTE_CHDIR, strlen(CMD_REMOTE_CHDIR)) == 0) {
			char resl[PATH_MAX] = {'\0'};

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");

			if (read(sockfd, resl, sizeof(resl)) == -1)
				perror("read error\n");

			printf("The working directory for the server is:\n");
			printf("\t%s\n", resl);
		}//cd
		else if (strncmp(command, CMD_LOCAL_DIR, strlen(CMD_LOCAL_DIR)) == 0) {
			FILE *fp;
			char path[PATH_MAX] = {'\0'};
			char dirpath[PATH_MAX] = {'\0'};
			char resl[PATH_MAX] = {'\0'};

			memcpy(path, CMD_LS_POPEN, strlen(CMD_LS_POPEN));
			getcwd(dirpath, sizeof(dirpath));
			strcat(path, " ");
			strcat(path, dirpath);
			if ((fp = popen(path, "r")) == NULL) {
				printf("Cannot list this directory\n");
				continue;
			}

			while(fgets(resl, sizeof(resl), fp) != NULL) {
				printf("%s", resl);
				memset(resl, '\0', sizeof(resl));
			}

			pclose(fp);
		}//ldir
		else if (strncmp(command, CMD_REMOTE_DIR, strlen(CMD_REMOTE_DIR)) == 0) {
			char resl[100] = {'\0'};
			ssize_t n;
			
			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");

			while(1) {
				if ((n = read(sockfd, resl, sizeof(resl))) == -1)
					perror("read error\n");
					
				if (strcmp(resl, RETURN_ERROR) == 0) {
					printf("%s\n", resl);
					break;
				}

					if (resl[n-1] == EOT_CHAR) {
						resl[n-1] = '\0';
						printf("%s", resl);
						break;
					}

				printf("%s", resl);
				
				memset(resl, '\0', sizeof(resl));
			}
		}//dir
		else if (strncmp(command, CMD_LOCAL_PWD, strlen(CMD_LOCAL_PWD)) == 0) {
			char resl[PATH_MAX] = {'\0'};

			getcwd(resl, sizeof(resl));

			printf("The working directory for the client is :\n");
			printf("\t%s\n", resl);
		}//lpwd
		else if (strncmp(command, CMD_REMOTE_PWD, strlen(CMD_REMOTE_PWD)) == 0) {
			char resl[PATH_MAX] = {'\0'};

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");

			if (read(sockfd, resl, sizeof(resl)) == -1)
				perror("send error\n");

			printf("The working directory for the server is :\n");
			printf("\t%s\n", resl);
		}//pwd
		else if (strncmp(command, CMD_LOCAL_HOME, strlen(CMD_LOCAL_HOME)) == 0) {
			chdir(getenv("HOME"));

			printf("The working directory for the client is :\n");
			printf("\t%s\n", getenv("HOME"));
		}//lhome
		else if (strncmp(command, CMD_REMOTE_HOME, strlen(CMD_REMOTE_HOME)) == 0) {
			char resl[PATH_MAX] = {'\0'};

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");

			if (read(sockfd, resl, sizeof(resl)) == -1)
				perror("send error\n");

			printf("The working directory for the server is :\n");
			printf("\t%s\n", resl);
		}//home
		else if (strncmp(command, CMD_PUT_TO_SERVER, strlen(CMD_PUT_TO_SERVER)) == 0) {
			int fd;
			ssize_t n;
			char resl[PATH_MAX] = {'\0'};
			char end[2] = {EOT_CHAR, '\0'};

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");
			
			if (read(sockfd, resl, sizeof(resl)) == -1)
				perror("read error\n");

			memset(resl, '\0', sizeof(resl));
			
			if ((fd = open(&command[strlen(CMD_PUT_TO_SERVER)], O_RDONLY)) == -1) {
				printf("No such file\n");
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
		}//put
		else if (strncmp(command, CMD_GET_FROM_SERVER, strlen(CMD_GET_FROM_SERVER)) == 0) {
			int fd;			
			ssize_t n;
			char resl[PATH_MAX] = {'\0'};

			if (write(sockfd, command, strlen(command)) == -1)
				perror("send error\n");
			
			fd = open(basename(&command[strlen(CMD_GET_FROM_SERVER)]), O_CREAT|O_TRUNC|O_WRONLY, SEND_FILE_PERMISSIONS);

			while(1) {
				if ((n = read(sockfd, resl, sizeof(resl))) == -1)
					perror("read error\n");

				if (strcmp(resl, RETURN_ERROR) == 0) {
					printf("%s\n", resl);
					remove(basename(&command[strlen(CMD_GET_FROM_SERVER)]));
					break;
				}				

					if (resl[n-1] == EOT_CHAR) {
						write(fd, resl, n-1);
						break;
					}

				write(fd, resl, n);
				
				memset(resl, '\0', sizeof(resl));
			}
		}//get
		else {
			printf("Client: Command not recognized.\n");
			printf("        This is my surprised face.  :-()\n");
		}
	}
}
