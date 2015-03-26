#ifndef __MP3_CHECK_H__
#define __MP3_CHECK_H__ 1

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*
 * This is where all the portability/useability tests are done. A lot of
 * this code is from the "system.h" file created from the autoproject
 * program. autoproject is a GNU licenced program.
 */
#include <sys/types.h>

/* <unistd.h> should be included before any preprocessor test
   of _POSIX_VERSION.  */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

/* Don't use bcopy!  Use memmove if source and destination may overlap,
   memcpy otherwise.  */

#ifdef HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#else
# include <strings.h>
char *memchr ();
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#ifdef STDC_HEADERS
#define getopt system_getopt
#include <stdlib.h>
#undef getopt
#else /* not STDC_HEADERS */
char *getenv ();
#endif /* STDC_HEADERS */

/* The following test is to work around the gross typo in
   systems like Sony NEWS-OS Release 4.0C, whereby EXIT_FAILURE
   is defined to 0, not 1.  */
#if !EXIT_FAILURE
# undef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif

#include <ctype.h>

#include <ctype.h>

/* Jim Meyering writes:

   "... Some ctype macros are valid only for character codes that
   isascii says are ASCII (SGI's IRIX-4.0.5 is one such system --when
   using /bin/cc or gcc but without giving an ansi option).  So, all
   ctype uses should be through macros like ISPRINT...  If
   STDC_HEADERS is defined, then autoconf has verified that the ctype
   macros don't need to be guarded with references to isascii. ...
   Defining isascii to 1 should let any compiler worth its salt
   eliminate the && through constant folding."

   Bruno Haible adds:

   "... Furthermore, isupper(c) etc. have an undefined result if c is
   outside the range -1 <= c <= 255. One is tempted to write isupper(c)
   with c being of type `char', but this is wrong if c is an 8-bit
   character >= 128 which gets sign-extended to a negative value.
   The macro ISUPPER protects against this as well."  */

#if defined (STDC_HEADERS) || (!defined (isascii) && !defined (HAVE_ISASCII))
#define IN_CTYPE_DOMAIN(c) 1
#else
#define IN_CTYPE_DOMAIN(c) isascii(c)
#endif

#ifdef isblank
#define ISBLANK(c) (IN_CTYPE_DOMAIN (c) && isblank (c))
#else
#define ISBLANK(c) ((c) == ' ' || (c) == '\t')
#endif
#ifdef isgraph
#define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isgraph (c))
#else
#define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isprint (c) && !isspace (c))
#endif

#define ISPRINT(c) (IN_CTYPE_DOMAIN (c) && isprint (c))
#define ISALNUM(c) (IN_CTYPE_DOMAIN (c) && isalnum (c))
#define ISALPHA(c) (IN_CTYPE_DOMAIN (c) && isalpha (c))
#define ISCNTRL(c) (IN_CTYPE_DOMAIN (c) && iscntrl (c))
#define ISLOWER(c) (IN_CTYPE_DOMAIN (c) && islower (c))
#define ISPUNCT(c) (IN_CTYPE_DOMAIN (c) && ispunct (c))
#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
#define ISUPPER(c) (IN_CTYPE_DOMAIN (c) && isupper (c))
#define ISXDIGIT(c) (IN_CTYPE_DOMAIN (c) && isxdigit (c))
#define ISDIGIT_LOCALE(c) (IN_CTYPE_DOMAIN (c) && isdigit (c))

/* ISDIGIT differs from ISDIGIT_LOCALE, as follows:
   - Its arg may be any int or unsigned int; it need not be an unsigned char.
   - It's guaranteed to evaluate its argument exactly once.
   - It's typically faster.
   Posix 1003.2-1992 section 2.5.2.1 page 50 lines 1556-1558 says that
   only '0' through '9' are digits.  Prefer ISDIGIT to ISDIGIT_LOCALE unless
   it's important to use the locale's definition of `digit' even when the
   host does not conform to Posix.  */
#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)

#ifndef __P
#if defined (__GNUC__) || (defined (__STDC__) && __STDC__)
#define __P(args) args
#else
#define __P(args) ()
#endif  /* GCC.  */
#endif  /* Not __P.  */

/* Take care of NLS matters.  */

#if HAVE_LOCALE_H
# include <locale.h>
#endif
#if !HAVE_SETLOCALE
# define setlocale(Category, Locale) /* empty */
#endif

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) Text
#endif

/*
 * The following pertains exclusively to mp3_check
 */
#define HOMEPAGE	"http://sourceforge.net/project/?group_id=6126"
#define AUTHOR		"Eric Bullen <ericb at thedeepsky.com>"

#define FALSE   (0)
#define TRUE    (!FALSE)

#define PASS    TRUE
#define FAIL    FALSE

#define YES	TRUE
#define NO	FALSE

