//Yunfan Li
//liyunf@onid.engr.oregonstate.edu
//CS344-001
//Assignment #2
//No References
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>
#include <utime.h>
#include "oscar.h"

int options[15] = {0};

void handle_a(int argc, char **argv);
void handle_t(int argc, char **argv);
void handle_T(int argc, char **argv);
void handle_d(int argc, char **argv);
void handle_e(int argc, char **argv);
void handle_E(int argc, char **argv);
int openArchiveFile(char *filename);
void substr(char *source, char *new, int n);
void isLegal(int fd, char *fname);

int main(int argc, char **argv)
{
	
	char tmp;
	while ((tmp = getopt(argc, argv, "aACdeEhmotTuvVS")) != -1) {
		switch (tmp) {
		case 'V':
			options[0] = 1;
			break;
		case 'h':
			options[1] = 1;
			break;
		case 'v':
			options[2] = 1;
			break;
		case 'a':
			options[3] = 1;
			break;
		case 'A':
			options[4] = 1;
			break;
		case 't':
			options[5] = 1;
			break;
		case 'T':
			options[6] = 1;
			break;
		case 'e':
			options[7] = 1;
			break;
		case 'E':
			options[8] = 1;
			break;
		case 'd':
			options[9] = 1;
			break;
		case 'm':
			options[10] = 1;
			break;
		case 'u':
			options[11] = 1;
			break;
		case 'C':
			options[12] = 1;
			break;
		case 'S':
			options[13] = 1;
			break;
		case 'o':
			options[14] = 1;
			break;
		default:
			printf("There are wrong options or wrong option format!\n");
			printf("Please use -h for help!\n");
			break;
		}
	}

	if (options[0] == 1) {
		printf("myoscar version: 0.0.1\nAuthor: Yunfan Li (Voldy)\n");
		exit(0);
	}
	if (options[1] == 1) {
		printf("Usage: myoscar <options> [archive-file] [member [...]]\n");
		printf("-a     Add member(s) from the command line to the archive file.\n");
		printf("-d     Delete member(s) from an archive file.\n");
		printf("-e     Extract member(s) from archive to file, from command line.\n");
		printf("-E     Extract member(s) from archive to file, from command line, keep current time.\n");
		printf("-h     Show the help text and exit.\n");
		printf("-o     Overwrite existing files on extract.\n");
		printf("-t     Short table of contents.\n");
		printf("-T     Long table of contents.\n");
		printf("-v     Verbose processing.\n");
		printf("-V     Print the version information and exit.\n");
		exit(0);
	}
	if (options[2] == 1) {
		printf("This is -v option print out information:\n\n");
	}
	if (options[3] == 1) {
		handle_a(argc, argv);
		exit(0);
	}
	if (options[4] == 1) {
		printf("Developing...\n");
		exit(0);
	}
	if (options[5] == 1) {
		handle_t(argc, argv);
		exit(0);
	}
	if (options[6] == 1) {
		handle_T(argc, argv);
		exit(0);
	}
	if (options[7] == 1) {
		handle_e(argc, argv);
		exit(0);
	}
	if (options[8] == 1) {
		handle_E(argc, argv);
		exit(0);
	}
	if (options[9] == 1) {
		handle_d(argc, argv);
		exit(0);
	}
	if (options[10] == 1) {
		printf("Developing...\n");
		exit(0);
	}
	if (options[11] == 1) {
		printf("Developing...\n");
		exit(0);
	}
	if (options[12] == 1) {
		printf("Developing...\n");
		exit(0);
	}
	if (options[13] == 1) {
		printf("Developing...\n");
		exit(0);
	}
	return 0;
}

void substr(char *source, char *new, int n)
{
	int i = 0;
	for (; i < n; i++)
		new[i] = source[i];
}

int openArchiveFile(char *filename)
{
	int fd;
	if ((fd = open(filename, O_WRONLY | O_APPEND)) == -1) {
		if (options[2] == 1)
			printf("Creating oscar file...\n");

		fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);

		if (options[2] == 1)
			printf("Success!\n");
		
		write(fd, OSCAR_ID, OSCAR_ID_LEN);

		return fd;
	}
	else {
		close(fd);
		fd = open(filename, O_RDONLY);
		isLegal(fd, filename);
		close(fd);
		fd = open(filename, O_WRONLY | O_APPEND);
		if (options[2] == 1)
			printf("Opened oscar file...\n");
		return fd;
	}
}

