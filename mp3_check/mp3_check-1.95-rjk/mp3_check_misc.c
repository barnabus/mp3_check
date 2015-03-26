#include "mp3_check.h"

#include <stdio.h>

static inline int move_to_next_frame(char *possible_mp3_tag,
				     frame_info * mp3_i, gen_info * file_info,
				     command_flags * flags, FILE *);
static int get_char_from_file(FILE *fp, unsigned int *header_value,
			      gen_info *file_info, command_flags *flags,
			      char *possible_mp3_tag);
static int check_vbr_and_time(frame_info *mp3_i, vbr_data *vbr_info,
			      gen_info *file_info);
static int rotate_char_array(char *byte_list, int *new_byte,
			     gen_info * info_file);
static int scan_file(FILE *fp, char *filename, meta_options *flag_options,
		     command_flags *flags);


static int rotate_char_array(char *byte_list, int *new_byte,
			     gen_info *file_info)
{
	int place_holder = 0;

	/* I don't think it'll get any faster than this. */
	place_holder = file_info->byte_count % 128;

	*(byte_list + place_holder) = *new_byte;

	return (TRUE);
}

static int scan_file(FILE *fp, char *filename, meta_options *flag_options,
		     command_flags *flags)
{
	int counter = 0;
	int END_OF_FILE = FALSE;
	int found_first_frame = FALSE;
	char possible_mp3_tag[BUFFER_LENGTH];
	unsigned int header_value = 0;

	/*
	 * This is set to 'YES' so that I will always get the first 4 bytes
	 * off the stream no matter what.
	 */
	char found_valid_header = YES;
	char found_weak_header = NO;

	/* Keep all the structures centralized. */
	frame_info mp3_i;
	vbr_data vbr_info;
	mp3_time song_time;
	frame_info first_mp3_frame;
	id3_tag_info id3_tag;
	gen_info file_info;

	/* Zero these structures out. */
	init_frame_struct(&first_mp3_frame);
	init_vbr_tag_struct(&vbr_info);
	init_gen_info_struct(&file_info);


	while (!END_OF_FILE) {
		if (flags->bflag || flags->aflag) {
			/*
			 * This is the part where if the current byte count is
			 * greater than the upper limit on the commandline, 
			 * it will break out.
			 */
			if (flags->bflag
			    && (file_info.byte_count >
				flag_options->byte_limit)) break;
		}

		/*
		 * The fist run defaults to 'YES', so the 'else' section runs
		 * first 
		 * 
		 * The logic below is what travels through the mp3 file, and
		 * then quickly does a sync checksum to see if it is a
		 * candidate.
		 */
		if (!found_valid_header) {
			/*
			 * Just add one character to the end and keep the
			 * rest. I do not want to grab another 4 bytes unless
			 * I got a success. 
			 *
			 * Putting the below action in a function slows it
			 * down quite a bit, but, it makes the code more
			 * readable.
			 */
			if (get_char_from_file
			    (fp, &header_value, &file_info, flags,
			     possible_mp3_tag)) {
				if (!flags->ssflag && flags->sflag
				    && !flags->fflag) {
					printf("%c", (header_value & 0xff));
				}
			} else
				break;
		} else {
			/* Clear it out of old values. */
			header_value = 0;

			/* Grab the complete 4-byte header (if it is one). */
			for (counter = 0; counter < 4; counter++) {
				if (get_char_from_file
				    (fp, &header_value, &file_info, flags,
				     possible_mp3_tag)) {
					if (!flags->ssflag && flags->sflag
					    && !flags->fflag) {
						printf("%c",
						       (header_value & 0xff));
					}
				} else
					break;
			}
		}


		/* Check if this is a valid frame sync. */
		if (((header_value >> 21) & 0x7ff) == 0x7ff) {
			found_weak_header = YES;

			init_frame_struct(&mp3_i);

			if (check_header_value
			    (&header_value, filename, &mp3_i)) {
				found_valid_header = YES;

				/*
				 * Was the offset of this current frame known 
				 * by the previous frame? It better have... 
				 * 
				 * This also defines junk at the beginning as
				 * being a bad frame. 
				 */ 
				if ((file_info.good_frame_count > 0)
				    && (file_info.next_expected_frame !=
					file_info.byte_count)) {
					file_info.bad_frame_count++;

					/*
					 * I guess if I get to this section,
					 * I have found an mp3 with some bad
					 * stuff in it, so I will return a
					 * fail when the program exits.
					 */
					fprintf(stderr,
						"\nAn expected frame was not found. Expected it at offset 0x%x (BYTE %d), now at offset 0x%x (BYTE %d).",
						file_info.next_expected_frame,
						file_info.next_expected_frame,
						file_info.byte_count,
						file_info.byte_count);

					/*
					 * Checking to see if a continuous
					 * amount of frames were found. This
					 * check only occurs when a bad frame
					 * was found.
					 */
					if (flags->qflag
					    && (file_info.
						frame_sequence_count > 0)
					    && (flag_options->min_frame_seq >
						file_info.
						frame_sequence_count)) {
						fprintf(stderr,
							"\nMininum contiguous number of frames wasn't reached. Got to %d, needed %d at offset 0x%x (BYTE %d).",
							file_info.
							frame_sequence_count,
							flag_options->
							min_frame_seq,
							file_info.
							next_expected_frame,
							file_info.
							next_expected_frame);

						file_info.
						    frame_sequence_count = 0;
					}

				} else if (file_info.next_expected_frame ==
					   file_info.byte_count) {
					file_info.good_frame_count++;
					file_info.frame_sequence_count++;
				} else
					printf
					    ("\nSomething happened at byte %d. Next expected frame: %d.\n",
					     file_info.byte_count,
					     file_info.next_expected_frame);


				/*
				 * The fflag is only good to use when there
				 * is *no* valid data, and not when there is
				 * good data (it should be implied). !sflag
				 * is set up this way because the data is
				 * already being shown above.
				 * 
				 * There are two parts to this header printing.
				 */
				if (!flags->ssflag &&
				    (flags->sflag && flags->fflag))
					printf("%c%c%c%c",
					       ((header_value >> 24) & 0xff),
					       ((header_value >> 16) & 0xff),
					       ((header_value >> 8) & 0xff),
					       (header_value & 0xff));

				/*
				 * I'm currently in a known good frame, so I
				 * can safely move forward two characters and
				 * still be in the frame. I will have to
				 * compensate when I move to the next frame.
				 * That is taken care of in move_to_next_frame.
				 */
				if (mp3_i.PROT_BIT) {
					/*
					 * Now, to grab the 2 bytes for the
					 * CRC value.
					 */
					for (counter = 0; counter < 2;
					     counter++) {
						if (get_char_from_file
						    (fp, &header_value,
						     &file_info, flags,
						     possible_mp3_tag)) {
							if (!flags->ssflag
							    && flags->sflag
							    && !flags->fflag) {
								printf("%c",
								       (header_value
									&
									0xff));
							}
						} else {
							break;
						}
					}

					/* Now, extract it from the header. */
					mp3_i.CRC16_VALUE =
					    header_value & 0xffff;

					/* 
					 * The fflag is only good to use when
					 * there is *no* valid data, and not
					 * when there is good data (it should
					 * be implied). !sflag is set up this
					 * way because the data is already
					 * being shown above.
					 */
					if (!flags->ssflag
					    && (flags->sflag && flags-> fflag))
						printf("%c%c",
						       ((header_value >> 8) & 0xff),
						       (header_value & 0xff));
				}

				/*
				 * If the header is valid, but has bad data, 
				 * it will still store this information.
				 * Perhaps it would be better to make sure it
				 * _really_ is a good frame before storing
				 * any VBR data...
				 */
				check_vbr_and_time(&mp3_i, &vbr_info,
						   &file_info);

				/*
				 * I always want to keep the first frame so I
				 * can print it out at the end consistently. 
				 * 
				 * I am going to disable the 'if' statement
				 * because for some reason if there's an
				 * anomaly in the frame, it keeps the data,
				 * so I guess for now I'll just have this
				 * information updated at every frame.
				 */
				if (file_info.good_frame_count > 0) {
					first_mp3_frame = mp3_i;
					found_first_frame = TRUE;
				}

				/* Print out per-frame stats. */
				if (!flags->sflag && flags->vvflag
				    && (file_info.good_frame_count > 0))
					print_frame_info(&mp3_i, &file_info);

				/*
				 * This will checkety-check to see if I get
				 * the next frame when I am supposed to. 
				 */ 
				if (mp3_i.PROT_BIT)
					file_info.next_expected_frame =
					    file_info.byte_count +
					    mp3_i.FRAME_LENGTH - 2;
				else
					file_info.next_expected_frame =
					    file_info.byte_count +
					    mp3_i.FRAME_LENGTH;

				/*
				 * Keep on searching the whole mp3? Ok, lemme
				 * skip though the data to get to the next
				 * header. 
				 */
				if (flags->aflag
				    || (flags->bflag
					&& (flag_options->byte_limit > 0))) {
					if (!move_to_next_frame
					    (possible_mp3_tag, &mp3_i,
					     &file_info, flags, fp))
						break;
				} else
					/*
					 * I found my first valid header, so
					 * I can quit now. 
					 */
					break;

			} else {
				/*
				 * I guess if I get to this section, I have
				 * found an mp3 with some bad stuff in it, so
				 * I will return a fail when the program exits.
				 */
				file_info.bad_frame_count++;

				if (flags->vflag)
					fprintf(stderr,
						"\nA possible header 0x%x passed the weak sieve, but failed the strong one at offset 0x%x (BYTE %d).",
						header_value,
						file_info.byte_count,
						file_info.byte_count);


				if (flags->qflag
				    && (file_info.frame_sequence_count > 0)
				    && (flag_options->min_frame_seq >
					file_info.frame_sequence_count)) {
					fprintf(stderr,
						"\nMininum contiguous number of frames wasn't reached. Got to %d, needed %d at offset 0x%x (BYTE %d).",
						file_info.frame_sequence_count,
						flag_options->min_frame_seq,
						file_info.next_expected_frame,
						file_info.next_expected_frame);

					file_info.frame_sequence_count = 0;
				}
			}

		} else if (file_info.file_pos == 4
			   && (header_value & 0xffffff00) == 0x49443300) {


			/*
			 * Getting to this section means we may have an
			 * ID3V2.x.x header
			 */
			fprintf(stderr,
				"Possible ID3v2 frame found, skipping\n");

			/*
			 * Since this section is going to be under serious
			 * development, the flag 'eflag' will allow mp3_check
			 * to record a bad frame when an id3v2 tag is found.
			 * This will allow the user to weed out troublesome
			 * id3v2 mp3s.
			 */
			mp3_i.ID3V2 = TRUE;

			if (flags->eflag) {
				if (flags->qflag
				    && (file_info.frame_sequence_count > 0)
				    && (flag_options->min_frame_seq >
					file_info.frame_sequence_count)) {
					fprintf(stderr,
						"\nMininum contiguous number of frames wasn't reached. Got to %d, needed %d at offset 0x%x (BYTE %d).",
						file_info.frame_sequence_count,
						flag_options->min_frame_seq,
						file_info.next_expected_frame,
						file_info.next_expected_frame);

					file_info.frame_sequence_count = 0;
				}

				file_info.bad_frame_count++;
			}

			/*
			 * TODO: Full ID3V2 checking & processing instead of
			 * skipping.
			 *
			 * Throw away the next two bytes - the first one
			 * should always be > 0xff
			 * The next 4 bytes represent the len encoded to
			 * avoid 0x80 bits set
			 */
			for (counter = 0; counter < 6; counter++) {
				if (get_char_from_file
				    (fp, &header_value, &file_info, flags,
				     possible_mp3_tag)) {
					if (!flags->ssflag && flags->sflag
					    && !flags->fflag) {
						printf("%c",
						       (header_value & 0xff));
					}
				} else {
					break;
				}
			}

			/* calculate the proper length */
			counter = ((header_value >> 3) & 0xfe00000)
			    | ((header_value >> 2) & 0x1fc000)
			    | ((header_value >> 1) & 0x3f80)
			    | (header_value & 0x7f);

			/*
			 * TODO: account for Unsynchronization!
			 * we may won't skip all of it if Unsynchroized is
			 * set (0x80 in byte 5 of file) account for the 10
			 * byte header and the next header this is to avoide
			 * excess messages during sync
			 */
			file_info.next_expected_frame = counter + 14;

			/* skip the ID3V2 frame */
			while (counter-- > 0) {
				if (get_char_from_file
				    (fp, &header_value, &file_info, flags,
				     possible_mp3_tag)) {
					if (!flags->ssflag && flags->sflag
					    && !flags->fflag) {
						printf("%c",
						       (header_value & 0xff));
					}
				} else {
					break;
				}
			}

			/* and mark as not currently synchronized - force it */
			found_valid_header = YES;
			found_weak_header = NO;
		} else {
			/*
			 * Getting to this section does not mean we got an
			 * invalid mp3 file...
			 */
			found_valid_header = NO;
			found_weak_header = NO;
		}
	}


	if (!flags->sflag) {
		if (flags->pflag) {
			printf("%s %s\t%s %d\t%s %d\t%s %d", "FILE_NAME",
			       filename, "GOOD_FRAMES",
			       file_info.good_frame_count, "BAD_FRAMES",
			       file_info.bad_frame_count, "LAST_BYTE_CHECKED",
			       file_info.byte_count);
		} else {
			printf("\n");
			printf("%-20s%s\n", "FILE_NAME", filename);
			printf("%-20s%d\n", "GOOD_FRAMES",
			       file_info.good_frame_count);
			printf("%-20s%d\n", "BAD_FRAMES",
			       file_info.bad_frame_count);
			printf("%-20s%d", "LAST_BYTE_CHECKED",
			       file_info.byte_count);
		}

		if ((vbr_info.high_rate != vbr_info.low_rate)
		    && (file_info.good_frame_count > 0)) {
			/*
			 * I do not want floating values in ave_rate. Seems
			 * silly to have a decimal point for that.
			 */
			vbr_info.ave_rate =
			    vbr_info.sum_rate / file_info.good_frame_count;

			if (flags->pflag) {
				/*
				 * I don't want any newlines for the below
				 * line because the pflag option is to print
				 * it out all one one line
				 */
				printf("\t%s %d\t%s %d\t%s %d", "VBR_HIGH",
				       vbr_info.high_rate, "VBR_LOW",
				       vbr_info.low_rate, "VBR_AVERAGE",
				       vbr_info.ave_rate);
			} else {
				printf("\n");
				printf("%-20s%d\n", "VBR_HIGH",
				       vbr_info.high_rate);
				printf("%-20s%d\n", "VBR_LOW",
				       vbr_info.low_rate);
				printf("%-20s%d", "VBR_AVERAGE",
				       vbr_info.ave_rate);
			}
		}

		if (found_first_frame) {
			if (flags->vflag && !flags->pflag) {
				print_summary(&first_mp3_frame, filename);
			}

			translate_time(&file_info, &song_time);

			if (flags->pflag)
				printf("\t%-20s%02u:%02u.%02u\n",
				       "SONG_LENGTH", song_time.minutes,
				       song_time.seconds,
				       song_time.frac_second);
			else
				printf("\n%-20s%02u:%02u.%02u\n",
				       "SONG_LENGTH", song_time.minutes,
				       song_time.seconds,
				       song_time.frac_second);

		} else {
			/*
			 * This line below closes the strings above. notice
			 * that at the end of the last string, there is no
			 * newline...
			 */
			printf("\n");
		}
	}


	init_id3_tag_struct(&id3_tag);
	transform_char_array(possible_mp3_tag, &file_info);


	if (flags->iflag && validate_id3_tag(possible_mp3_tag, &id3_tag)) {
		if (flags->iflag && flags->fflag && flags->sflag)
			dump_id3_tag(&id3_tag);

		if (!flags->sflag && flags->vflag) {
			printf("\n%-20s%d\n", "MP3_TAG", 1);

			printf("%-20s%s\n", "TITLE", id3_tag.TITLE);
			printf("%-20s%s\n", "ARTIST", id3_tag.ARTIST);
			printf("%-20s%s\n", "ALBUM", id3_tag.ALBUM);
			printf("%-20s%s\n", "YEAR", id3_tag.YEAR);
			printf("%-20s%s\n", "COMMENT", id3_tag.COMMENT);
			printf("%-20s%d\n", "GENRE", id3_tag.GENRE);
			printf("%-20s%d\n", "TRACK", id3_tag.TRACK_NUMBER);
			printf("%-20s%d\n", "ID3.11", id3_tag.ID3_311_VERSION);
		}
	} else if (!flags->sflag && flags->iflag) {
		printf("\n%-20s%d\n", "MP3_TAG", 0);
	}

	if (ferror(fp)) {
		fprintf(stderr, _("%s: %s %s\n"), PACKAGE, filename,
			strerror(errno));
		clearerr(fp);
	}

	if (ferror(stdout)) {
		fprintf(stderr, _("%s: stdout %s\n"), PACKAGE,
			strerror(errno));
		exit(1);
	}

	/*
	 * The 'file_info.bad_frame_count' seems the best way to find errors,
	 * so I will use it.
	 */
	if (file_info.bad_frame_count > 0)
		return (FAIL);
	else
		return (PASS);
}

