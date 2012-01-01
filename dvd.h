/*this files is part of vobcopy*/

int get_dvd_name(const char *, char *);
int get_device(char *, char *);
int get_device_on_your_own(char *, char *);
off_t get_vob_size(int , char *); 
/* int dvdtime2msec(dvd_time_t *); */
/* void converttime(playback_time_t *, dvd_time_t *); */
int get_longest_title( dvd_reader_t * );
typedef struct {
	int hour;
	int minute;
	int second;
	int usec;
} playback_time_t;


struct dvd_info {
	struct {
		char *device;
		char *disc_title;
		char *vmg_id;
		char *provider_id;
	} discinfo;
	int title_count;
	struct {
		int enabled;
		struct {
			float length;
			playback_time_t playback_time;
			char *vts_id;
		} general;
		struct {
			int vts;
			int ttn;
			float fps;
			char *format;
			char *aspect;
			char *width;
			char *height;
			char *df;
		} parameter;
		int angle_count; /* no real angle detail is available... but hey. */
		int audiostream_count;
		struct {
			char *langcode;
			char *language;
			char *format;
			char *frequency;
			char *quantization;
			int channels;
			int ap_mode;
			char *content;
			int streamid;
		} *audiostreams;
		int chapter_count_reported; /* This value is sometimes wrong */
		int chapter_count; /* This value is real */
		struct {
			float length;
			playback_time_t playback_time;
			int startcell;
		} *chapters;
		int cell_count;
		struct {
			float length;
			playback_time_t playback_time;
		} *cells;
		int subtitle_count;
		struct {
			char *langcode;
			char *language;
			char *content;
			int streamid;
		} *subtitles;
		int *palette;
	} *titles;
	int longest_track;
};
