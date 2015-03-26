#include "mp3_check.h"

#include <stdio.h>
#include <sys/resource.h>
#include <sysexits.h>

void print_usage(void)
{
	printf(_("Usage: %s [OPTION]... [FILE]\n"), PACKAGE);
	printf(_("\
Options:\n\
  -q number         Determines the minimum <number> of continuous frames\n\
                    that must be present for an error NOT to occur.\n\
                    Analysing random data will find spurious MP3 headers,\n\
                    but finding <number> sequential headers are even hard to\n\
                    find.\n\
  -e                While id3v2 support is under development, this flag will\n\
                    allow you to have mp3_check record a frame error when a\n\
                    id3v2 tag is found.\n\
  -p                Shows just the essentials (what you get without the -v\n\
                    option) on a single line for easy parsing. This fields\n\
                    are separated by tabs, and the name/value pairs are\n\
                    separated by spaces.\n\
  -v[v]             Lists details about the MP3 in name->value order. Adding\n\
                    an extra 'v' at the end gives details about each frame.\n\
                    Extremely verbose!\n\
  -a                Checks the MP3 from stem to stern (default.) If any\n\
                    other flags are present, it will not be enabled, and\n\
                    will instead behave as though the 'b' switch is present.\n\
  -b <byte_count>   If a frame is not found by <byte_count> bytes, quit and\n\
                    return an error. If <byte_count> is not specified, it\n\
                    will search until the first valid frame and quit.\n\
  -i                Check to see if there is an ID3 tag, and if so display\n\
                    the info.\n\
  -s[s|f]           Sends the MP3 to stdout (for CGI applications.) If an\n\
                    additional 's' is there, then no output is made, just\n\
                    errors are reported (super silent.) With the 'f' used,\n\
                    only valid frames are sent to stdout ('f'ixing the MP3\n\
                    is attempted.) Also, it is important to note that the\n\
                    ID3 tag will not be included in the 'f'ixed MP3. If you\n\
                    want it included, you have to include 'i' option.\n\
  -h                Print this text including the version.\n\
\n\
"));

	printf(_("* VERSION:  %s\n"), VERSION);
	printf(_("* HOMEPAGE: %s\n"), HOMEPAGE);
	printf(_("* AUTHOR:   %s\n\n\n"), AUTHOR);

	exit(EX_USAGE);
}

void print_summary(frame_info *mp3_i, char *filename)
{
	printf("\n");
	printf("%-20s%d\n", "TRUE", 1);
	printf("%-20s%d\n", "FALSE", 0);
	printf("%-20s%d\n", "MPV_1", mp3_i->MPV_1);
	printf("%-20s%d\n", "MPV_2", mp3_i->MPV_2);
	printf("%-20s%d\n", "MPV_25", mp3_i->MPV_25);
	printf("%-20s%d\n", "MPV_RESERVED", mp3_i->MPV_RESERVED);
	printf("%-20s%d\n", "L1", mp3_i->L1);
	printf("%-20s%d\n", "L2", mp3_i->L2);
	printf("%-20s%d\n", "L3", mp3_i->L3);
	printf("%-20s%d\n", "L_RESERVED", mp3_i->L_RESERVED);
	printf("%-20s%d\n", "PROT_BIT", mp3_i->PROT_BIT);
	printf("%-20s%d\n", "BIT_RATE", mp3_i->BIT_RATE);
	printf("%-20s%d\n", "SAMPLE_FREQ", mp3_i->SAMPLE_FREQ);
	printf("%-20s%d\n", "SAMPLES_PER_FRAME", mp3_i->SAMPLES_PER_FRAME);
	printf("%-20s%d\n", "PAD_BIT", mp3_i->PAD_BIT);
	printf("%-20s%d\n", "PRIV_BIT", mp3_i->PRIV_BIT);
	printf("%-20s%d\n", "STEREO", mp3_i->STEREO);
	printf("%-20s%d\n", "JOINT_STEREO", mp3_i->JOINT_STEREO);
	printf("%-20s%d\n", "DUAL_STEREO", mp3_i->DUAL_STEREO);
	printf("%-20s%d\n", "SINGLE_CHANNEL", mp3_i->SINGLE_CHANNEL);
	printf("%-20s%d\n", "MODE_EXTENSION", mp3_i->MODE_EXTENSION);
	printf("%-20s%d\n", "ID3V2", mp3_i->ID3V2);
	printf("%-20s%d\n", "FRAME_LENGTH", mp3_i->FRAME_LENGTH);
	printf("%-20s%d\n", "COPYRIGHT", mp3_i->COPYRIGHT);
	printf("%-20s%d\n", "ORIGINAL", mp3_i->ORIGINAL);
	printf("%-20s%d\n", "EMPH_NONE", mp3_i->EMPH_NONE);
	printf("%-20s%d\n", "EMPH_5015", mp3_i->EMPH_5015);
	printf("%-20s%d\n", "EMPH_RESERV", mp3_i->EMPH_RESERV);
	printf("%-20s%d\n", "EMPH_CCIT", mp3_i->EMPH_CCIT);
	printf("%-20s%d\n", "CHECK_STATE", mp3_i->check_state);
	printf("%-20s%s\n", "BIN_STRING", mp3_i->BIN_STRING);
}

void print_sys_usage(void)
{
	struct rusage process_usage;

	if (getrusage(RUSAGE_SELF, &process_usage)) {
		fprintf(stderr, "Could not get system usage for myself.");
	} else {
		printf("\n");
		printf("%-20s%1ld.%02lds\n", "USER_TIME",
		       process_usage.ru_utime.tv_sec,
		       process_usage.ru_utime.tv_usec / 10000);
		printf("%-20s%1ld.%02lds\n", "SYS_TIME",
		       process_usage.ru_stime.tv_sec,
		       process_usage.ru_stime.tv_usec / 10000);
	}
}

int print_frame_info(frame_info *mp3_i, gen_info *file_info)
{
	printf("\n");
	printf("%-20s%d\n", "FRAME", file_info->good_frame_count);
	printf("%-20s%d\n", "FrameOffset", file_info->byte_count);
	printf("%-20s%d\n", "FrameLength", mp3_i->FRAME_LENGTH);
	printf("%-20s%d\n", "BitRate", mp3_i->BIT_RATE);
	printf("%-20s%d\n", "SampRate", mp3_i->SAMPLE_FREQ);
	printf("%-20s%s\n", "BinString", mp3_i->BIN_STRING);
	printf("%-20s%d\n", "BinLen", strlen(mp3_i->BIN_STRING));
	printf("%-20s%u\n", "IntHeader", mp3_i->INT_HEADER);
	printf("%-20s0x%x\n", "CRC16Value", mp3_i->CRC16_VALUE);
	printf("%-20s%d\n", "NEXT_FRAME",
	       file_info->byte_count + mp3_i->FRAME_LENGTH);

	return (TRUE);
}
