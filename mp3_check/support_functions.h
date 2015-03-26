/***************************************************
*
* These functions below rarely get changed, so
* I'm going to put them here so there's less
* clutter in the main C file.
*
****************************************************/

// Function prototypes go here
inline int	cmp_str(const char *full_str, const char *chk_str, int full_str_offset);
int		check_header_value(unsigned int *header, char *filename, frame_info *FI);
int		check_vbr_and_time(frame_info *mp3_i, vbr_data *vbr_info, gen_info *file_info);
int		copy_int_array_to_str(char *possible_mp3_tag, char *tag_field, int offset, int length, int max_length);
int		dump_id3_tag(id3_tag_info *id3_tag);
int		get_last_char_offset(char *fat_string);
int		print_frame_info(frame_info *mp3_i, gen_info *file_info);
int		rotate_char_array(char *byte_list, int *new_byte, gen_info *file_info);
int		transform_char_array(char *byte_list, gen_info *file_info);
int		validate_id3_tag(char *possible_mp3_tag, id3_tag_info *id3_tag);
void		init_id3_tag_struct(id3_tag_info *TAG);
void		init_vbr_tag_struct(vbr_data *vbr_info);
void		init_mp3_time_struct(mp3_time *song_time);
void		init_gen_info_struct(gen_info *file_info);
void		print_sys_usage(void);
void		translate_time(gen_info *file_info, mp3_time *song_time);


void
init_vbr_tag_struct(vbr_info)
	vbr_data	*vbr_info;
{
	vbr_info->high_rate	= 0;
	vbr_info->low_rate	= 0;
	vbr_info->ave_rate	= 0;
	vbr_info->sum_rate	= 0;
}

void
init_gen_info_struct(file_info)
	gen_info	*file_info;
{
	file_info->file_pos 		= 0;
	file_info->good_frame_count	= 0;
	file_info->bad_frame_count	= 0;
	file_info->frame_sequence_count	= 0;
	file_info->byte_count		= 0;
	file_info->next_expected_frame	= 4;
	file_info->time_in_seconds 	= 0;
}

void
init_mp3_time_struct(song_time)
	mp3_time	*song_time;
{
	song_time->minutes	= 0;
	song_time->seconds	= 0;
	song_time->frac_second	= 0;
}
	
inline int
cmp_str (full_str, chk_str, full_str_offset)
	register const char * full_str;
	register const char * chk_str;
	int full_str_offset;
{
	int     check_state     = FAIL;
	int     start           = 31 - full_str_offset; /* ?? */

	if (!memcmp (full_str+start, chk_str, strlen(chk_str)))
		return PASS;

	return check_state;
}

int
copy_int_array_to_str(possible_mp3_tag, tag_field, offset, length, max_length)
	char		*possible_mp3_tag;
	char		*tag_field;
	int		offset;
	int		length;
	int		max_length;
{
	int 	counter = 0;
	int 	position = 0;

	if (max_length < (offset + length))
		return(FAIL);

	for (position = offset ; position < (offset + length) ; position++) {
		*(tag_field + counter) = (char)*(possible_mp3_tag + position);
		counter++;
	}

	return(PASS);
}	