void isLegal(int fd, char *fname)
{
	char head[OSCAR_ID_LEN+1] = {'\0'};
	read(fd, head, OSCAR_ID_LEN);
	if (strcmp(head, OSCAR_ID) != 0) {
		printf("%s is not a oscar file!\n", fname);
		exit(0);
	}
}

void handle_a(int argc, char **argv)
{
	int i = optind+1, fd, archiveFd;
	char *archiveName = argv[optind];
	char *memberName = (char *)malloc(sizeof(char));
	struct stat *fileStat = (struct stat *)malloc(sizeof(struct stat)); 
	struct oscar_hdr_s *oscarInfo = (struct oscar_hdr_s *)malloc(sizeof(struct oscar_hdr_s));
	
	archiveFd = openArchiveFile(archiveName);

	if (i == argc) {
		printf("Must have at least 1 member file!\n");
		exit(0);
	}
	
	for (; i < argc; i++) {
		int j = 0;
		char buffer[OSCAR_MAX_MEMBER_FILE_SIZE] = {'\0'};
		char *spc = (char *)malloc(sizeof(char));

		spc = " ";
		memberName = argv[i];
		if ((int)strlen(basename(memberName)) > 30) {
			printf("Warning: File %s's name is too long to add in oscar file!\n", memberName);
			continue;
		}
		
		if (options[2] == 1)
			printf("Openning %s...\n", memberName);
		fd = open(memberName, O_RDONLY);
		read(fd, buffer, OSCAR_MAX_MEMBER_FILE_SIZE);
		fstat(fd, fileStat);
		
		if (!S_ISREG(fileStat->st_mode)) {
			printf("FATAL ERROR: File %s is not a regular file!\n", memberName);
			exit(0);
		}

		if (options[2] == 1)
			printf("Success!\n");
		sprintf(oscarInfo->oscar_fname, "%-30s", basename(memberName));
		sprintf(oscarInfo->oscar_fname_len, "%2d", (int)strlen(basename(memberName)));
		//sprintf(oscarInfo->oscar_fname_len, "11");
		sprintf(oscarInfo->oscar_adate, "%10lld", (long long)fileStat->st_atime);
		sprintf(oscarInfo->oscar_mdate, "%10lld", (long long)fileStat->st_mtime);
		sprintf(oscarInfo->oscar_cdate, "%10lld",(long long)fileStat->st_ctime);
		sprintf(oscarInfo->oscar_uid, "%5d", fileStat->st_uid);
		sprintf(oscarInfo->oscar_gid, "%5d", fileStat->st_gid);
		sprintf(oscarInfo->oscar_mode, "%6o", fileStat->st_mode);
		sprintf(oscarInfo->oscar_size, "%-16ld", (long)fileStat->st_size);
		
		oscarInfo->oscar_deleted = ' ';
		sprintf(oscarInfo->oscar_hdr_end, "%s", OSCAR_HDR_END);
		
		for (; j < OSCAR_SHA_DIGEST_LEN; j++)
			oscarInfo->oscar_sha[j] = 32;
		
		write(archiveFd, oscarInfo->oscar_fname, OSCAR_MAX_FILE_NAME_LEN);
		write(archiveFd, oscarInfo->oscar_fname_len, 2);
		write(archiveFd, oscarInfo->oscar_adate, OSCAR_DATE_SIZE);
		write(archiveFd, oscarInfo->oscar_mdate, OSCAR_DATE_SIZE);
		write(archiveFd, oscarInfo->oscar_cdate, OSCAR_DATE_SIZE);
		write(archiveFd, oscarInfo->oscar_uid, OSCAR_GUID_SIZE);
		write(archiveFd, oscarInfo->oscar_gid, OSCAR_GUID_SIZE);
		write(archiveFd, oscarInfo->oscar_mode, OSCAR_MODE_SIZE);
		write(archiveFd, oscarInfo->oscar_size, OSCAR_FILE_SIZE);
		write(archiveFd, spc, 1);
		write(archiveFd, oscarInfo->oscar_sha, OSCAR_SHA_DIGEST_LEN);
		write(archiveFd, oscarInfo->oscar_hdr_end, OSCAR_HDR_END_LEN);
		write(archiveFd, buffer, strlen(buffer));

		close(fd);
		if (options[2] == 1)
			printf("Added %s!\n", memberName);
	}
	close(archiveFd);
}

