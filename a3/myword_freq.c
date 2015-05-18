#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
	int p1[2], p2[2], p3[2], p4[2], p5[2];

	if (argv[1] != NULL)
		freopen(argv[1], "r", stdin);

	pipe(p1);
	{
		switch(fork()) {
		case -1:
			perror("failed creating fork for p1!");
			return(EXIT_FAILURE);
			break;
		case 0:
			if (dup2(p1[1], 1) == -1) {
				perror("sed failed dup2");
				return(EXIT_FAILURE);
			}

			close(p1[0]);

			execlp("sed", "sed", "s/[^a-zA-Z]/ /g", (char *)NULL);
			perror("child process cannot execlp sed!");
			fprintf(stderr,"%d child process cannot execlp sed!\n", getpid());
			exit(EXIT_FAILURE);

			break;
		default:						
			close(p1[1]);
			break;
		}
	}//pipe p1
	
	pipe(p2);
	{
		switch(fork()) {
		case -1:
			perror("failed creating fork for p2!");
			return(EXIT_FAILURE);
			break;
		case 0:
			if (dup2(p1[0], 0) == -1) {
				perror("tr failed dup2 p1");
				return(EXIT_FAILURE);
			}

	   		if (dup2(p2[1], 1) == -1) {
				perror("tr failed dup2 p2");
				return(EXIT_FAILURE);
			}

			close(p1[0]);
			close(p2[0]);

			execlp("tr", "tr", "[A-Z]", "[a-z]", (char *)NULL);
			perror("child process cannot execlp tr!");
			fprintf(stderr,"%d child process cannot execlp tr!\n", getpid());
			exit(EXIT_FAILURE);

			break;
		default:
			close(p1[0]);			
			close(p2[1]);
			break;
		}
	}//pipe p2

	pipe(p3);
	{
		switch(fork()) {
		case -1:
			perror("failed creating fork for p3!");
			return(EXIT_FAILURE);
			break;
		case 0:
			if (dup2(p2[0], 0) == -1) {
				perror("awk failed dup2 p2");
				return(EXIT_FAILURE);
			}

  			if (dup2(p3[1], 1) == -1) {
				perror("awk failed dup2 p3");
				return(EXIT_FAILURE);
			}

			close(p2[0]);
			close(p3[0]);

			execlp("awk", "awk", "{ for(i = 1; i <= NF; i++) {print $i; } }", (char *)NULL);
			perror("child process cannot execlp awk!");
			fprintf(stderr, "%d child process cannot execlp awk!\n", getpid());
			exit(EXIT_FAILURE);

			break;
		default:
			close(p2[0]);
			close(p3[1]);
			break;
		}
	}//pipe p3

	pipe(p4);
	{
		switch(fork()) {
		case -1:
			perror("failed creating fork for p4!");
			return(EXIT_FAILURE);
			break;
		case 0:
			if (dup2(p3[0], 0) == -1) {
				perror("sort failed dup2 p3");
				return(EXIT_FAILURE);
			}

			if (dup2(p4[1], 1) == -1) {
				perror("sort failed dup2 p4");
				return(EXIT_FAILURE);
			}

			close(p3[0]);
			close(p4[0]);

			execlp("sort", "sort", (char *)NULL);
			perror("child process cannot execlp sort!");
			fprintf(stderr, "%d child process cannot execlp sort!\n", getpid());
			exit(EXIT_FAILURE);

			break;
		default:
			close(p3[0]);			
			close(p4[1]);
			break;
		}
	}//pipe p4

	pipe(p5);
	{
		switch(fork()) {
		case -1:
			perror("failed creating fork for p5!");
			return(EXIT_FAILURE);
			break;
		case 0:
			if (dup2(p4[0], 0) == -1) {
				perror("uniq failed dup2 p4");
				return(EXIT_FAILURE);
			}

			if (dup2(p5[1], 1) == -1) {
				perror("uniq failed dup2 p5");
				return(EXIT_FAILURE);
			}

			close(p4[0]);
			close(p5[0]);

			execlp("uniq", "uniq", "-c", (char *)NULL);
			perror("child process failed execlp uniq!");
			fprintf(stderr, "%d child process failed execlp uniq!\n", getpid());
			exit(EXIT_FAILURE);

			break;
		default:
			if (dup2(p5[0], 0) == -1) {
				perror("sort -nr failed dup2 p5");
				return(EXIT_FAILURE);
			}
			close(p4[0]);
			close(p5[0]);
			close(p5[1]);

			execlp("sort", "sort", "-nr", (char *)NULL);
			perror("parent process failed execlp sort -nr!");
			fprintf(stderr, "%d parent process failed execlp sort -nr!\n", getpid());
			exit(EXIT_FAILURE);
			break;
		}
	}//pipe p5

	//	return(EXIT_SUCCESS);
}