int
validate_id3_tag(possible_mp3_tag, id3_tag)
	char		*possible_mp3_tag;
	id3_tag_info 	*id3_tag;
{
	//  The first 3 characters must be "TAG" for it 
	//  to be an id3 tag. 

	//  
	//  This section is just to see the 128 bit buffer in numerical format 
	//  
	
	if ((possible_mp3_tag[0] == 'T') && (possible_mp3_tag[1] == 'A') && (possible_mp3_tag[2] == 'G')) {
		id3_tag->TAG_PRESENT = YES;

		if (possible_mp3_tag[126] != '\0') {
			id3_tag->TRACK_NUMBER = possible_mp3_tag[126];
			id3_tag->ID3_311_VERSION = YES;
		} else {
			id3_tag->TRACK_NUMBER = 0;
			id3_tag->ID3_311_VERSION = NO;
		}
			

		id3_tag->GENRE = possible_mp3_tag[127];

		copy_int_array_to_str(possible_mp3_tag, id3_tag->TITLE, 3, 30, 128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->ARTIST, 33, 30, 128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->ALBUM, 63, 30, 128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->YEAR, 93, 4, 128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->COMMENT, 97, 30, 128);


		//  This section below does not work because I am working with an 
		//  integer array and not a character array. The reason for that is that 
		//  with a character array, if there are any nulls, the array shortens and 
		//  I need the (null)s to find out certain info... 
		
		// memcpy(id3_tag->TITLE, possible_mp3_tag + 3, 30); 
		// memcpy(id3_tag->ARTIST, possible_mp3_tag + 33, 30); 
		// memcpy(id3_tag->ALBUM, possible_mp3_tag + 63, 30); 
		// memcpy(id3_tag->YEAR, possible_mp3_tag + 93, 4); 
		// memcpy(id3_tag->COMMENT, possible_mp3_tag + 97, 30); 
	} else {
		id3_tag->TAG_PRESENT = NO;
		return(FAIL);
	}

	return(PASS);
}

void
init_id3_tag_struct(TAG)
	id3_tag_info *TAG;
{
	TAG->TAG_PRESENT = NO;
	TAG->ID3_311_VERSION = NO;

	memset(TAG->TITLE, '\0', sizeof(TAG->TITLE));
	memset(TAG->ARTIST, '\0', sizeof(TAG->ARTIST));
	memset(TAG->ALBUM, '\0', sizeof(TAG->ALBUM));
	memset(TAG->YEAR, '\0', sizeof(TAG->YEAR));
	memset(TAG->COMMENT, '\0', sizeof(TAG->COMMENT));

	TAG->GENRE = 0;
	TAG->TRACK_NUMBER = 0;
	TAG->COMPLIANT_PAD_FIELDS = NO;
}