void handle_d(int argc, char **argv)
{
	int i = optind+1, archiveFd, fd, j = 0;
	char *archiveName = argv[optind];
	char **deleteName = (char **)malloc(sizeof(char *)); 
	oscar_hdr_t *fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
	char *newName = "tmp.osrcar";
	char fileContent[OSCAR_MAX_MEMBER_FILE_SIZE];
	
	if ((archiveFd = open(archiveName, O_RDONLY)) == -1) {
		printf("No such file: %s\n", archiveName);
		exit(0);
	}

	isLegal(archiveFd, archiveName);

	if (options[2] == 1)
		printf("Openning additional tmp file...\n");
	fd = open(newName, O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	write(fd, OSCAR_ID, OSCAR_ID_LEN);
	if (options[2] == 1)
		printf("Success!\n");

	if (i == argc) {
		printf("Must have at least 1 member file!\n");
		exit(0);
	}
	
	for (; i < argc; i++, j++) {
		deleteName[j] = argv[i];
	}
	
	
	while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
		long len = strtol(fileHead->oscar_size, NULL, 10);
		char fname[31] = {'\0'};
		int k = 0, flag = 0;

		while (fileHead->oscar_fname[k] != 32) {
			fname[k] = fileHead->oscar_fname[k];
			k++;
		}

		for (k = 0; k < j; k++) {
			if (strcmp(fname, deleteName[k]) == 0) {
				memset(deleteName[k], '\0', sizeof(deleteName[k]));
				flag = 1;
				break;
			}
		}

		if (flag == 0) {
			write(fd, fileHead, sizeof(oscar_hdr_t));
			read(archiveFd, fileContent, len);
			write(fd, fileContent, len);
		}
		else {
			if (options[2] == 1)
				printf("Deleting %s...\n", fname);
			read(archiveFd, fileContent, len);
			if (options[2] == 1)
				printf("Success!\n");
		}

		fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
		memset(fileContent, '\0', sizeof(fileContent));
	}
	
	remove(archiveName);
	rename(newName, archiveName);
	close(archiveFd);
	close(fd);
}

void handle_t(int argc, char **argv)
{
	int archiveFd;
	char *archiveName = argv[optind];
	struct oscar_hdr_s *fileHead = (struct oscar_hdr_s *)malloc(sizeof(struct oscar_hdr_s));
	char fileContent[OSCAR_MAX_MEMBER_FILE_SIZE];
	
	if ((archiveFd = open(archiveName, O_RDONLY)) == -1) {
		printf("No such file: %s\n", archiveName);
		exit(0);
	}

	isLegal(archiveFd, archiveName);

	
	
	printf("Short table of contents for oscar archive file: %s\n", archiveName);
	while (read(archiveFd, fileHead, sizeof(struct oscar_hdr_s)) != 0) {
		long len = strtol(fileHead->oscar_size, NULL, 10);
		int i = 0;
		printf("\t");
		while (fileHead->oscar_fname[i] != 32) 
			printf("%c", fileHead->oscar_fname[i++]);
		printf("\n");
		fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
		
		read(archiveFd, fileContent, len);
	}
	close(archiveFd);
}

