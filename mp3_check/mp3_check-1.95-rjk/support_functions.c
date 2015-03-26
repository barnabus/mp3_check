#include "mp3_check.h"

static inline int cmp_str(const char *full_str, const char *chk_str,
			  int full_str_offset)
{
	int check_state = FAIL;
	int start = 31 - full_str_offset;	/* ?? */

	if (!memcmp(full_str + start, chk_str, strlen(chk_str)))
		return PASS;

	return check_state;
}

/* 
 * This is the one function that truely will make this a bad-ass proggie.
 * Lots of logic can go in here to check for compliant mp3s. 
 */
int check_header_value(unsigned int *header, char *filename, frame_info * FI)
{
 	char bin_string[33] = { 0 };
	int i = 0;
	int value_part = 0;
	int column_part = 0;


	/*
	 * The last column makes it easy to trap the 'everything else' stuff.
	 */
	static int bitrate_matrix[6][16] = {
		
		    {1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352,
		 384, 416, 448, 0},
		{1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320,
		 384, 0},
		{1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256,
		 320, 0},
		{1, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224,
		 256, 0},
		{1, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,
		 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};

	static int sampling_rate_matrix[4][3] = {
		{44100, 22050, 11025},
		{48000, 24000, 12000},
		{32000, 16000, 8000},
		{0, 0, 0}
	};

#ifdef DEBUG
	printf("HV: %d\n", *header);
#endif

	/* 
	 * Set the default return value of this funtion to FAIL in case of a
	 * invalid header.
	 */ 
	FI->check_state = FAIL;

	bin_string[32] = '\0';

	for (i = 0; i < 32; i++) {
		bin_string[i] = '0';
	}


	/* Convert 32bit integer to binary. */
	for (i = 0; i < 32; i++) {
		if (((*header >> i) & 1) == 1) {
			bin_string[31 - i] = '1';
		}
	}

	/*
	 * Store this data in case I want to print it out for debugging stuff.
	 */
	strcpy(FI->BIN_STRING, bin_string);
	FI->INT_HEADER = *header;

	/*
	 * Is it a mp3 sync bit? 
	 *  
	 * Don't even continue if it doesn't pass this one. 
	 */ 
	if (cmp_str(bin_string, "11111111111", 31))
		FI->check_state = PASS;
	else
		return FI->check_state;


	/* Get the MPEG version */
	if (cmp_str(bin_string, "00", 20))
		FI->MPV_25 = TRUE;

	else if (cmp_str(bin_string, "01", 20))
		FI->MPV_RESERVED = TRUE;

	else if (cmp_str(bin_string, "10", 20))
		FI->MPV_2 = TRUE;

	else if (cmp_str(bin_string, "11", 20))
		FI->MPV_1 = TRUE;


	/* Layer Version */ 
	if (cmp_str(bin_string, "00", 18)) {
		FI->L_RESERVED = TRUE;
		FI->SAMPLES_PER_FRAME = 0;

	} else if (cmp_str(bin_string, "01", 18)) {
		FI->L3 = TRUE;
		FI->SAMPLES_PER_FRAME = 1152;

	} else if (cmp_str(bin_string, "10", 18)) {
		FI->L2 = TRUE;
		FI->SAMPLES_PER_FRAME = 1152;

	} else if (cmp_str(bin_string, "11", 18)) {
		FI->L1 = TRUE;
		FI->SAMPLES_PER_FRAME = 384;
	}

	/* CRC Bit */ 
	if (cmp_str(bin_string, "0", 16))
		FI->PROT_BIT = TRUE;


	/* 
	 * BITRATE 
	 *  
	 * Grab the specific integer from the "header" that I need. 
	 * 
	 * Truncate at the 12th bit (0xf), and shift right 12 bits to just
	 * get bitrange 15 -> 12. 
	 */
	value_part = ((*header >> 12) & 0xf);

	/* Column 5 is for everything else (junk) */
	if (FI->MPV_1 && FI->L1)
		column_part = 0;

	else if (FI->MPV_1 && FI->L2)
		column_part = 1;

	else if (FI->MPV_1 && FI->L3)
		column_part = 2;

	else if ((FI->MPV_2 || FI->MPV_25) && FI->L1)
		column_part = 3;

	else if ((FI->MPV_2 || FI->MPV_25) && (FI->L2 || FI->L3))
		column_part = 4;

	else
		column_part = 5;


	FI->BIT_RATE = bitrate_matrix[column_part][value_part];


	/*
	 * Sampling Rate 
	 *  
	 * Truncate at the 2nd bit (0x3), and shift right 9 bits to just get
	 * bitrange 11 -> 10. 
	 */
	value_part = ((*header >> 10) & 0x3);

	if (FI->MPV_1)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][0];

	else if (FI->MPV_2)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][1];

	else if (FI->MPV_25)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][2];

	else
		FI->SAMPLE_FREQ = 0;


	/* Padding Bit */ 
	if (cmp_str(bin_string, "1", 9))
		FI->PAD_BIT = TRUE;


	/* Private Bit */
	if (cmp_str(bin_string, "1", 8))
		FI->PRIV_BIT = TRUE;


	/*
	 * Get the channel mode 
	 *  
	 * The 'mode_extension' will be added later perhaps in V2.0. 
	 */
	if (cmp_str(bin_string, "00", 7))
		FI->STEREO = TRUE;

	else if (cmp_str(bin_string, "01", 7)) {
		FI->JOINT_STEREO = TRUE;
		FI->MODE_EXTENSION = TRUE;
	}

	else if (cmp_str(bin_string, "10", 7))
		FI->DUAL_STEREO = TRUE;

	else if (cmp_str(bin_string, "11", 7))
		FI->SINGLE_CHANNEL = TRUE;


	/* Copyright Bit */ 
	if (cmp_str(bin_string, "1", 3))
		FI->COPYRIGHT = TRUE;


	/* Original Bit */ 
	if (cmp_str(bin_string, "1", 2))
		FI->ORIGINAL = TRUE;


	/* Emphasis */ 
	if (cmp_str(bin_string, "00", 1))
		FI->EMPH_NONE = TRUE;

	else if (cmp_str(bin_string, "01", 1))
		FI->EMPH_5015 = TRUE;

	else if (cmp_str(bin_string, "10", 1))
		FI->EMPH_RESERV = TRUE;

	else if (cmp_str(bin_string, "11", 1))
		FI->EMPH_CCIT = TRUE;


	/* FRAME LENGTH */ 
	if (FI->SAMPLE_FREQ > 0) {
		if (FI->MPV_1) {
			if (FI->L1 && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH =
				    (12 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ + 4) * 4;

			} else if (FI->L1 && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH =
				    (12 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ) * 4;

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH =
				    (144 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ + 1);

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH =
				    (144 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ);
			}
		} else if (FI->MPV_2 || FI->MPV_25) {
			if (FI->L1 && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH =
				    (240 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ + 4) * 4;

			} else if (FI->L1 && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH =
				    (240 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ) * 4;

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH =
				    (72 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ + 1);

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH =
				    (72 * FI->BIT_RATE * 1000 /
				     FI->SAMPLE_FREQ);
			}
		}
	} else
		FI->FRAME_LENGTH = 0;



	/*
	 * ERROR TRAPPING HAPPENS HERE. IN FUTURE VERSIONS, I WILL CHECK FOR
	 * JOINT STEREO and INTENSITY STEREO.
	 */
	if (FI->L_RESERVED || FI->BIT_RATE == 0 || FI->BIT_RATE == 1
	    || FI->SAMPLE_FREQ == 0)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 32 && FI->SINGLE_CHANNEL == FALSE)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 48 && FI->SINGLE_CHANNEL == FALSE)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 56 && FI->SINGLE_CHANNEL == FALSE)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 80 && FI->SINGLE_CHANNEL == FALSE)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 32 && FI->SINGLE_CHANNEL == FALSE)
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 224
		 && !(FI->STEREO || FI->DUAL_STEREO)) FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 256
		 && !(FI->STEREO || FI->DUAL_STEREO)) FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 320
		 && !(FI->STEREO || FI->DUAL_STEREO)) FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 384
		 && !(FI->STEREO || FI->DUAL_STEREO)) FI->check_state = FAIL;


	return FI->check_state;
}