int
check_header_value(header, filename, FI)
	unsigned int	*header;
	char		*filename;
	frame_info	*FI;
{
	//  
	//  This is the one function that truely will make this a  
	//  bad-ass proggie. Lots of logic can go in here to check 
	//  for compliant mp3s. 
	//  

	char	bin_string[33] = {0};
	int	i = 0;
	int	value_part = 0;
	int	column_part = 0;


	//  The last column makes it easy to trap the 'everything else' stuff... 
	static int bitrate_matrix[6][16] = {
		{1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
		{1, 32, 48, 56, 64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 384, 0},
		{1, 32, 40, 48, 56,  64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 0},
		{1, 32, 48, 56, 64,  80,  96,  112, 128, 144, 160, 176, 192, 224, 256, 0},
		{1, 8,	16, 24, 32,  40,  48,  56,  64,	 80,  96,  112, 128, 144, 160, 0},
		{0, 0,	0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
	};

	static int sampling_rate_matrix[4][3] = {
		{44100,	22050,	11025},
		{48000,	24000,	12000},
		{32000,	16000,	8000},
		{0,	0,	0}
	};

	//  For debugging purposes 
	// printf("HV: %d\n", *header); 

	//  
	//  Set the default return value of this funtion to FAIL 
	//  in case of a invalid header. 
	//  
	FI->check_state = FAIL;

	bin_string[32] = '\0';

	for (i=0 ; i < 32 ; i++) {
		bin_string[i] = '0';
	}


	//  
	//  Convert 32bit integer to binary. 
	//  
	for (i=0 ; i < 32 ; i++){
		if (((*header >> i) & 1) == 1){
			bin_string[31 - i] = '1';
		}
	}

	//  Store this data in case I want to print it 
	//  out for debugging stuff. 

	strcpy(FI->BIN_STRING, bin_string);
	FI->INT_HEADER = *header;

	//  
	//  Is it a mp3 sync bit? 
	//  
	//  Don't even continue if it doesn't pass this one. 
	//  

	if (cmp_str(bin_string, "11111111111", 31))
		FI->check_state = PASS;
	else
		return FI->check_state;


	//  
	//  Get the MPEG version 
	//  
	if (cmp_str(bin_string, "00", 20))
		FI->MPV_25 = TRUE;

	else if (cmp_str(bin_string, "01", 20))
		FI->MPV_RESERVED = TRUE;

	else if (cmp_str(bin_string, "10", 20))
		FI->MPV_2 = TRUE;

	else if (cmp_str(bin_string, "11", 20))
		FI->MPV_1 = TRUE;


	//  
	//  Layer Version 
	//  
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


	//  
	//  CRC Bit 
	//  
	if (cmp_str(bin_string, "0", 16))
		FI->PROT_BIT = TRUE;


	//  
	//  BITRATE 
	//  
	//  Grab the specific integer from the "header" 
	//  that I need. 
	//  
	//  Truncate at the 12th bit (0xf), and shift right 12 bits 
	//  to just get bitrange 15 -> 12. 

	value_part = ((*header >> 12 ) & 0xf);

	//  Column 5 is for everything else (junk) 
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


	//  
	//  Sampling Rate 
	//  
	//  Truncate at the 2nd bit (0x3), and shift right 9 bits 
	//  to just get bitrange 11 -> 10. 

	value_part = ((*header >> 10) & 0x3);

	if (FI->MPV_1)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][0];
		
	else if (FI->MPV_2)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][1];
		
	else if (FI->MPV_25)
		FI->SAMPLE_FREQ = sampling_rate_matrix[value_part][2];

	else
		FI->SAMPLE_FREQ = 0;


	//  
	//  Padding Bit 
	//  
	if (cmp_str(bin_string, "1", 9))
		FI->PAD_BIT = TRUE;


	//  
	//  Private Bit 
	//  
	if (cmp_str(bin_string, "1", 8))
		FI->PRIV_BIT = TRUE;


	//  
	//  Get the channel mode 
	//  
	//  The 'mode_extension' will be added later 
	//  perhaps in V2.0. 
	//  
	if (cmp_str(bin_string, "00", 7))
		FI->STEREO = TRUE;

	else if (cmp_str(bin_string, "01", 7)) 
		{
		FI->JOINT_STEREO = TRUE;
		FI->MODE_EXTENSION = TRUE;
		}

	else if (cmp_str(bin_string, "10", 7))
		FI->DUAL_STEREO = TRUE;

	else if (cmp_str(bin_string, "11", 7))
		FI->SINGLE_CHANNEL = TRUE;


	//  
	//  Copyright Bit 
	//  
	if (cmp_str(bin_string, "1", 3))
		FI->COPYRIGHT = TRUE;


	//  
	//  Original Bit 
	//  
	if (cmp_str(bin_string, "1", 2))
		FI->ORIGINAL = TRUE;


	//  
	//  Emphasis 
	//  
	if (cmp_str(bin_string, "00", 1))
		FI->EMPH_NONE = TRUE;

	else if (cmp_str(bin_string, "01", 1))
		FI->EMPH_5015 = TRUE;

	else if (cmp_str(bin_string, "10", 1))
		FI->EMPH_RESERV = TRUE;

	else if (cmp_str(bin_string, "11", 1))
		FI->EMPH_CCIT = TRUE;


	//  
	//  FRAME LENGTH 
	//  
	if (FI->SAMPLE_FREQ > 0) {
		if (FI->MPV_1) {
			if (FI->L1 && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH = (12 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ + 4) * 4;

			} else if (FI->L1 && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH = (12 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ) * 4;

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH = (144 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ + 1);

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH = (144 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ);
			}
		} else if (FI->MPV_2 || FI->MPV_25) {
			if (FI->L1 && FI->PAD_BIT == TRUE) {
				FI->FRAME_LENGTH = (240 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ + 4) * 4;

			} else if (FI->L1 && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH = (240 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ) * 4;

			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == TRUE) { 
				FI->FRAME_LENGTH = (72 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ + 1);
		
			} else if ((FI->L2 || FI->L3) && FI->PAD_BIT == FALSE) {
				FI->FRAME_LENGTH = (72 * FI->BIT_RATE * 1000 / FI->SAMPLE_FREQ);
			}
		}
	} else
		FI->FRAME_LENGTH = 0;



	//  
	//  ERROR TRAPPING HAPPENS HERE. 
	//  IN FUTURE VERSIONS, I WILL CHECK FOR 
	//  JOINT STEREO and INTENSITY STEREO. 
	//  
	if (FI->L_RESERVED || FI->BIT_RATE == 0 || FI->BIT_RATE == 1 || FI->SAMPLE_FREQ == 0)
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

	else if (FI->L2 && FI->BIT_RATE == 224 && !(FI->STEREO || FI->DUAL_STEREO))
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 256 && !(FI->STEREO || FI->DUAL_STEREO))
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 320 && !(FI->STEREO || FI->DUAL_STEREO))
		FI->check_state = FAIL;

	else if (FI->L2 && FI->BIT_RATE == 384 && !(FI->STEREO || FI->DUAL_STEREO))
		FI->check_state = FAIL;


	return FI->check_state;
}