void handle_T(int argc, char **argv)
{
	int archiveFd;
	char *archiveName = argv[optind];
	oscar_hdr_t *fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
	char fileContent[OSCAR_MAX_MEMBER_FILE_SIZE];

	if ((archiveFd = open(archiveName, O_RDONLY)) == -1) {
		printf("No such file: %s\n", archiveName);
		exit(0);
	}

	isLegal(archiveFd, archiveName);

	
	
	printf("Long table of contents for oscar archive file: %s\n", archiveName);
	while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
		long len = strtol(fileHead->oscar_size, NULL, 10);
		char fname[30] = {'\0'};
		int i = 0;

		char permission[6] = {'\0'};
		char fper[10] = {'\0'};
		char own[2] = {'\0'};
		char gro[2] = {'\0'};
		char oth[2] = {'\0'};
		long owner, group, other;

		char uid[6] = {'\0'}, gid[6] = {'\0'};
		long ui, gi;
		struct passwd *pwd = (struct passwd *)malloc(sizeof(struct passwd));
		struct group *grp = (struct group *)malloc(sizeof(struct group));

		char adate[11] = {'\0'};
		char mdate[11] = {'\0'};
		char cdate[11] = {'\0'};
		time_t atim = 0;
		time_t mtim = 0;
		time_t ctim = 0;
		struct tm *aat = (struct tm *)malloc(sizeof(struct tm));
		struct tm *mmt = (struct tm *)malloc(sizeof(struct tm));
		struct tm *cct = (struct tm *)malloc(sizeof(struct tm));
		char fadate[100] = {'\0'};
		char fmdate[100] = {'\0'};
		char fcdate[100] = {'\0'};

		read(archiveFd, fileContent, len);
		
		while (fileHead->oscar_fname[i] != 32) {
			fname[i] = fileHead->oscar_fname[i];
			i++;
		}

		printf("    File name: %s\n", fname);
		printf("\tFile size:   %ld bytes\n", len);

		substr(fileHead->oscar_mode, permission, OSCAR_MODE_SIZE);
		own[0] = permission[3];
		gro[0] = permission[4];
		oth[0] = permission[5];
		owner = strtol(own, NULL, 10);
		group = strtol(gro, NULL, 10);
		other = strtol(oth, NULL, 10);
		if (owner == 7) {
			fper[0] = 'r';
			fper[1] = 'w';
			fper[2] = 'x';
		}
		else if (owner == 6) {
			fper[0] = 'r';
			fper[1] = 'w';
			fper[2] = '-';
		}
		else if (owner == 5) {
			fper[0] = 'r';
			fper[1] = '-';
			fper[2] = 'x';
		}
		else if (owner == 4) {
			fper[0] = 'r';
			fper[1] = '-';
			fper[2] = '-';
		}
		else if (owner == 3) {
			fper[0] = '-';
			fper[1] = 'w';
			fper[2] = 'x';
		}
		else if (owner == 2) {
			fper[0] = '-';
			fper[1] = 'w';
			fper[2] = '-';
		}
		else if (owner == 1) {
			fper[0] = '-';
			fper[1] = '-';
			fper[2] = 'x';
		}
		else if (owner == 0) {
			fper[0] = '-';
			fper[0] = '-';
			fper[0] = '-';
		}

		if (group == 7) {
			fper[3] = 'r';
			fper[4] = 'w';
			fper[5] = 'x';
		}
		else if (group == 6) {
			fper[3] = 'r';
			fper[4] = 'w';
			fper[5] = '-';
		}
		else if (group == 5) {
			fper[3] = 'r';
			fper[4] = '-';
			fper[5] = 'x';
		}
		else if (group == 4) {
			fper[3] = 'r';
			fper[4] = '-';
			fper[5] = '-';
		}
		else if (group == 3) {
			fper[3] = '-';
			fper[4] = 'w';
			fper[5] = 'x';
		}
		else if (group == 2) {
			fper[3] = '-';
			fper[4] = 'w';
			fper[5] = '-';
		}
		else if (group == 1) {
			fper[3] = '-';
			fper[4] = '-';
			fper[5] = 'x';
		}
		else if (group == 0) {
			fper[3] = '-';
			fper[4] = '-';
			fper[5] = '-';
		}

		if (other == 7) {
			fper[6] = 'r';
			fper[7] = 'w';
			fper[8] = 'x';
		}
		else if (other == 6) {
			fper[6] = 'r';
			fper[7] = 'w';
			fper[8] = '-';
		}
		else if (other == 5) {
			fper[6] = 'r';
			fper[7] = '-';
			fper[8] = 'x';
		}
		else if (other == 4) {
			fper[6] = 'r';
			fper[7] = '-';
			fper[8] = '-';
		}
		else if (other == 3) {
			fper[6] = '-';
			fper[7] = 'w';
			fper[8] = 'x';
		}
		else if (other == 2) {
			fper[6] = '-';
			fper[7] = 'w';
			fper[8] = '-';
		}
		else if (other == 1) {
			fper[6] = '-';
			fper[7] = '-';
			fper[8] = 'x';
		}
		else if (other == 0) {
			fper[6] = '-';
			fper[7] = '-';
			fper[8] = '-';
		}

		printf("\tPermissions: %-16s(%c%ld%ld%ld)\n", fper, permission[2], owner, group, other);
		
		substr(fileHead->oscar_uid, uid, OSCAR_GUID_SIZE);
		substr(fileHead->oscar_gid, gid, OSCAR_GUID_SIZE);
		ui = strtol(uid, NULL, 10);
		gi = strtol(gid, NULL, 10);
		pwd = getpwuid(ui);
		grp = getgrgid(gi);
		printf("\tFile owner:  %-16s(uid: %ld)\n", pwd->pw_name, ui);
		printf("\tFile group:  %-16s(gid: %ld)\n", grp->gr_name, gi);

		substr(fileHead->oscar_adate, adate, OSCAR_DATE_SIZE);
		substr(fileHead->oscar_mdate, mdate, OSCAR_DATE_SIZE);
		substr(fileHead->oscar_cdate, cdate, OSCAR_DATE_SIZE);
		atim = strtol(adate, NULL, 10);
		mtim = strtol(mdate, NULL, 10);
		ctim = strtol(cdate, NULL, 10);
		aat = localtime(&atim);
		strftime(fadate, sizeof(fadate), "%F %T %z (%Z) %a", aat);
		mmt = localtime(&mtim);
		strftime(fmdate, sizeof(fmdate), "%F %T %z (%Z) %a", mmt);
		cct = localtime(&ctim);				
		strftime(fcdate, sizeof(fcdate), "%F %T %z (%Z) %a", cct);
		printf("\tAccess date: %s  %ld\n", fadate, atim);
		printf("\tModify date: %s  %ld\n", fmdate, mtim);
		printf("\tStatus date: %s  %ld\n", fcdate, ctim);
		printf("\tMarked deleted: no\n");
	}
	close(archiveFd);
}

