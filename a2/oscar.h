/*
 * R Jesse Chaney
 * chaneyr@eecs.orst.edu
 * CS344-001
 * School of EECS
 * Oregon State University
 * Corvallis, Oregon
 */

/*
 * oscar.h
 *
 */

/*
 * $Author: chaneyr $
 * $Date: 2015/01/22 03:07:56 $
 * $RCSfile: oscar.h,v $
 * $Revision: 1.9 $
 */

#ifndef OSCAR_H_
# define OSCAR_H_

# include <string.h>
# include <sys/cdefs.h>

/*
 * oscar archive files start with the OSCAR_ID identifying string.  Then follows a
 * 'struct oscar_hdr', and as many bytes of member file data as its 'oscar_size'
 * member indicates, for each member file.
 */

# define OSCAR_ID	"!<oscar>\n"     /* String that begins an oscar archive file. */
# define OSCAR_ID_LEN	strlen(OSCAR_ID) /* Size of the oscar id string string. */

# define OSCAR_HDR_END	"++\n"		 /* String at end of each member header.  */
# define OSCAR_HDR_END_LEN  strlen(OSCAR_HDR_END)

# define OSCAR_MAX_FILE_NAME_LEN     30
# define OSCAR_DATE_SIZE	     10
# define OSCAR_GUID_SIZE	     5
# define OSCAR_MODE_SIZE	     6
# define OSCAR_FILE_SIZE	     16
# define OSCAR_MAX_MEMBER_FILE_SIZE  1000000 /* Just make things a little easier. */

# ifndef DO_SHA  // Enable the SHA256 code.
#  define DO_SHA
# endif // DO_SHA

# ifdef DO_SHA
#  include <openssl/sha.h>
# endif // DO SHA

# ifdef DO_SHA
#  define OSCAR_SHA_DIGEST_LEN        (SHA256_DIGEST_LENGTH * 2)
# else // DO_SHA
// If not buidling with the sha 256, I need to go ahead and put a value in
// here.  I don't like at all that I have to try and keep the value of
// SHA256_DIGEST_LENGTH the same as the value here.
#  define OSCAR_SHA_DIGEST_LEN        (/* SHA256_DIGEST_LENGTH */ 32 * 2)
# endif // DO_SHA

// Notice that is both a struct and a typedef.  You can declare it using
// either just the typedef name or with the struct ... syntax.
typedef struct oscar_hdr_s {
  char oscar_fname[OSCAR_MAX_FILE_NAME_LEN]; /* Member file name, may not be NULL terminated. */
  char oscar_fname_len[2];		     /* The length of the member file name */
  char oscar_adate[OSCAR_DATE_SIZE];	     /* File access date, decimal sec since Epoch. */
  char oscar_mdate[OSCAR_DATE_SIZE];	     /* File modify date, decimal sec since Epoch. */
  char oscar_cdate[OSCAR_DATE_SIZE];	     /* Time of last status change, decimal sec since Epoch. */
  char oscar_uid[OSCAR_GUID_SIZE];	     /* user id in ASCII decimal */
  char oscar_gid[OSCAR_GUID_SIZE];           /* group id in ASCII decimal. */
  char oscar_mode[OSCAR_MODE_SIZE];	     /* File mode, in ASCII octal. */
  char oscar_size[OSCAR_FILE_SIZE];	     /* File size, in ASCII decimal. */
  char oscar_deleted;			     /* If member is deleted = y, otherwise a space */
  char oscar_sha[OSCAR_SHA_DIGEST_LEN];      /* SHA256 check bits */
  char oscar_hdr_end[OSCAR_HDR_END_LEN];     /* Always contains OSCAR_HDR_END. */
} oscar_hdr_t;

#endif /* OSCAR_H_ */