int parse_args(char *argv[], meta_options *flag_options, command_flags *flags)
{
	FILE *fp;
	char *filename;
	int error_count = 0;

	fp = stdin;
	filename = "stdin";

	do {
		if (*argv) {
			if (strcmp(*argv, "-") == 0) {
				fp = stdin;
			} else if ((fp = fopen(*argv, "r")) == NULL) {
				fprintf(stderr, "mp3_check: %s %s\n", *argv,
					strerror(errno));
				++argv;
				filename = *argv++;
				continue;
			}
			filename = *argv++;
		}

		if (!scan_file(fp, filename, flag_options, flags))
			error_count++;

		if (fp != stdin)
			fclose(fp);

	} while (*argv);

	if (error_count > 0)
		return (FAIL);
	else
		return (PASS);
}

static inline int move_to_next_frame(char *possible_mp3_tag,
				     frame_info *mp3_i, gen_info *file_info,
				     command_flags *flags, FILE *fp)
{
	int counter = 0;
	int unk_char = 0;

	const int print_to_stdout = !flags->ssflag && (flags->sflag
						       || flags->fflag);
	int bytes_to_read = 0;

	/* Here, mp3_check adjusts for the CRC checksum */
	if (mp3_i->PROT_BIT)
		/* move past the header (4 bytes) and CRC (2 bytes) = 6 */
		bytes_to_read = mp3_i->FRAME_LENGTH - 6;
	else
		/* move past the header (4 bytes) since no CRC */
		bytes_to_read = mp3_i->FRAME_LENGTH - 4;

	/*
	 * This for loop cycles though the frame data. All I want to know is
	 * if the frame header is valid or not... basically, I know I got a
	 * good frame, I just want to hurry to the next one. 
	 */
	if (fp != stdin && !print_to_stdout && !flags->iflag) {

		if (!fseek(fp, bytes_to_read, SEEK_CUR)) {
			file_info->file_pos += bytes_to_read;
			file_info->byte_count += bytes_to_read;

			return (PASS);
		}
	}

	/*
	 * Walk though the contents by hand if we've got to print the
	 * contents, or if we're reading from stdin, or if something went
	 * wrong with fseek().
	 */
	for (counter = 0; counter < bytes_to_read; counter++) {
		if ((unk_char = getc(fp)) != EOF) {

			++file_info->file_pos;
			++file_info->byte_count;

			/*
			 * This is to keep track of the id3 tag that is at
			 * the end of mp3s.
			 */
			if (flags->iflag)
				rotate_char_array(possible_mp3_tag, &unk_char,
						  file_info);

			/*
			 * Since I know I am in a good frame, print out the
			 * data. When I am in this function, it doesnt matter
			 * if the fflag is set. 
			 */
			if (print_to_stdout)
				printf("%c", unk_char);


		} else {
			return (FAIL);
			break;
		}
	}

	return (PASS);
}