void handle_e(int argc, char **argv)
{
	int archiveFd, i = optind+1, fd;
	char *archiveName = argv[optind];
	char fname[OSCAR_MAX_FILE_NAME_LEN];
	oscar_hdr_t *fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
	char fileContent[OSCAR_MAX_MEMBER_FILE_SIZE] = {'\0'};
	char **deleteName = (char **)malloc(sizeof(char *));
	
	if ((archiveFd = open(archiveName, O_RDONLY)) == -1) {
		printf("No such file: %s\n", archiveName);
		exit(0);
	}

	isLegal(archiveFd, archiveName);

	
	if (i == argc) {
		while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
			int j = 0;
			char adate[11] = {'\0'};
			char mdate[11] = {'\0'};
			long atim, mtim;
			char permission[7] = {'\0'};
			char own[2] = {'\0'};
			char grp[2] = {'\0'};
			char oth[2] = {'\0'};
			long owner, group, other;
			mode_t mod = 0;
			long len = strtol(fileHead->oscar_size, NULL, 10);
			struct utimbuf *buf = (struct utimbuf *)malloc(sizeof(struct utimbuf));	
			
			memset(fname, '\0', sizeof(fname));
			while (fileHead->oscar_fname[j] != 32) {
				fname[j] = fileHead->oscar_fname[j];
				j++;
			}
			read(archiveFd, fileContent, len);
			substr(fileHead->oscar_adate, adate, OSCAR_DATE_SIZE);
			substr(fileHead->oscar_mdate, mdate, OSCAR_DATE_SIZE);
			atim = strtol(adate, NULL, 10);
			mtim = strtol(mdate, NULL, 10);
			buf->actime = atim;
			buf->modtime = mtim;

			substr(fileHead->oscar_mode, permission, OSCAR_MODE_SIZE);
			own[0] = permission[3];
			grp[0] = permission[4];
			oth[0] = permission[5];
			owner = strtol(own, NULL, 10);
			group = strtol(grp, NULL, 10);
			other = strtol(oth, NULL, 10);
			
			if (owner == 7) {
				mod = mod | S_IRWXU;
			}
			else if (owner == 6) {
				mod = mod | S_IRUSR | S_IWUSR;
		   	}
			else if (owner == 5) {
				mod = mod | S_IRUSR | S_IXUSR;
			}
	   		else if (owner == 4) {
   				mod = mod | S_IRUSR;
   			}
			else if (owner == 3) {
				mod = mod | S_IWUSR | S_IXUSR;
			}
   			else if (owner == 2) {
   				mod = mod | S_IWUSR;
   			}
			else if (owner == 1) {
				mod = mod | S_IXUSR;
			}
			else if (owner == 0) {
				mod = mod | mod;
			}
			
   			if (group == 7) {
   				mod = mod | S_IRWXG;
   			}
   			else if (group == 6) {
   				mod = mod | S_IRGRP | S_IWGRP;
   			}
			else if (group == 5) {
				mod = mod | S_IRGRP | S_IXGRP;
			}
   			else if (group == 4) {
   				mod = mod | S_IRGRP;
   			}
			else if (group == 3) {
				mod = mod | S_IWGRP | S_IXGRP;
			}
	   		else if (group == 2) {
	   			mod = mod | S_IWGRP;
	   		}
			else if (group == 1) {
				mod = mod | S_IXGRP;
			}
			else if (group == 0) {
				mod = mod | mod;
			}

   			if (other == 7) {
   				mod = mod | S_IRWXO;
   			}
   			else if (other == 6) {
   				mod = mod | S_IROTH | S_IWOTH;
   			}
			else if (other == 5) {
				mod = mod | S_IROTH | S_IXOTH;
			}
   			else if (other == 4) {
   				mod = mod | S_IROTH;
   			}
			else if (other == 3) {
				mod = mod | S_IWOTH | S_IXOTH;
			}
   			else if (other == 2) {
   				mod = mod | S_IWOTH;
   			}
			else if (other == 1) {
				mod = mod | S_IXOTH;
			}
			
			if (options[14] == 1) {
				fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
				if (options[2] == 1)
					printf("Extrcating %s...\n", fname);
				write(fd, fileContent, len);
				close(fd);
				utime(fname, buf);
				chmod(fname, mod);
				if (options[2] == 1)
					printf("Success!\n");
			}
			else {
				int ac = access(fname, F_OK);
				if (ac != 0) {
					fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
					if (options[2] == 1)
						printf("Extracting %s...\n", fname);
					write(fd, fileContent, len);
					close(fd);
					utime(fname, buf);
					chmod(fname, mod);
					if (options[2] == 1)
						printf("Success!\n");
				}
			}
			fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
			memset(fileContent, '\0', sizeof(fileContent));
		}
	}
	else {
		int dl = 0;
		for (; i < argc; i++, dl++)
			deleteName[dl] = argv[i];

		while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
			int j = 0, flag = 0;
			char adate[11] = {'\0'};
			char mdate[11] = {'\0'};
			long atim, mtim;
			char permission[7] = {'\0'};
			char own[2] = {'\0'};
			char grp[2] = {'\0'};
			char oth[2] = {'\0'};
			long owner, group, other;
			mode_t mod = 0;
			long len = strtol(fileHead->oscar_size, NULL, 10);
			struct utimbuf *buf = (struct utimbuf *)malloc(sizeof(struct utimbuf));	
			memset(fname, '\0', sizeof(fname));
			while (fileHead->oscar_fname[j] != 32) {
				fname[j] = fileHead->oscar_fname[j];
				j++;
			}
			
			for (j = 0; j < dl; j++)
				if (strcmp(fname, deleteName[j]) == 0) {
					flag = 1;
					memset(deleteName[j], '\0', sizeof(deleteName[j]));
					break;
				}

			if (flag == 0) {
				read(archiveFd, fileContent, len);
				continue;
			}

			read(archiveFd, fileContent, len);
			substr(fileHead->oscar_adate, adate, OSCAR_DATE_SIZE);
			substr(fileHead->oscar_mdate, mdate, OSCAR_DATE_SIZE);
			atim = strtol(adate, NULL, 10);
			mtim = strtol(mdate, NULL, 10);
			buf->actime = atim;
			buf->modtime = mtim;

			substr(fileHead->oscar_mode, permission, OSCAR_MODE_SIZE);
			own[0] = permission[3];
			grp[0] = permission[4];
			oth[0] = permission[5];
			owner = strtol(own, NULL, 10);
			group = strtol(grp, NULL, 10);
			other = strtol(oth, NULL, 10);
			
			if (owner == 7) {
				mod = mod | S_IRWXU;
			}
			else if (owner == 6) {
				mod = mod | S_IRUSR | S_IWUSR;
		   	}
			else if (owner == 5) {
				mod = mod | S_IRUSR | S_IXUSR;
			}
	   		else if (owner == 4) {
   				mod = mod | S_IRUSR;
   			}
			else if (owner == 3) {
				mod = mod | S_IWUSR | S_IXUSR;
			}
   			else if (owner == 2) {
   				mod = mod | S_IWUSR;
   			}
			else if (owner == 1) {
				mod = mod | S_IXUSR;
			}
			else if (owner == 0) {
				mod = mod | mod;
			}
			
   			if (group == 7) {
   				mod = mod | S_IRWXG;
   			}
   			else if (group == 6) {
   				mod = mod | S_IRGRP | S_IWGRP;
   			}
			else if (group == 5) {
				mod = mod | S_IRGRP | S_IXGRP;
			}
   			else if (group == 4) {
   				mod = mod | S_IRGRP;
   			}
			else if (group == 3) {
				mod = mod | S_IWGRP | S_IXGRP;
			}
	   		else if (group == 2) {
	   			mod = mod | S_IWGRP;
	   		}
			else if (group == 1) {
				mod = mod | S_IXGRP;
			}
			else if (group == 0) {
				mod = mod | mod;
			}

   			if (other == 7) {
   				mod = mod | S_IRWXO;
   			}
   			else if (other == 6) {
   				mod = mod | S_IROTH | S_IWOTH;
   			}
			else if (other == 5) {
				mod = mod | S_IROTH | S_IXOTH;
			}
   			else if (other == 4) {
   				mod = mod | S_IROTH;
   			}
			else if (other == 3) {
				mod = mod | S_IWOTH | S_IXOTH;
			}
   			else if (other == 2) {
   				mod = mod | S_IWOTH;
   			}
			else if (other == 1) {
				mod = mod | S_IXOTH;
			}
			
			if (options[14] == 1) {
				fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
				if (options[2] == 1)
					printf("Extracting %s...\n", fname);
				write(fd, fileContent, len);
				close(fd);
				utime(fname, buf);
				chmod(fname, mod);
				if (options[2] == 1)
					printf("Success!\n");
			}
			else {
				int ac = access(fname, F_OK);
				if (ac != 0) {
					
					fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
					if (options[2] == 1)
						printf("Extracting %s...\n", fname);
					write(fd, fileContent, len);
					close(fd);
					utime(fname, buf);
					chmod(fname, mod);
					if (options[2] == 1)
						printf("Success!\n");
				}
			}
			fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
			memset(fileContent, '\0', sizeof(fileContent));
		}
	}
	close(archiveFd);
}

