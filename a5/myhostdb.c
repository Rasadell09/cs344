#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <linux/limits.h>
#include <netdb.h>
#include <time.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <errno.h>
#include "hostdb.h"

#define SHARED_BETWEEN_PROCESSES 1
#define SHARED_WITHIN_PROCESSES 0
#define NUM_RESOURCES 1

void exitFunc(void);
void sigHandler(int sig);
void getIpByName(char *name, char *ip4, char *ip6, char *o4, char *o6);

int dblock = 0;
int rowlock = -1;
int sig = 1;
sem_t *sem;
host_row_t *shared_base;

void sigHandler(int s)
{
	sig = 0;
	exit(EXIT_SUCCESS);
	//	printf("sig 0\n");
}

void exitFunc(void)
{
	if (dblock) {
		sem_post(sem);
		dblock = 0;
		//printf("db 0\n");
	}

	if (rowlock != -1) {
		int c = 0;
		for (; c < MAX_ROWS; c++) {
			if (sem_trywait(&shared_base[c].host_lock) == 0) {
				sem_post(&shared_base[c].host_lock);
			}
			else {
				sem_post(&shared_base[c].host_lock);
				rowlock = -1;
				//		printf("row 0\n");
				break;
			}
		}
		
	}
}

void getIpByName(char *name, char *ip4, char *ip6, char *o4, char *o6)
{
	struct addrinfo hints, *addr_list, *p;
	int status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(name, NULL, &hints, &addr_list);
	if (status != 0) {

		strncpy(ip4, "error", strlen("error"));
	}
	else {
		for (p = addr_list; p != NULL; p = p->ai_next) {
			void *addr;
			char ipstr[INET6_ADDRSTRLEN] = {'\0'};
			
			memset(ipstr, '\0', sizeof(ipstr));

			//			if ((p->ai_family == AF_INET) && (strlen(o4) != 0)) {
			//	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			//	addr = &(ipv4->sin_addr);
			//	inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
			//	if (strncmp(ipstr, o4, strlen(o4)) == 0)
			//		continue;
			//}

			//if ((p->ai_family == AF_INET6) && (strlen(o6) != 0)) {
			//	struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			//	addr = &(ipv6->sin6_addr);
			//	inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
			//	if (strncmp(ipstr, o6, strlen(o6)) == 0)
			//		continue;
			//}
			
			if ((p->ai_family == AF_INET) && (strlen(ip4) == 0)) {			
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				addr = &(ipv4->sin_addr);
				inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
				strncpy(ip4, ipstr, strlen(ipstr));
			}
			else if ((p->ai_family == AF_INET6) && (strlen(ip6) == 0)){
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				addr = &(ipv6->sin6_addr);
				inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
				strncpy(ip6, ipstr, strlen(ipstr));
			}
		}
		freeaddrinfo(addr_list);
	}
}