#define BUFFER_LENGTH 128

//
// General MP3 file info
//
typedef struct gen_info_s {
	int	file_pos;
	int	good_frame_count;
	int	bad_frame_count;

	// This variable stores the number of
	// contiguous number of frames found, and is reset
	// back to zero on the discovery of a
	// bad frame. This is used for the '-q'
	// option.
	int	frame_sequence_count;
	int	byte_count;

	//  This is set to 4 so it works corectly
	//  with a frame being the first thing in a file.

	int	next_expected_frame;

	// This stores the length of the song in seconds.
	double	time_in_seconds;
} gen_info;

//
// VBR Structure
//
typedef struct vbr_data_s {
        int	high_rate;
        int	low_rate;
        int	sum_rate;
        int	ave_rate;
} vbr_data;


//
// MP3 Time Structure
//
typedef struct mp3_time_s {
	int	minutes;
	int	seconds;
	int	frac_second;
} mp3_time;


//
// Structure to hold the flag options.
//
typedef struct meta_options_s {
	// Well, there are flags, and then 
	// there are the opions that the flags
	// can have. This structure stores those
	// options. This structure is small
	// now, but it'll probably grow pretty fast.

	int		byte_limit;
	int		min_frame_seq;

} meta_options;

//  
//  Store all the flags in a cool structure for easy referencing 
//  in funtions. 
//  
typedef struct command_flags_s {
	//  
	//  These are the flags that come  
	//  off the commandline. 
	//  
	int		aflag;
	int		bflag;
	int		eflag;
	int		fflag;
	int		iflag;
	int		pflag;
	int		qflag;
	int		sflag;
	int		ssflag;
	int		vflag;
	int		vvflag;
} command_flags;	


//  
//  Create a structure for easy frame info storage 
//  
typedef struct frame_info_s {
	//  
	//  Declare variables for the header storage 
	//  
	int		FRAME_LENGTH;
	int		BIT_RATE;
	int		SAMPLE_FREQ;
	int		SAMPLES_PER_FRAME;

	short int	MPV_1, MPV_2, MPV_25, MPV_RESERVED; // MPEG Version 
	short int	L1, L2, L3, L_RESERVED; // Layer Version 
	short int	PROT_BIT; // Protection Bit 
	short int	PAD_BIT;
	short int	PRIV_BIT;
	short int	STEREO;
	short int	JOINT_STEREO;
	short int	DUAL_STEREO;
	short int	SINGLE_CHANNEL;
	short int	MODE_EXTENSION; // Do not use this one right now. 
	short int	ID3V2;
	short int	COPYRIGHT;
	short int	ORIGINAL;
	short int	EMPH_NONE;
	short int	EMPH_5015;
	short int	EMPH_RESERV;
	short int	EMPH_CCIT;

	char		BIN_STRING[33];
	int		INT_HEADER;
	int		CRC16_VALUE;

	short int	check_state;
} frame_info;


//  
//  This gives info about the id3 tag found in the last part 
//  of the mp3 file. 
//  
typedef struct id3_tag_info_s {
	short int	TAG_PRESENT;
	short int	ID3_311_VERSION;
	char	    	TITLE[31];
	char	    	ARTIST[31];
	char	    	ALBUM[31];
	char	    	YEAR[5];
	char	    	COMMENT[31];
	short int	GENRE;
	short int	TRACK_NUMBER;
	short int	COMPLIANT_PAD_FIELDS;
} id3_tag_info;

/* File: id3.c --- ID3 manipulation */
extern int dump_id3_tag(id3_tag_info *id3_tag);
extern int validate_id3_tag(char *possible_mp3_tag, id3_tag_info *id3_tag);

/* File: init.c --- initialize the various stuctures */
extern void init_command_flags_struct(command_flags *flags);
extern void init_frame_struct(frame_info *FI);
extern void init_vbr_tag_struct(vbr_data *vbr_info);
extern void init_gen_info_struct(gen_info *file_info);
extern void init_mp3_time_struct(mp3_time *song_time);
extern void init_id3_tag_struct(id3_tag_info *TAG);

/* File: mp3_check_misc.c --- scan the MP3 files */
extern int parse_args(char **argv, meta_options *flag_options,
		      command_flags *flags);

/* File: print.c --- printing/reporting */
extern void print_usage(void);
extern void print_summary(frame_info* mp3_i, char* filename);
extern void print_sys_usage(void);
extern int print_frame_info(frame_info* mp3_i, gen_info* file_info);

/* File: support_functions.c --- MP3 header testing */
extern int check_header_value(unsigned int *header, char *filename,
			      frame_info *FI);
extern int transform_char_array(char *byte_list, gen_info *file_info);
extern void translate_time(gen_info *file_info, mp3_time *song_time);

#endif
