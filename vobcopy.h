#define VERSION "0.5.13"

#define DVDCSS_VERBOSE 1
#define BLOCK_COUNT 64
#define MAX_STRING  81
#define MAX_DIFFER  2000

#if defined(__APPLE__) && defined(__GNUC__)
typedef int bool;

#define FALSE 0
#define TRUE 1
#else
  typedef enum  { FALSE=0, TRUE=1 }  bool;
#endif /* Darwin */


void usage(char *);
int end_slash_adder( char * );
off_t freespace_getter( char *, int );
off_t usedspace_getter( char *path, int verbosity_level );
int output_path_maker( char *, char *, int, char *, int, int );
int is_nav_pack( unsigned char *buffer );
void re_name( char *output_file );
int makedir( char *name );