int main(int argc, char **argv, char **envp)
{
	int sfd = 0;
	int shared_seq_size = sizeof(sem_t) + (MAX_ROWS * sizeof(host_row_t));
	char shm_name[NAME_SIZE] = {'\0'};
	void *shared_void;
	
	SHARED_MEM_NAME(shm_name);

	(void) signal(SIGINT, sigHandler);
	(void) atexit(exitFunc);
	
	sfd = shm_open(shm_name, O_CREAT|O_EXCL|O_RDWR, SHARED_MEM_PERMISSIONS);
	
	if (sfd < 0) {
		sfd = shm_open(shm_name, O_RDWR, SHARED_MEM_PERMISSIONS);
		if (ftruncate(sfd, shared_seq_size) != 0) {
			printf("Failed resize sfd!\n");
			exit(EXIT_FAILURE);
		}
		shared_void = mmap(NULL, shared_seq_size, PROT_READ|PROT_WRITE, MAP_SHARED, sfd, 0);
		if (shared_void == NULL) {
			printf("Failed map memory!\n");
			exit(EXIT_FAILURE);
		}
		sem = (sem_t *)shared_void;
		
		shared_base = (host_row_t *)(shared_void + sizeof(sem_t));
	}
	else {
		if (ftruncate(sfd, shared_seq_size) != 0) {
			printf("Failed resize sfd!\n");
			exit(EXIT_FAILURE);
		}
		shared_void = mmap(NULL, shared_seq_size, PROT_READ|PROT_WRITE, MAP_SHARED, sfd, 0);
		if (shared_void == NULL) {
			printf("Failed map memory!\n");
			exit(EXIT_FAILURE);
		}
		sem = (sem_t *)shared_void;
		sem_init(sem, SHARED_BETWEEN_PROCESSES, NUM_RESOURCES);
		
		shared_base = (host_row_t *)(shared_void + sizeof(sem_t));
		
		{
			int i = 0;
			for (; i < MAX_ROWS; i++) {
				sem_init(&shared_base[i].host_lock, SHARED_BETWEEN_PROCESSES, NUM_RESOURCES);
				shared_base[i].in_use = 0;
				memset(shared_base[i].host_name, '\0', sizeof(shared_base[i].host_name));
				memset(shared_base[i].host_address_ipv4, '\0', sizeof(shared_base[i].host_address_ipv4));
				memset(shared_base[i].host_address_ipv6, '\0', sizeof(shared_base[i].host_address_ipv6));
				shared_base[i].time_when_fetched = time(NULL);
			}
		}
	}
	
	while(1) {
		char cmd[PATH_MAX] = {'\0'};
		char name[PATH_MAX] = {'\0'};
		
		memset(cmd, '\0', sizeof(cmd));
		memset(name, '\0', sizeof(name));

		if (dblock)
			printf("DBlock ");
		if (rowlock != -1)
			printf("Rlock 1 ");
		printf("%s", PROMPT);
		fgets(cmd, sizeof(cmd), stdin);

		if (strncmp(cmd, CMD_HELP, strlen(CMD_HELP)) == 0) {
			printf("Available commands:\n");
			printf("help               : print this help text\n");
			printf("exit               : exit the application\n");
			printf("select [<host>]    : display hostname and address(es)\n");
			printf("insert <host>      : insert an address given a name\n");
			printf("update <host>      : refresh <name> in database\n");
			printf("delete [<host>]    : remove one or all hosts from the database\n");
			printf("count              : count the number of rows in db - EXTRA CREDIT\n");
			printf("drop_database      : drop the database, unlink shared memory\n");
			printf("save <file>        : save all names to a file\n");
			printf("load <file>        : load all names from a file and insert addresses\n");
			printf("lock_db            : lock db\n");
			printf("unlock_db          : unlock db\n");
			printf("lock_row <host>    : lock row <name>\n");
			printf("unlock_row         : unlock the locked row\n");
			printf("locks              : show all locks - EXTRA CREDIT\n");
		}//help
		else if (strncmp(cmd, CMD_EXIT, strlen(CMD_EXIT)) == 0) {
			exit(EXIT_SUCCESS);
		}//exit
		else if (strncmp(cmd, CMD_INSERT, strlen(CMD_INSERT)) == 0) {
			int c = 0;
			int flag = 0;
			
			{
				int i = strlen(CMD_INSERT);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_INSERT)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}
			
			// check if database is deleted
			if (shared_void == NULL) {
				printf("Not connected to database!\n");
				continue;
			}

			//check if have a self db lock
			if (dblock == 1) {
				printf("Cannot insert while holding a database lock.\n");
				continue;
			}

			//check if have a self row lock
			if (rowlock != -1) {
				printf("Cannot insert while holding a row lock.\n");
				continue;
			}
			
			//Don't have self db lock, check if there is other process lock db
			//if it is, block(wait) it
			//if not, trywait->post it
			if (sem_trywait(sem) != 0) {
				printf("Database currently locked, waiting...\n");
				sem_wait(sem);
			} 
			sem_post(sem);

			//Don't have self row lock, check if others lock other row
			//if find a row that has been locked by others, block it
			//and then post it
			for (c = 0; c < MAX_ROWS; c++) {
				if (sem_trywait(&shared_base[c].host_lock) != 0) {
					sem_wait(&shared_base[c].host_lock);
					break;
				}
				else
					sem_post(&shared_base[c].host_lock);
			}
			sem_post(&shared_base[c].host_lock);
			
			for (c = 0; c < MAX_ROWS; c++) {
				if (strcmp(shared_base[c].host_name, name) == 0) {
					flag = 1;
					break;
				}
			}

			if (flag) {				
				struct tm *t = (struct tm *)malloc(sizeof(struct tm));
				char ft[PATH_MAX] = {'\0'};
				
				t = localtime(&shared_base[c].time_when_fetched);
				strftime(ft, sizeof(ft), "%F %T %Z", t);
				
				printf("Name: %s\n", shared_base[c].host_name);
				printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
				printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
				printf("\tTime fetched: %s\n", ft);
								
				continue;
			}
			
			for (c = 0; c < MAX_ROWS; c++) {					
				if ((shared_base[c].in_use == 0) && (sem_trywait(&shared_base[c].host_lock) == 0))
					break;
			}

			{
				char ip[2][INET6_ADDRSTRLEN];

				memset(ip[0], '\0', sizeof(ip[0]));
				memset(ip[1], '\0', sizeof(ip[1]));

				getIpByName(name, ip[0], ip[1], "\0", "\0");

				if (strncmp(ip[0], "error", strlen("error")) == 0) {
					printf("  *** name <%s> not found ***\n", name);
					continue;
				}
				strncpy(shared_base[c].host_name, name, strlen(name));
				strncpy(shared_base[c].host_address_ipv4, ip[0], strlen(ip[0]));
				strncpy(shared_base[c].host_address_ipv6, ip[1], strlen(ip[1]));
				shared_base[c].time_when_fetched = time(NULL);
				shared_base[c].in_use = 1;
			}

			sem_post(&shared_base[c].host_lock);

			{
				struct tm *t = (struct tm *)malloc(sizeof(struct tm));
				char ft[PATH_MAX] = {'\0'};
				
				t = localtime(&shared_base[c].time_when_fetched);
				strftime(ft, sizeof(ft), "%F %T %Z", t);
				
				printf("Name: %s\n", shared_base[c].host_name);
				printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
				printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
				printf("\tTime fetched: %s\n", ft);
			}

			//			if (sem_post(sem) == -1)
			//	printf("%d\n", errno);

		}//insert
		else if (strncmp(cmd, CMD_UPDATE, strlen(CMD_UPDATE)) == 0) {
			int c = 0;
			
			{
				int i = strlen(CMD_UPDATE);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_UPDATE)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}
			
			if (shared_void == NULL) {
				printf("Cannot connect to database!\n");
				continue;
			}			

			if (rowlock != -1) {
				printf("Cannot lock row while holding a row lock\n");
				sem_post(&shared_base[rowlock].host_lock);
				rowlock = -1;
			}
			else {
				for (c = 0; c < MAX_ROWS; c++) {
					if (sem_trywait(&shared_base[c].host_lock) != 0) {
						sem_wait(&shared_base[c].host_lock);
						break;
					}
					else
						sem_post(&shared_base[c].host_lock);
				}
				sem_post(&shared_base[c].host_lock);
			}

			if (dblock == 0) {
				if (sem_trywait(sem) != 0) {
					printf("Database currently locked, waiting...\n");
					sem_wait(sem);
				}
				sem_post(sem);
			}
			
			for (c = 0; c < MAX_ROWS; c++) {
				if (strcmp(shared_base[c].host_name, name) == 0)							
					break;			
			}
			
			if (c == MAX_ROWS) {
				printf("  *** name <%s> not found ***\n", name);
				continue;
			}
			
			sem_trywait(&shared_base[c].host_lock);
			
			{
				char ip[2][INET6_ADDRSTRLEN];

				memset(ip[0], '\0', sizeof(ip[0]));
				memset(ip[1], '\0', sizeof(ip[1]));
				memset(shared_base[c].host_address_ipv4, '\0', sizeof(shared_base[c].host_address_ipv4));
				memset(shared_base[c].host_address_ipv6, '\0', sizeof(shared_base[c].host_address_ipv6));
				
				getIpByName(name, ip[0], ip[1], shared_base[c].host_address_ipv4, shared_base[c].host_address_ipv6);

				strncpy(shared_base[c].host_name, name, strlen(name));
				strncpy(shared_base[c].host_address_ipv4, ip[0], strlen(ip[0]));
				strncpy(shared_base[c].host_address_ipv6, ip[1], strlen(ip[1]));
				shared_base[c].time_when_fetched = time(NULL);
				shared_base[c].in_use = 1;
			}
			
			{
				struct tm *t = (struct tm *)malloc(sizeof(struct tm));
				char ft[PATH_MAX] = {'\0'};
				
				t = localtime(&shared_base[c].time_when_fetched);
				strftime(ft, sizeof(ft), "%F %T %Z", t);
				
				printf("Name: %s\n", shared_base[c].host_name);
				printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
				printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
				printf("\tTime fetched: %s\n", ft);
			}

			//	if (dblock == 0) {
			//	if (sem_post(sem) != 0) {
			//		printf("Failed unlock database\n");
			//		}
			//	}

			if (sem_post(&shared_base[c].host_lock) != 0) {
				printf("Failed unlock row\n");
			}
		}//update
		else if (strncmp(cmd, CMD_SELECT, strlen(CMD_SELECT)) == 0) {
			int c = 0;
			int cnt = 0;
			
			{
				int i = strlen(CMD_SELECT);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_SELECT)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (; j< strlen(name); j++) 
							name[j] = name[j+1];						
					}
				}
			}
			
			if (shared_void == NULL) {
				printf("Cannot connect to database!\n");
				continue;
			}			

			if (rowlock != -1) {
				printf("Cannot lock row while holding a row lock\n");
				sem_post(&shared_base[rowlock].host_lock);
				rowlock = -1;
			}
			else {
				for (c = 0; c < MAX_ROWS; c++) {
					if (sem_trywait(&shared_base[c].host_lock) != 0) {
						sem_wait(&shared_base[c].host_lock);
						break;
					}
					else
						sem_post(&shared_base[c].host_lock);
				}
				sem_post(&shared_base[c].host_lock);
			}
			
			if (dblock == 0) {
				if (sem_trywait(sem) != 0) {
					printf("Database currently locked, waiting...\n");
					sem_wait(sem);
				}
				sem_post(sem);
			}

			if (strlen(name) == 0) {
				for (c = 0; c < MAX_ROWS; c++) {
					if (shared_base[c].in_use == 1) {
						struct tm *t = (struct tm *)malloc(sizeof(struct tm));
						char ft[PATH_MAX] = {'\0'};
				
						t = localtime(&shared_base[c].time_when_fetched);
						strftime(ft, sizeof(ft), "%F %T %Z", t);
				
						printf("Name: %s\n", shared_base[c].host_name);
						printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
						printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
						printf("\tTime fetched: %s\n", ft);
						cnt++;
					}
				}
				printf("Rows selected: %d\n", cnt);
			}
			else {				
				struct tm *t = (struct tm *)malloc(sizeof(struct tm));
				char ft[PATH_MAX] = {'\0'};
				
				for(c = 0; c < MAX_ROWS; c++)
					if (strcmp(shared_base[c].host_name, name) == 0)
						break;

				if (c == MAX_ROWS) {
					printf("hostname %s not in database\n", name);
					continue;
				}	
				
				for (c = 0; c < MAX_ROWS; c++) {
					if (strcmp(shared_base[c].host_name, name) == 0) {
						break;
					}
				}				
				
				t = localtime(&shared_base[c].time_when_fetched);
				strftime(ft, sizeof(ft), "%F %T %Z", t);
				
				printf("Name: %s\n", shared_base[c].host_name);
				printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
				printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
				printf("\tTime fetched: %s\n", ft);
				printf("Rows selected: 1\n");
			
			}

			//	if (dblock == 0) {
			//		if (sem_post(sem) != 0) {
			//			printf("Failed unlock database\n");
			//		}
			//	}

			if (sem_post(&shared_base[rowlock].host_lock) != 0) {
				printf("Failed unlock row\n");
			}
		}//select
		else if (strncmp(cmd, CMD_DELETE, strlen(CMD_DELETE)) == 0) {
			int c = 0;

			{
				int i = strlen(CMD_DELETE);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_DELETE)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}

			if (shared_void == NULL) {
				printf("Cannot connect to database!\n");
				continue;
			}			

			if (rowlock != -1) {
				printf("Cannot lock row while holding a row lock\n");
				sem_post(&shared_base[rowlock].host_lock);
				rowlock = -1;
			}
			else {
				for (c = 0; c < MAX_ROWS; c++) {
					if (sem_trywait(&shared_base[c].host_lock) != 0) {
						sem_wait(&shared_base[c].host_lock);
						break;
					}
					else
						sem_post(&shared_base[c].host_lock);
				}
				sem_post(&shared_base[c].host_lock);
			}

			if (dblock == 0) {
				if (sem_trywait(sem) != 0) {
					printf("Database currently locked, waiting...\n");
					sem_wait(sem);
				}
				sem_post(sem);
			}
			
			if (strlen(name) == 0) {
				for (c = 0; c < MAX_ROWS; c++) {
					if (shared_base[c].in_use == 1) {
						sem_trywait(&shared_base[c].host_lock);
						shared_base[c].in_use = 0;
						memset(shared_base[c].host_name, '\0', sizeof(shared_base[c].host_name));
						memset(shared_base[c].host_address_ipv4, '\0', sizeof(shared_base[c].host_address_ipv4));
						memset(shared_base[c].host_address_ipv6, '\0', sizeof(shared_base[c].host_address_ipv6));
						shared_base[c].time_when_fetched = time(NULL);
						sem_post(&shared_base[c].host_lock);
					}
				}
			}
			else {
				for (c = 0; c < MAX_ROWS; c++) {
					if (strcmp(shared_base[c].host_name, name) == 0) {
						break;
					}
				}
				sem_trywait(&shared_base[c].host_lock);
				shared_base[c].in_use = 0;
				memset(shared_base[c].host_name, '\0', sizeof(shared_base[c].host_name));
				memset(shared_base[c].host_address_ipv4, '\0', sizeof(shared_base[c].host_address_ipv4));
				memset(shared_base[c].host_address_ipv6, '\0', sizeof(shared_base[c].host_address_ipv6));
				shared_base[c].time_when_fetched = time(NULL);
				sem_post(&shared_base[c].host_lock);
			}

			//	if (dblock == 0) {
			//		if (sem_post(sem) != 0) {
			//			printf("Failed unlock database\n");
			//		}
			//	}
			
		}//delete
		else if (strncmp(cmd, CMD_SAVE, strlen(CMD_SAVE)) == 0) {
			int c = 0;
			
			{
				int i = strlen(CMD_SAVE);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_SAVE)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}

			if (dblock == 1) {
				sem_post(sem);
				dblock = 0;
			}
			else {
				if (sem_trywait(sem) != 0) {
					printf("Database is locked, waiting...\n");
					sem_wait(sem);
				}
				sem_post(sem);
			}

			if (rowlock != -1) {
				sem_post(&shared_base[rowlock].host_lock);
				rowlock = -1;
			}
			else {
				for (c = 0; c < MAX_ROWS; c++) {
					if (sem_trywait(&shared_base[c].host_lock) != 0) {
						sem_wait(&shared_base[c].host_lock);
						break;
					}
					else
						sem_post(&shared_base[c].host_lock);
				}
			}
			
			{
				int fd = 0;
				umask(0);
				fd = open(name, O_CREAT|O_TRUNC|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

				for (; c < MAX_ROWS; c++) {
					if (shared_base[c].in_use == 1){
						sem_trywait(&shared_base[c].host_lock);
						write(fd, shared_base[c].host_name, strlen(shared_base[c].host_name));
						write(fd, "\n", strlen("\n"));
						sem_post(&shared_base[c].host_lock);
					}
				}

				close(fd);
			}
		}//save
		else if (strncmp(cmd, CMD_LOAD, strlen(CMD_LOAD)) == 0) {
			int c = 0;
			{
				int i = strlen(CMD_LOAD);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_LOAD)] = cmd[i];
					i++;
				}
			}

			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}

			{
				char row[PATH_MAX] = {'\0'};
				FILE *fp;

				// check if database is deleted
				if (shared_void == NULL) {
					printf("Not connected to database!\n");
					continue;
				}

				//check if have a self db lock
				if (dblock == 1) {
					printf("Cannot load while holding a database lock.\n");
					continue;
				}

				//check if have a self row lock
				if (rowlock != -1) {
					printf("Cannot load while holding a row lock.\n");
					continue;
				}
			
				//Don't have self db lock, check if there is other process lock db
				//if it is, block(wait) it
				//if not, trywait->post it
				if (sem_trywait(sem) != 0) {
					printf("Database currently locked, waiting...\n");
					sem_wait(sem);
				} 
				sem_post(sem);

				//Don't have self row lock, check if others lock other row
				//if find a row that has been locked by others, block it
				//and then post it
				for (c = 0; c < MAX_ROWS; c++) {
					if (sem_trywait(&shared_base[c].host_lock) != 0) {
						sem_wait(&shared_base[c].host_lock);
						break;
					}
					else
						sem_post(&shared_base[c].host_lock);
				}
				sem_post(&shared_base[c].host_lock);
			
				fp = fopen(name, "r");
				if (fp == NULL) {
					printf("No such file: %s", name);
					continue;
				}
				memset(name, '\0', sizeof(name));
				while(fgets(row, sizeof(row), fp) != NULL) {
					int i = 0;
					int flag = 0;
					while(row[i] != '\n') {
						name[i] = row[i];
						i++;
					}
					
					
					for (c = 0; c < MAX_ROWS; c++) {
						if (strcmp(shared_base[c].host_name, name) == 0) {
							flag = 1;
							break;
						}
					}

					if (flag) {
						memset(row, '\0', sizeof(row));
						memset(name, '\0', sizeof(row));
						continue;
					}
			
					for (c = 0; c < MAX_ROWS; c++) {					
						if ((shared_base[c].in_use == 0) && (sem_trywait(&shared_base[c].host_lock) == 0))
							break;
					}
			
					{
						char ip[2][INET6_ADDRSTRLEN];

						memset(ip[0], '\0', sizeof(ip[0]));
						memset(ip[1], '\0', sizeof(ip[1]));

						getIpByName(name, ip[0], ip[1], "\0", "\0");

						if (strncmp(ip[0], "error", strlen("error")) == 0) {
							printf("  *** no <%s>***\n", name);
							continue;
						}
						strncpy(shared_base[c].host_name, name, strlen(name));
						strncpy(shared_base[c].host_address_ipv4, ip[0], strlen(ip[0]));
						strncpy(shared_base[c].host_address_ipv6, ip[1], strlen(ip[1]));
						shared_base[c].time_when_fetched = time(NULL);
						shared_base[c].in_use = 1;
					}

					if (sem_post(&shared_base[c].host_lock) != 0) {
						printf("Unlock %d row failed!\n", c);
						continue;
					}

					{
						struct tm *t = (struct tm *)malloc(sizeof(struct tm));
						char ft[PATH_MAX] = {'\0'};
				
						t = localtime(&shared_base[c].time_when_fetched);
						strftime(ft, sizeof(ft), "%F %T %Z", t);
				
						printf("Name: %s\n", shared_base[c].host_name);
						printf("\tIPv4: %s\n", shared_base[c].host_address_ipv4);
						printf("\tIPv6: %s\n", shared_base[c].host_address_ipv6);
						printf("\tTime fetched: %s\n", ft);
					}
					
					memset(row, '\0', sizeof(row));
					memset(name, '\0', sizeof(name));
				}
				
				fclose(fp);
			}
		}//load
		else if (strncmp(cmd, CMD_DROP_DB, strlen(CMD_DROP_DB)) == 0) {
			
			if (shm_unlink(shm_name) != 0) {
				printf("Failed unlink shared memory!\nTry again!\n");
				continue;
			}
			if (munmap(shared_void, shared_seq_size) != 0) {
				printf("Failed unmap memory!\nTryagain!\n");
				continue;
			}
			if (close(sfd) != 0) {
				printf("Failed close shared object!\nTry again\n");
				continue;
			}
			dblock = 0;
			rowlock = -1;
			shared_void = NULL;
			shared_base = NULL;
			printf("Database droped\n");
		}//drop_db
		else if (strncmp(cmd, CMD_LOCK_DB, strlen(CMD_LOCK_DB)) == 0) {
			if (shared_void == NULL) {
				printf("Not connected a database\n");
				continue;
			}
			
			if (dblock == 1) {
				printf("Cannot lock database while already holding a database lock.\n");
				continue;
			}
			else {
				if (sem_trywait(sem) != 0) {
					printf("Database currently locked, waiting...\n");
					sem_wait(sem);
					dblock = 1;
				}
				else {
					dblock = 1;
					continue;
				}
			}
		}//lock_db
		else if (strncmp(cmd, CMD_UNLOCK_DB, strlen(CMD_UNLOCK_DB)) == 0) {
			if (shared_void == NULL) {
				printf("Not connected a database\n");
				continue;
			}

			if (dblock == 0) {
				printf("Cannot unlock database unless currently holding a database lock.\n");
				continue;
			}
			
			if (sem_post(sem) != 0) {
				printf("Failed to unlock database!\n");
				continue;
			}
			else {
				printf("Unlocked database\n");
				dblock = 0;
				continue;
			}
		}//unlock_db
		else if (strncmp(cmd, CMD_LOCK_ROW, strlen(CMD_LOCK_ROW)) == 0) {
			int c = 0;

			if (shared_void == NULL) {
				printf("Not connected a database\n");
				continue;
			}
			
			{
				int i = strlen(CMD_LOCK_ROW);
				while(cmd[i]!='\n') {
					name[i-strlen(CMD_LOCK_ROW)] = cmd[i];
					i++;
				}
			}
			
			{
				int i = 0;
				for (; i < strlen(name); i++) {
					if (name[i] == 32) {
						int j = i;
						for (;j < strlen(name); j++)
							name[j] = name[j+1];
					}
				}
			}

			if (rowlock != -1) {
				printf("Cannot lock row while holding a row lock\n");
				continue;
			}
			
			if (dblock == 1) {
				printf("Cannot lock row while holding a database lock\n");
				continue;
			}
			
			for (c = 0; c < MAX_ROWS; c++) {
				if (sem_trywait(&shared_base[c].host_lock) != 0) {
					sem_wait(&shared_base[c].host_lock);
					break;
				}
				else
					sem_post(&shared_base[c].host_lock);
			}
			
			for (c = 0; c < MAX_ROWS; c++) {
				if (strcmp(shared_base[c].host_name, name) == 0) {
					sem_wait(&shared_base[c].host_lock);
					rowlock = c;
					break;
				}
			}

			if (c == MAX_ROWS) {
				printf("hostname %s not in database\n", name);
				continue;
			}
		}//lock_row
		else if (strncmp(cmd, CMD_UNLOCK_ROW, strlen(CMD_UNLOCK_ROW)) == 0) {
		
			if (shared_void == NULL) {
				printf("Not connected a database\n");
				continue;
			}

			if (rowlock == -1) {
				printf("Cannot unlock row unless holding a row lock\n");
				continue;
			}

			sem_post(&shared_base[rowlock].host_lock);
			rowlock = -1;
		}//unlock_row
		else if (strncmp(cmd, CMD_COUNT, strlen(CMD_COUNT)) == 0) {
			int c = 0;
			int cnt = 0;
			
			if (shared_void == NULL) {
				printf("Cannot connect to database!\n");
				continue;
			}
			
			for (c = 0; c < MAX_ROWS; c++)
				if (shared_base[c].in_use == 1)
					cnt++;
			printf("Row count: %d\n", cnt);
		}//count
		else if (strncmp(cmd, CMD_LOCKS, strlen(CMD_LOCKS)) == 0) {
			int c = 0;
			if (shared_void == NULL) {
				printf("Cannot connect to database!\n");
				continue;
			}

			if (sem_trywait(sem) != 0) {
				printf("Database is locked.\n");
			}
			else {
				printf("Database is unlocked\n");
				sem_post(sem);
			}

			for (c = 0; c < MAX_ROWS; c++) {
				if (sem_trywait(&shared_base[c].host_lock) == 0) {
					sem_post(&shared_base[c].host_lock);
				}
				else {
					printf("Row %s (%d) is locked  0.", shared_base[c].host_name, c);
					break;
				}
			}
		}
	}//while 1

	//exit(EXIT_SUCCESS);
	//return 0;
}
