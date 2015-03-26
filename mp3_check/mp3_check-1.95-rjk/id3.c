#include "mp3_check.h"

#include <stdio.h>


static int get_last_char_offset(char *fat_string);
static int copy_int_array_to_str(char *possible_mp3_tag, char *tag_field,
				 int offset, int length, int max_length);


static int get_last_char_offset(char *fat_string)
{
	int i = 0;

	for (i = strlen(fat_string) - 1; i >= 0; i--) {
		if (ISGRAPH((int) *(fat_string + i)))
			return (i + 1);
	}

	return (strlen(fat_string));
}

int dump_id3_tag(id3_tag_info * id3_tag)
{
	char title[30];
	char artist[30];
	char album[30];
	char year[4];
	char comment[30];
	int i = 0;

	memset(title, '\0', sizeof(title));
	memset(artist, '\0', sizeof(artist));
	memset(album, '\0', sizeof(album));
	memset(year, '\0', sizeof(year));
	memset(comment, '\0', sizeof(comment));

	/*
	 * The rules state, that nulls should pad the fields,
	 * so, that is what I am doing here.
	 */
	strncpy(title, id3_tag->TITLE, get_last_char_offset(id3_tag->TITLE));
	strncpy(artist, id3_tag->ARTIST,
		get_last_char_offset(id3_tag->ARTIST));
	strncpy(album, id3_tag->ALBUM, get_last_char_offset(id3_tag->ALBUM));
	strncpy(year, id3_tag->YEAR, get_last_char_offset(id3_tag->YEAR));
	strncpy(comment, id3_tag->COMMENT,
		get_last_char_offset(id3_tag->COMMENT));


	/*
	 * Now, I am going to send the null-pointed fields to the file pointer.
	 * This below is not very elegant. It works though,
	 * so I will revisit this at a later time.
	 *
	 * The problem is that I need to print the null values,
	 * if I didn't need to do that, it would be much cleaner.
	 */
	fputs("TAG", stdout);

	for (i = 0; i < 30; i++)
		fputc(*(title + i), stdout);

	for (i = 0; i < 30; i++)
		fputc(*(artist + i), stdout);

	for (i = 0; i < 30; i++)
		fputc(*(album + i), stdout);

	for (i = 0; i < 4; i++)
		fputc(*(year + i), stdout);

	/* It's 29 because the last byte may be the track number. */
	for (i = 0; i < 29; i++)
		fputc(*(comment + i), stdout);

	if (id3_tag->TRACK_NUMBER != 0)
		fputc(id3_tag->TRACK_NUMBER, stdout);
	else
		fputc(*(comment + 29), stdout);

	fputc(id3_tag->GENRE, stdout);

	return (TRUE);
}

static int copy_int_array_to_str(char *possible_mp3_tag, char *tag_field,
				 int offset, int length, int max_length)
{
	int counter = 0;
	int position = 0;

	if (max_length < (offset + length))
		return (FAIL);

	for (position = offset; position < (offset + length); position++) {
		*(tag_field + counter) = (char) *(possible_mp3_tag + position);
		counter++;
	}

	return (PASS);
}

int validate_id3_tag(char *possible_mp3_tag, id3_tag_info * id3_tag)
{
	/* The first 3 characters must be "TAG" for it to be an id3 tag. */

	/*
	 * This section is just to see the 128 bit buffer in numerical format
	 */
	if ((possible_mp3_tag[0] == 'T') && (possible_mp3_tag[1] == 'A')
	    && (possible_mp3_tag[2] == 'G')) {
		id3_tag->TAG_PRESENT = YES;

		if (possible_mp3_tag[126] != '\0') {
			id3_tag->TRACK_NUMBER = possible_mp3_tag[126];
			id3_tag->ID3_311_VERSION = YES;
		} else {
			id3_tag->TRACK_NUMBER = 0;
			id3_tag->ID3_311_VERSION = NO;
		}


		id3_tag->GENRE = possible_mp3_tag[127];

		copy_int_array_to_str(possible_mp3_tag, id3_tag->TITLE, 3, 30,
				      128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->ARTIST, 33,
				      30, 128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->ALBUM, 63, 30,
				      128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->YEAR, 93, 4,
				      128);
		copy_int_array_to_str(possible_mp3_tag, id3_tag->COMMENT, 97,
				      30, 128);

#if 0
		/*
		 * This section below does not work because I am working with
		 * an integer array and not a character array. The reason for
		 * that is that with a character array, if there are any
		 * nulls, the array shortens and I need the (null)s to find
		 * out certain info... 
		 */
		memcpy(id3_tag->TITLE, possible_mp3_tag + 3, 30);
		memcpy(id3_tag->ARTIST, possible_mp3_tag + 33, 30);
		memcpy(id3_tag->ALBUM, possible_mp3_tag + 63, 30);
		memcpy(id3_tag->YEAR, possible_mp3_tag + 93, 4);
		memcpy(id3_tag->COMMENT, possible_mp3_tag + 97, 30);
#endif
	} else {
		id3_tag->TAG_PRESENT = NO;
		return (FAIL);
	}

	return (PASS);
}
