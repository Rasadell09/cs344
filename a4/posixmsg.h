/*
 * R Jesse Chaney
 * chaneyr@eecs.orst.edu
 * CS344-001
 * Fall 2014
 * Oregon State University
 * School of EECS
 */

/*
 * $Author: chaneyr $
 * $Date: 2015/02/27 18:16:37 $
 * $RCSfile: posixmsg.h,v $
 * $Revision: 1.8 $
 */


#ifndef _POSIXMSG_H_
# define _POSIXMSG_H_

# include <limits.h>

// The basename for the various FIFOs used in the programs.
// They are used in the macos below.
# define SERVER_QUEUE_FILENAME    "ServerQueue__"
# define CLIENT_QUEUE_READER_NAME "ClientReaderQueue__"
# define CLIENT_QUEUE_WRITER_NAME "ClientWriterQueue__"

// Some handy macros for creating file names of the message queues.
// _BUF_ is a char *
// _PID_ is an int (HINT: you may need to cast getpid())
# define CREATE_SERVER_QUEUE_NAME(_BUF_) sprintf(_BUF_,"/%s_%s"\
						 ,SERVER_QUEUE_FILENAME,getenv("LOGNAME"));
# define CREATE_CLIENT_READER_NAME(_BUF_,_PID_) sprintf(_BUF_,"/%s_%s_%d"\
						,CLIENT_QUEUE_READER_NAME,getenv("LOGNAME"),_PID_);
# define CREATE_CLIENT_WRITER_NAME(_BUF_,_PID_) sprintf(_BUF_,"/%s_%s_%d"\
						,CLIENT_QUEUE_WRITER_NAME,getenv("LOGNAME"),_PID_);

// When creating a message queue, use these permissions.
# define QUEUE_PERMISSIONS  (S_IRUSR | S_IWUSR)

// When sending a file (put and get), use these flags and permissions
// on the file created.
# define SEND_FILE_FLAGS (O_CREAT | O_WRONLY | O_TRUNC)
# define SEND_FILE_PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP)

// The maximum number of concurrently connected clients to the
// the server.
# define MAX_CLIENTS     10

// When using POSIX message queues, you see the error
// message ": Message too long" when trying to read from a
// message queue, it probably means that your message queue
// buffer is not long enough.  Strange, but true.
# define COMMAND_LENGTH  11
# define PAYLOAD_LENGTH  PATH_MAX

// Various message types.  This is a little like an enumaration.
// In fact, making is an enumaration would be a good idea.
# define MESSAGE_TYPE_ERROR    -1
# define MESSAGE_TYPE_NORMAL   0
# define MESSAGE_TYPE_DIR      1
# define MESSAGE_TYPE_DIR_END  2
# define MESSAGE_TYPE_SEND     3
# define MESSAGE_TYPE_SEND_END 4

typedef struct message_s {
  int message_type;
  char command[COMMAND_LENGTH];
  char payload[PAYLOAD_LENGTH];
  int num_bytes;
} message_t;


// This is a list of the available commands for the client.
// You can get more about them by typing 'help' into the client.
# define CMD_EXIT         "exit"

# define CMD_REMOTE_HOME  "home"
# define CMD_LOCAL_HOME   "lhome"

# define CMD_REMOTE_CHDIR "cd"
# define CMD_REMOTE_DIR   "dir"
# define CMD_REMOTE_PWD   "pwd"

# define CMD_LOCAL_CHDIR  "lcd"
# define CMD_LOCAL_DIR    "ldir"
# define CMD_LOCAL_PWD    "lpwd"

# define CMD_PUT          "put"
# define CMD_GET          "get"
# define CMD_HELP         "help"


// This is the command to use with popen() to get a
// directory listing.
# define CMD_LS_POPEN    "ls -lFa"

# define PROMPT    "+++ "

//# define DIR_SEP "/"
//# define RETURN_ERROR "**ERROR**"

#endif // _POSIXMSG_H_
