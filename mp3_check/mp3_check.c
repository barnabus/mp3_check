/**********************************************************************
PROGRAM NAME:	mp3_check

VERSION:	1.97	

AUTHOR:		Eric Bullen <ericb@thedeepsky.com>

PURPOSE:	Identify in explicit detail mp3s that do not correctly
		follow the mp3 format. Also look for invalid frame
		headers, missing frames, etc. This can be especially
		important when building an archive, and you want 
		quality mp3s.

USAGE:		mp3s are accepted either from stdin, or on the
		command-line. Statistical data and summary information
		is sent to stout, and any errors are sent to stderr. 
		More information is available by using the '-h' switch
		on the commandline.

		Common execution can be:
		========================

		cat some_song.mp3 | mp3_check -s - | mpg123

		OR

		mp3_check -a -v some_song.mp3

		INTERESTING TEST

		mp3_check -b100000 -vv /dev/urandom
		

REFERENCES:	All original code. However the format of an mp3 file
		was obtained from long-lost web sites and nntp articles.

RESTRICTIONS:	This program has been released under the GNU GPL license.
		
REVISIONS: 	27-Mar-00: Initial release v.1.0.

NOTES:		One area that could use some improvement is in the
		function:

		check_header_value

		The check_header_value could use some more error checking.

DOWNLOADING:	The master ftp archive for new versions is: 
		ftp://ftp.thedeepsky.com/outgoing/ 

GOALS:		Initially, making a higly accurate mp3 analysis tool is the
		goal, however, building a solid framework for a command-line
		mp3 player is a possibility. I wanted clean, readable code
		so it is easily expandable to do a variety of things. With
		readable code, more people can follow my logic, and find
		improvements quicker and easier.


**********************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "mp3_check.h"

// FUNCTION PROTOTYPES HAPPEN HERE 
extern	int		parse_args(char **argv, meta_options *flag_options, command_flags *flags);
extern	void		print_usage(void);
extern	void            init_command_flags_struct(command_flags *flags);
extern	void		print_sys_usage(void);

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int ch;

	int got_error = 0;
	command_flags flags;
	meta_options flag_options;

	init_command_flags_struct(&flags);

	while ((ch = getopt(argc, argv, "q:epv::ahs::ib::")) != -1)
		switch (ch) {
			case 'q':
				// Determine a mininum number of
				// contiguous frames that must be
				// present for an error not to occur.
				// Reason why is that mp3 headers are very
				// easy to generate, and often this is
				// the best way to see if it is actually
				// an mp3 and not some random data.

				// The 'q' is for se'q'uence.
	
				// NOTE TO SELF: Why isn't the single ':' forcing 
				// an option to be there? Weird.	

				if (atoi(optarg) > 1) {
					flag_options.min_frame_seq = atoi(optarg);	
					flags.qflag = TRUE;
				
				} else {
					(void)fprintf(stderr, "You must supply an integer greater than 1 to use the '-q' flag.");
					
					return(FAIL - 1);						
				}

				break;
			case 'e':
				// Record an invalid frame
				// when an id3v2 tag is found.
				flags.eflag = TRUE;
	
				break;
			case 'p':
				// Single-line output only 
				// for easier parsing.
				flags.pflag = TRUE;
				
				break;
			case 'v':
				//  Verbose 
				flags.vflag = TRUE;

				//  Be very verbose - every pice of cool 
				//  info about each frame is shot to the screen 
				if (optarg && *optarg == 'v')
					flags.vvflag = TRUE;
	
				break;

			case 'a':
				//  Check the whole mp3 
				flags.aflag = TRUE;
				break;
				
			case 's':
				//  Send mp3 to stdout 
				flags.sflag = TRUE;

				//  Be betty quiet. I do not want any 
				//  output unless it is an error. 

				if (optarg && *optarg == 's')
					flags.ssflag = TRUE;

				if (optarg && *optarg == 'f') {
					flags.fflag = TRUE;
					//flags.sflag = FALSE;
				}

				break;
				
			case 'i':
				//  Check for that pesky id3 tag. It is going 
				//  to run a LOT slower with this on. 
				flags.iflag = TRUE;
				break;
				
			case 'b':
				//  Only scan the first # of bytes, then quit. 
				if (optarg)
					flag_options.byte_limit = atoi(optarg);
	
				flags.bflag = TRUE;
				break;
				
			case 'h':
				//  What? You need the help message? 
				print_usage();
				break;
				
			default:
				print_usage();
				break;
		}

	argv += optind;

	//
	// If there's no data after the flags, return an error.
	//
	if (*argv == NULL) {
		print_usage();
	}


	//  
	//  Required fields go here. Everything 
	//  else not listed here are optional. 
	//  
	//  The below 'if' statement seems a little silly, 
	//  but it works. I will clean it up a little later. 
	//  
	if (!(flags.vflag || flags.aflag || flags.bflag))
		flags.aflag = TRUE;

	if (!flags.aflag)
		flags.bflag = TRUE;


	// parse_args is where the program begins.
	// everything else stems from it.
	if (!parse_args(argv, &flag_options, &flags))
		got_error++;

	if (!flags.sflag)
		print_sys_usage();


	//
	// At least in bash, I have to return a '0' to indicate
	// a success, while in C, a success is typically a '1'.
	//
	if (got_error > 0)
		return(FAIL - 1);
	else
		return(PASS - 1);

} // MAIN 