void
print_sys_usage(void)
{
	struct	rusage	process_usage;

	if (getrusage(RUSAGE_SELF, &process_usage))
		(void)fprintf(stderr, "Could not get system usage for myself.");
	else {
		printf("\n");
		printf("%-20s%1ld.%02lds\n", "USER_TIME", process_usage.ru_utime.tv_sec, process_usage.ru_utime.tv_usec / 10000);
		printf("%-20s%1ld.%02lds\n", "SYS_TIME", process_usage.ru_stime.tv_sec, process_usage.ru_stime.tv_usec / 10000);
	}
}

int
rotate_char_array(byte_list, new_byte, file_info)
	char	*byte_list;
	int	*new_byte;
	gen_info *file_info;
{
	int	place_holder = 0;

	//  I don't think it'll get any faster than this. 
	place_holder = file_info->byte_count % 128;

	*(byte_list + place_holder) = *new_byte;
	
	return(TRUE); 
}

int
transform_char_array(byte_list, file_info)
	char	*byte_list;
	gen_info *file_info;
{
	int	place_holder = 0;
	int	counter;
	char	trans_list[128] = {'*'};

	place_holder = file_info->byte_count % 128 + 1;

	//  This is going to be a little confusing 
	//  at the 'place_holder' mark, the data from place_holder to 127  
	//  is the oldest, and data from 0 to place_holder is newest 

	//  Get the oldest stuff first, and put it at the beginning 
	//  of the array. 
	for(counter = place_holder; counter < 128; counter++)
		*(trans_list + (counter - place_holder)) = *(byte_list + counter); 	

	// Now, grab the newest stuff, and slap it at the end of the array. 
	for(counter = 0; counter < place_holder; counter++)
		*(trans_list + (128 - place_holder + counter)) = *(byte_list + counter); 

	memcpy(byte_list, trans_list, 128);

	return(TRUE);
}

void
translate_time(file_info, song_time)
	mp3_time 	*song_time;
	gen_info	*file_info;
{
	double float_minute = 0.0;
	double float_second = 0.0;

	float_minute = file_info->time_in_seconds / 60.0;
	float_second = 60.0 * (float_minute - (int)float_minute);

	//  Return the values. 
	song_time->frac_second = 100.0 * (float_second - (int)float_second);
	song_time->minutes = (int)float_minute;
	song_time->seconds = (int)float_second;
}