int transform_char_array(char *byte_list, gen_info *file_info)
{
	int place_holder = 0;
	int counter;
	char trans_list[128] = { '*' };

	place_holder = file_info->byte_count % 128 + 1;

	/*
	 * This is going to be a little confusing at the 'place_holder' mark,
	 * the data from place_holder to 127 is the oldest, and data from 0
	 * to place_holder is newest 
	 * 
	 * Get the oldest stuff first, and put it at the beginning of the
	 * array.
	 */
	for (counter = place_holder; counter < 128; counter++)
		*(trans_list + (counter - place_holder)) =
		    *(byte_list + counter);

	/* Now, grab the newest stuff, and slap it at the end of the array. */
	for (counter = 0; counter < place_holder; counter++)
		*(trans_list + (128 - place_holder + counter)) =
		    *(byte_list + counter);

	memcpy(byte_list, trans_list, 128);

	return (TRUE);
}

void translate_time(gen_info *file_info, mp3_time *song_time)
{
	double float_minute = 0.0;
	double float_second = 0.0;

	float_minute = file_info->time_in_seconds / 60.0;
	float_second = 60.0 * (float_minute - (int) float_minute);

	/* Return the values. */
	song_time->frac_second = 100.0 * (float_second - (int) float_second);
	song_time->minutes = (int) float_minute;
	song_time->seconds = (int) float_second;
}
