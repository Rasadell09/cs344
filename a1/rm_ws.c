//Yunfan Li
//liyunf@onid.oregonstate.edu
//CS344-001
//Assignment #1
//No references
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_NUM 1024

void handle_b(char *s);
void handle_i(char *s);
void handle_e(char *s);

//Delete all the spaces and tabs at front
void handle_b(char *s)
{
	while ((s[0] == 32) || (s[0] == 9)) {
		int i = 1;
		for (; i <= strlen(s); i++)
			s[i-1] = s[i];
	}
}

//Delete all the spaces and tabs at middle
void handle_i(char *s)
{
	int i = 0;
	int start, end;
	while ((s[i] == 32) || (s[i] == 9))
		i++;
	start = i;
	i = strlen(s);
	while ((s[i] == 32) || (s[i] == 9) || (s[i] == '\n'))
		i--;
	end = i;
	for (i = start; i <= end; i++)
		while ((s[i] == 32) || (s[i] == 9)) {
			int j = i+1;
			for(; j <= strlen(s); j++)
				s[j-1] = s[j];
			end--;
		}
}

//Delete all the spaces and tabs at end
void handle_e(char *s)
{
	int i = strlen(s)-1;
	while ((s[i] == 32) || (s[i] == 9) || (s[i] == '\n')) {
        s[i] = '\0';
		i--;
	}
    s[i+1] = '\n';
}

int main(int argc, char **argv)
{	
	char buf[MAX_NUM], opt[3], tmp;
	
	memset(buf, -1, MAX_NUM);
	memset(opt, 0, 3);

	if(argc > 1)
		while ((tmp = getopt(argc, argv, "bei")) != -1)
			switch (tmp) {
			case 'b':
				opt[0] = 1;
				break;
			case 'i':
				opt[1] = 1;
				break;
			case 'e':
				opt[2] = 1;
				break;
			}
	
	while (fgets(buf, MAX_NUM, stdin) != NULL) {
		if (opt[0] == 1)
			handle_b(buf);
		if (opt[1] == 1)
			handle_i(buf);
		if (opt[2] == 1)
			handle_e(buf);
		printf("%s", buf);
	}

	return 0;
}
