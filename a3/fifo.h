 /*
 * R Jesse Chaney
 * chaneyr@eecs.orst.edu
 * CS344-001
 * Oregon State University
 * School of EECS
 */

/*
 * $Author: chaneyr $
 * $Date: 2015/02/02 12:42:14 $
 * $RCSfile: fifo.h,v $
 * $Revision: 1.8 $
 */

#ifndef __FIFO_H_
# define __FIFO_H__

# include <stdlib.h>

// The basename for the various FIFOs used in the programs.
// They are used in the macos below.
# define SERVER_FIFO_FILENAME    "FifoServer__"
# define CLIENT_FIFO_READER_NAME "FifoClientReader__"
# define CLIENT_FIFO_WRITER_NAME "FifoClientWriter__"

// Some handy macros for creating file names of the FIFOs.
// _BUF_ is a char *
// _PID_ is an int (HINT: you may need to cast getpid())
# define CREATE_SERVER_FIFO_NAME(_BUF_) sprintf(_BUF_,"%s/%s"\
		,getenv("HOME"),SERVER_FIFO_FILENAME);

// These names from from the perspective of the client process.  The
// client process reads from the CLIENT_READER and writes to the
// CLIENT_WRITER.
// The "client server" reads from the CLEINT_WRITER and writes to
// the CLIENT_READER.
# define CREATE_CLIENT_READER_NAME(_BUF_,_PID_) sprintf(_BUF_,"%s/%s%d"\
		,getenv("HOME"),CLIENT_FIFO_READER_NAME,_PID_);
# define CREATE_CLIENT_WRITER_NAME(_BUF_,_PID_) sprintf(_BUF_,"%s/%s%d"\
		,getenv("HOME"),CLIENT_FIFO_WRITER_NAME,_PID_);

// These are permissions you should use to create your new FIFOs.
# define FIFO_PERMISSIONS  (S_IRUSR | S_IWUSR)

// This is a list of the available commands for the client.
// You can get more about them by typing 'help' into the client.
# define CMD_EXIT         "exit"

# define CMD_REMOTE_HOME  "home"
# define CMD_LOCAL_HOME   "lhome"

# define CMD_REMOTE_CHDIR "cd"
# define CMD_REMOTE_LS    "ls"
# define CMD_REMOTE_DIR   "dir"
# define CMD_REMOTE_PWD   "pwd"

# define CMD_LOCAL_CHDIR  "lcd"
# define CMD_LOCAL_LS     "lls"
# define CMD_LOCAL_DIR    "ldir"
# define CMD_LOCAL_PWD    "lpwd"

# define CMD_PUT          "put"
# define CMD_GET          "get"
# define CMD_HELP         "help"

// Might be handy
# define NEW_LINE         '\n'

// The prompt the client shows.
# define CLIENT_PROMPT    ">>> "

// When the client server is unhappy about something.
# define RETURN_ERROR     "SERVER ERROR\n"

// This is the command to use with popen() to get a
// direcctory listing.
# define CMD_LS_POPEN    "ls -lahiSr"

#endif /* __FIFO_H__ */
