#define VERSION "1.0.2"

#define DVDCSS_VERBOSE 1
#define BLOCK_COUNT 64
#define MAX_STRING  81
#define MAX_DIFFER  2000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h> /*for readdir*/
#include <errno.h>

#if ( defined( __unix__ ) || defined( unix )) && !defined( USG )
#include <sys/param.h>
#else
#endif

#if defined( __GNUC__ ) && \
    !( defined( sun ) )
#include <getopt.h>
#endif

/* FreeBSD 4.10 and OpenBSD 3.2 has not <stdint.h> */
/* by some bugreport:*/
#if !( defined( BSD ) && ( BSD >= 199306 ) ) && !defined( sun )
#include <stdint.h>
#endif

/*for/from play_title.c*/
#include <assert.h>
/* #include "config.h" */
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_types.h>
#include <dvdread/ifo_read.h>
/* #include <dvdread/dvd_udf.h> */
#include <dvdread/nav_read.h>
#include <dvdread/nav_print.h>

/* I'm trying to have all supported OSes definitions clearly separated here */
/* The appearance could probably be made more readable -- lb                */

/* ////////// Solaris ////////// */
#if defined( __sun )

#include <stdlib.h>
#include <sys/mnttab.h>
#include <sys/statvfs.h>

typedef enum  { FALSE=0, TRUE=1 }  bool;

#  if ( _FILE_OFFSET_BITS == 64 )

#define HAS_LARGEFILE 1

#  endif

#else /* Solaris */

/* //////////  *BSD //////////  */
#if ( defined( BSD ) && ( BSD >= 199306 ) )

#  if  !defined( __NetBSD__ ) ) || \
       ( defined( __NetBSD__) && ( __NetBSD_Version__ < 200040000 ) )
#include <sys/mount.h>
#define USE_STATFS 1

#  else

#include <sys/statvfs.h>

#  endif

#  if defined(NetBSD)

#include <sys/param.h>

#define USE_GETMNTINFO

#    if ( __NetBSD_Version__ < 200040000 )

#include <sys/mount.h>
#define USE_STATFS_FOR_DEV
#define GETMNTINFO_USES_STATFS

#    else
#include <sys/statvfs.h>
#define USE_STATVFS_FOR_DEV
#define GETMNTINFO_USES_STATVFS

#    endif

#  else

#include <sys/vfs.h>

#  endif

#define HAS_LARGEFILE 1

typedef enum  { FALSE=0, TRUE=1 }  bool;

#else /* *BSD */

/* ////////// Darwin / OS X ////////// */
#if defined ( __APPLE__ ) 

/* ////////// Darwin ////////// */
#  if defined( __GNUC__ )

#include <sys/param.h>
#include <sys/mount.h>

#include <sys/vfs.h>
#include <sys/statvfs.h>

#define USE_STATFS     1
#define HAS_LARGEFILE  1
#define USE_GETMNTINFO 1

#define FALSE 0
#define TRUE 1
typedef int bool;

#  endif

/* ////////// OS X ////////// */
#  if defined( __MACH__ )

#include <unistd.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>

#  endif

#else  /* Darwin / OS X */

/* ////////// GNU/Linux ////////// */
#if ( defined( __linux__ ) )

#include <mntent.h>
#include <sys/vfs.h>
#include <sys/statfs.h>

#define USE_STATFS       1
#define HAVE_GETOPT_LONG 1
#define HAS_LARGEFILE    1

  typedef enum  { FALSE=0, TRUE=1 }  bool;

#else /* GNU/Linux */

/* ////////// For other cases ////////// */

typedef enum  { FALSE=0, TRUE=1 }  bool;

#if defined( __USE_FILE_OFFSET64 )
#  define HAS_LARGEFILE 1
#endif
#endif
#endif
#endif
#endif 

#include "dvd.h"

#define off_t __off64_t

void usage(char *);
int add_end_slash( char * );
off_t get_free_space( char *, int );
off_t get_used_space( char *path, int verbosity_level );
int make_output_path( char *, char *, int, char *, int, int );
int is_nav_pack( unsigned char *buffer );
void re_name( char *output_file );
int makedir( char *name );