void handle_E(int argc, char **argv)
{
	int archiveFd, i = optind+1, fd;
	char *archiveName = argv[optind];
	char fname[OSCAR_MAX_FILE_NAME_LEN];
	oscar_hdr_t *fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
	char fileContent[OSCAR_MAX_MEMBER_FILE_SIZE] = {'\0'};
	char **deleteName = (char **)malloc(sizeof(char *));
	
	if ((archiveFd = open(archiveName, O_RDONLY)) == -1) {
		printf("No such file: %s\n", archiveName);
		exit(0);
	}

	isLegal(archiveFd, archiveName);

	
	if (i == argc) {
		while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
			int j = 0;
			char permission[7] = {'\0'};
			char own[2] = {'\0'};
			char grp[2] = {'\0'};
			char oth[2] = {'\0'};
			long owner, group, other;
			mode_t mod = 0;
			long len = strtol(fileHead->oscar_size, NULL, 10);
			
			memset(fname, '\0', sizeof(fname));
			while (fileHead->oscar_fname[j] != 32) {
				fname[j] = fileHead->oscar_fname[j];
				j++;
			}
			read(archiveFd, fileContent, len);
			substr(fileHead->oscar_mode, permission, OSCAR_MODE_SIZE);
			own[0] = permission[3];
			grp[0] = permission[4];
			oth[0] = permission[5];
			owner = strtol(own, NULL, 10);
			group = strtol(grp, NULL, 10);
			other = strtol(oth, NULL, 10);
			
			if (owner == 7) {
				mod = mod | S_IRWXU;
			}
			else if (owner == 6) {
				mod = mod | S_IRUSR | S_IWUSR;
		   	}
			else if (owner == 5) {
				mod = mod | S_IRUSR | S_IXUSR;
			}
	   		else if (owner == 4) {
   				mod = mod | S_IRUSR;
   			}
			else if (owner == 3) {
				mod = mod | S_IWUSR | S_IXUSR;
			}
   			else if (owner == 2) {
   				mod = mod | S_IWUSR;
   			}
			else if (owner == 1) {
				mod = mod | S_IXUSR;
			}
			else if (owner == 0) {
				mod = mod | mod;
			}
			
   			if (group == 7) {
   				mod = mod | S_IRWXG;
   			}
   			else if (group == 6) {
   				mod = mod | S_IRGRP | S_IWGRP;
   			}
			else if (group == 5) {
				mod = mod | S_IRGRP | S_IXGRP;
			}
   			else if (group == 4) {
   				mod = mod | S_IRGRP;
   			}
			else if (group == 3) {
				mod = mod | S_IWGRP | S_IXGRP;
			}
	   		else if (group == 2) {
	   			mod = mod | S_IWGRP;
	   		}
			else if (group == 1) {
				mod = mod | S_IXGRP;
			}
			else if (group == 0) {
				mod = mod | mod;
			}

   			if (other == 7) {
   				mod = mod | S_IRWXO;
   			}
   			else if (other == 6) {
   				mod = mod | S_IROTH | S_IWOTH;
   			}
			else if (other == 5) {
				mod = mod | S_IROTH | S_IXOTH;
			}
   			else if (other == 4) {
   				mod = mod | S_IROTH;
   			}
			else if (other == 3) {
				mod = mod | S_IWOTH | S_IXOTH;
			}
   			else if (other == 2) {
   				mod = mod | S_IWOTH;
   			}
			else if (other == 1) {
				mod = mod | S_IXOTH;
			}
			
			if (options[14] == 1) {
				fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
				if (options[2] == 1)
					printf("Extracting %s...\n", fname);
				write(fd, fileContent, len);
				close(fd);
				chmod(fname, mod);
				if (options[2] == 1)
					printf("Success!\n");
			}
			else {
				int ac = access(fname, F_OK);
				if (ac != 0) {
					
					fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
					if (options[2] == 1)
						printf("Extracting %s...\n", fname);
					write(fd, fileContent, len);
					close(fd);
					chmod(fname, mod);
					if (options[2] == 1)
						printf("Success!\n");
				}
			}
			fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
			memset(fileContent, '\0', sizeof(fileContent));
		}
	}
	else {
		int dl = 0;
		for (; i < argc; i++, dl++)
			deleteName[dl] = argv[i];

		while (read(archiveFd, fileHead, sizeof(oscar_hdr_t)) != 0) {
			int j = 0, flag = 0;
			char permission[7] = {'\0'};
			char own[2] = {'\0'};
			char grp[2] = {'\0'};
			char oth[2] = {'\0'};
			long owner, group, other;
			mode_t mod = 0;
			long len = strtol(fileHead->oscar_size, NULL, 10);
			
			memset(fname, '\0', sizeof(fname));
			while (fileHead->oscar_fname[j] != 32) {
				fname[j] = fileHead->oscar_fname[j];
				j++;
			}
			
			for (j = 0; j < dl; j++)
				if (strcmp(fname, deleteName[j]) == 0) {
					flag = 1;
					
					break;
				}

			if (flag == 0) {
				read(archiveFd, fileContent, len);
				continue;
			}

			read(archiveFd, fileContent, len);
			
			substr(fileHead->oscar_mode, permission, OSCAR_MODE_SIZE);
			own[0] = permission[3];
			grp[0] = permission[4];
			oth[0] = permission[5];
			owner = strtol(own, NULL, 10);
			group = strtol(grp, NULL, 10);
			other = strtol(oth, NULL, 10);
			
			if (owner == 7) {
				mod = mod | S_IRWXU;
			}
			else if (owner == 6) {
				mod = mod | S_IRUSR | S_IWUSR;
		   	}
			else if (owner == 5) {
				mod = mod | S_IRUSR | S_IXUSR;
			}
	   		else if (owner == 4) {
   				mod = mod | S_IRUSR;
   			}
			else if (owner == 3) {
				mod = mod | S_IWUSR | S_IXUSR;
			}
   			else if (owner == 2) {
   				mod = mod | S_IWUSR;
   			}
			else if (owner == 1) {
				mod = mod | S_IXUSR;
			}
			else if (owner == 0) {
				mod = mod | mod;
			}
			
   			if (group == 7) {
   				mod = mod | S_IRWXG;
   			}
   			else if (group == 6) {
   				mod = mod | S_IRGRP | S_IWGRP;
   			}
			else if (group == 5) {
				mod = mod | S_IRGRP | S_IXGRP;
			}
   			else if (group == 4) {
   				mod = mod | S_IRGRP;
   			}
			else if (group == 3) {
				mod = mod | S_IWGRP | S_IXGRP;
			}
	   		else if (group == 2) {
	   			mod = mod | S_IWGRP;
	   		}
			else if (group == 1) {
				mod = mod | S_IXGRP;
			}
			else if (group == 0) {
				mod = mod | mod;
			}

   			if (other == 7) {
   				mod = mod | S_IRWXO;
   			}
   			else if (other == 6) {
   				mod = mod | S_IROTH | S_IWOTH;
   			}
			else if (other == 5) {
				mod = mod | S_IROTH | S_IXOTH;
			}
   			else if (other == 4) {
   				mod = mod | S_IROTH;
   			}
			else if (other == 3) {
				mod = mod | S_IWOTH | S_IXOTH;
			}
   			else if (other == 2) {
   				mod = mod | S_IWOTH;
   			}
			else if (other == 1) {
				mod = mod | S_IXOTH;
			}
			
			if (options[14] == 1) {
				fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
				if (options[2] == 1)
					printf("Extracting %s...\n", fname);
				write(fd, fileContent, len);
				close(fd);
				chmod(fname, mod);
				if (options[2] == 1)
					printf("Success!\n");
			}
			else {
				int ac = access(fname, F_OK);
				if (ac != 0) {
			
					fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, mod);
					if (options[2] == 1)
						printf("Extracting %s...\n", fname);
					write(fd, fileContent, len);
					close(fd);
					chmod(fname, mod);
					if (options[2] == 1)
						printf("Success!\n");
				}
			}
			fileHead = (oscar_hdr_t *)malloc(sizeof(oscar_hdr_t));
			memset(fileContent, '\0', sizeof(fileContent));
		}
	}
	close(archiveFd);
}