static int get_char_from_file(FILE *fp, unsigned int *header_value,
			      gen_info *file_info, command_flags *flags,
			      char *possible_mp3_tag)
{
	int step_char = 0;

	if ((step_char = getc(fp)) != EOF) {
		/*
		 * This keeps the 32 bit header, and rotates out the oldest,
		 * and adds a new one.
		 */
		*header_value = (*header_value << 8) + step_char;

		++file_info->file_pos;
		++file_info->byte_count;

		/*
		 * This section keeps track of the last 128 Bytes of the stream
		 * to later check for mp3 tags.
		 */
		if (flags->iflag)
			rotate_char_array(possible_mp3_tag, &step_char,
					  file_info);

		return (PASS);
	} else
		return (FAIL);
}

static int check_vbr_and_time(frame_info *mp3_i, vbr_data *vbr_info,
			      gen_info *file_info)
{
	/*
	 * Lets get the time of this frame, and add it to the total length of
	 * the mp3 file
	 */ 
	file_info->time_in_seconds +=
	    (mp3_i->SAMPLES_PER_FRAME * 1.0) / (mp3_i->SAMPLE_FREQ * 1.0);

	/* Time to do some nifty VBR checking. */ 
	if (vbr_info->high_rate < mp3_i->BIT_RATE)
		vbr_info->high_rate = mp3_i->BIT_RATE;

	if ((mp3_i->BIT_RATE < vbr_info->low_rate)
	    || (vbr_info->low_rate == 0)) vbr_info->low_rate = mp3_i->BIT_RATE;


	/* Get a running total for division at the end. */
	vbr_info->sum_rate += mp3_i->BIT_RATE;

	return (TRUE);
}
