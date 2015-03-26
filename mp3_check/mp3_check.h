#define CURRENT_VERSION	"1.98"
#define HOMEPAGE	"http://sourceforge.net/project/?group_id=6126"
#define AUTHOR		"Eric Bullen <ericb at thedeepsky.com>"

#define TRUE    1
#define FALSE   0

#define PASS    1
#define FAIL    0

#define YES	1
#define NO	0

#define BUFFER_LENGTH 128

//
// General MP3 file info
//
typedef struct {
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
typedef struct {
        int	high_rate;
        int	low_rate;
        int	sum_rate;
        int	ave_rate;
} vbr_data;


//
// MP3 Time Structure
//
typedef struct {
	int	minutes;
	int	seconds;
	int	frac_second;
} mp3_time;


//
// Structure to hold the flag options.
//
typedef struct {
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
typedef struct {
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
typedef struct {
	//  
	//  Declare variables for the header storage 
	//  
	int		FRAME_LENGTH;
	int		FRAME_DATA_LENGTH;
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
	short int	CRC16_VALUE;
	short int	CORRECT_CRC16_VALUE;

	short int	check_state;
} frame_info;


//  
//  This gives info about the id3 tag found in the last part 
//  of the mp3 file. 
//  
typedef struct {
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
