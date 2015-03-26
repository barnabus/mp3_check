#include "mp3_check.h"

void init_command_flags_struct(command_flags *flags)
{
	flags->aflag = FALSE;
	flags->bflag = FALSE;
	flags->eflag = FALSE;
	flags->fflag = FALSE;
	flags->iflag = FALSE;
	flags->pflag = FALSE;
	flags->qflag = FALSE;
	flags->sflag = FALSE;
	flags->ssflag = FALSE;
	flags->vflag = FALSE;
	flags->vvflag = FALSE;
}

void init_frame_struct(frame_info *FI)
{
	static frame_info empty_frame;
	static int inited = 0;

	if (!inited) {
		int i = 0;
		inited = 1;

		empty_frame.FRAME_LENGTH = 0;
		empty_frame.CRC16_VALUE = 0;
		empty_frame.BIT_RATE = 0;
		empty_frame.SAMPLE_FREQ = 0;
		empty_frame.SAMPLES_PER_FRAME = 0;
		empty_frame.MPV_1 = 0;
		empty_frame.MPV_2 = 0;
		empty_frame.MPV_25 = 0;
		empty_frame.MPV_RESERVED = 0;
		empty_frame.L1 = 0;
		empty_frame.L2 = 0;
		empty_frame.L3 = 0;
		empty_frame.L_RESERVED = 0;
		empty_frame.PROT_BIT = 0;
		empty_frame.PAD_BIT = 0;
		empty_frame.PRIV_BIT = 0;
		empty_frame.STEREO = 0;
		empty_frame.JOINT_STEREO = 0;
		empty_frame.DUAL_STEREO = 0;
		empty_frame.SINGLE_CHANNEL = 0;
		empty_frame.MODE_EXTENSION = 0;
		empty_frame.ID3V2 = 0;
		empty_frame.COPYRIGHT = 0;
		empty_frame.ORIGINAL = 0;
		empty_frame.EMPH_NONE = 0;
		empty_frame.EMPH_5015 = 0;
		empty_frame.EMPH_RESERV = 0;
		empty_frame.EMPH_CCIT = 0;

		for (i = 0; i < 32; i++)
			empty_frame.BIN_STRING[i] = '0';

		empty_frame.BIN_STRING[32] = '\0';

		empty_frame.INT_HEADER = 0;

		empty_frame.check_state = 0;
	}

	/* bitwise copy */
	*FI = empty_frame;
}

void init_vbr_tag_struct(vbr_data *vbr_info)
{
	vbr_info->high_rate = 0;
	vbr_info->low_rate = 0;
	vbr_info->ave_rate = 0;
	vbr_info->sum_rate = 0;
}

void init_gen_info_struct(gen_info *file_info)
{
	file_info->file_pos = 0;
	file_info->good_frame_count = 0;
	file_info->bad_frame_count = 0;
	file_info->frame_sequence_count = 0;
	file_info->byte_count = 0;
	file_info->next_expected_frame = 4;
	file_info->time_in_seconds = 0;
}

void init_mp3_time_struct(mp3_time *song_time)
{
	song_time->minutes = 0;
	song_time->seconds = 0;
	song_time->frac_second = 0;
}

void init_id3_tag_struct(id3_tag_info *TAG)
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