int
dump_id3_tag(id3_tag)
	id3_tag_info    *id3_tag;	
{
	char	title[30];
	char	artist[30];
	char	album[30];
	char	year[4];
	char	comment[30];
	int	i = 0;


	memset(title, '\0', sizeof(title));
	memset(artist, '\0', sizeof(artist));
	memset(album, '\0', sizeof(album));
	memset(year, '\0', sizeof(year));
	memset(comment, '\0', sizeof(comment));

	// The rules state, that nulls should pad the fields,
	// so, that is what I am doing here.

	strncpy(title, id3_tag->TITLE, get_last_char_offset(id3_tag->TITLE));
	strncpy(artist, id3_tag->ARTIST, get_last_char_offset(id3_tag->ARTIST));
	strncpy(album, id3_tag->ALBUM, get_last_char_offset(id3_tag->ALBUM));
	strncpy(year, id3_tag->YEAR, get_last_char_offset(id3_tag->YEAR));
	strncpy(comment, id3_tag->COMMENT, get_last_char_offset(id3_tag->COMMENT));	


	// Now, I am going to send the null-pointed fields to the file pointer.
	// This below is not very elegant. It works though,
	// so I will revisit this at a later time.
	//
	// The problem is that I need to print the null values,
	// if I didn't need to do that, it would be much cleaner.

	
	fputs("TAG", stdout);

	for(i=0; i < 30; i++)
		fputc(*(title + i), stdout);

	for(i=0; i < 30; i++)
		fputc(*(artist + i), stdout);

	for(i=0; i < 30; i++)
		fputc(*(album + i), stdout);

	for(i=0; i < 4; i++)
		fputc(*(year + i), stdout);


	// It's 29 because the last byte may
	// be the track number.

	for(i=0; i < 29; i++)
		fputc(*(comment + i), stdout);

	if(id3_tag->TRACK_NUMBER != 0)	
		fputc(id3_tag->TRACK_NUMBER, stdout);
	else
		fputc(*(comment + 29), stdout);		


	fputc(id3_tag->GENRE, stdout);

	return(TRUE);
}	

int
get_last_char_offset(fat_string)
	char *fat_string;
{
	int	i = 0;

	for (i = strlen(fat_string) - 1 ; i >= 0 ; i--) {
		if (isgraph((int)*(fat_string + i)))
			return(i + 1);	
	}

	return(strlen(fat_string));
}

int
check_vbr_and_time(mp3_i, vbr_info, file_info)
	frame_info	*mp3_i;
	vbr_data	*vbr_info;
	gen_info	*file_info;
{
	//  
	//  Lets get the time of this frame, and 
	//  add it to the total length of the mp3 file 
	//  
	file_info->time_in_seconds += (mp3_i->SAMPLES_PER_FRAME * 1.0) / (mp3_i->SAMPLE_FREQ * 1.0);

	//  
	//  Time to do some nifty VBR checking. 
	//  
	if (vbr_info->high_rate < mp3_i->BIT_RATE)
		vbr_info->high_rate = mp3_i->BIT_RATE;

	if ((mp3_i->BIT_RATE < vbr_info->low_rate) || (vbr_info->low_rate == 0))
		vbr_info->low_rate = mp3_i->BIT_RATE;


	//  Get a running total for division at the end. 
	vbr_info->sum_rate += mp3_i->BIT_RATE;

	return(TRUE);
}

int
print_frame_info(mp3_i, file_info)
	frame_info	*mp3_i;
	gen_info	*file_info;
{
	printf("\n");
	printf("%-20s%d\n", "FRAME", file_info->good_frame_count);
	printf("%-20s%d\n", "FrameOffset", file_info->byte_count);
	printf("%-20s%d\n", "FrameLength", mp3_i->FRAME_LENGTH);
	printf("%-20s%d\n", "FrameDataLength", mp3_i->FRAME_DATA_LENGTH);
	printf("%-20s%d\n", "BitRate", mp3_i->BIT_RATE);
	printf("%-20s%d\n", "SampRate", mp3_i->SAMPLE_FREQ);
	printf("%-20s%s\n", "BinString", mp3_i->BIN_STRING);
	printf("%-20s%d\n", "BinLen", strlen(mp3_i->BIN_STRING));
	printf("%-20s%u\n", "IntHeader", mp3_i->INT_HEADER);
	printf("%-20s0x%x\n", "CRC16Value", mp3_i->CRC16_VALUE);
	printf("%-20s0x%x\n", "Correct_CRC16Value", mp3_i->CORRECT_CRC16_VALUE);
	printf("%-20s%d\n", "NEXT_FRAME", file_info->byte_count + mp3_i->FRAME_LENGTH);

	return(TRUE);
}
