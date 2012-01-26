#if defined( __gettext__ )
#include <locale.h>
#include <libintl.h>
#define _(Text) gettext(Text)
#else
#define _(Text) Text
#endif

#define DVDCSS_VERBOSE 1
#define BLOCK_COUNT 64
#define MAX_STRING  81
#define MAX_DIFFER  2000

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <features.h>
#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <string.h>
#include <ctype.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <dirent.h> /*for readdir*/
#include <errno.h>
#include <signal.h>
#include <time.h>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if defined( __GNUC__ ) && \
    !( defined( sun ) )
#include <getopt.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#ifdef HAVE_GETMNTINFO
#define USE_GETMNTINFO
#endif

#ifndef HAVE_STDBOOL_H
typedef enum  { FALSE=0, TRUE=1 }  bool;
#else
#include <stdbool.h>
#define TRUE true
#define FALSE false
#endif

/* I'm trying to have all supported OSes definitions clearly separated here */
/* The appearance could probably be made more readable -- lb                */

/* ////////// Solaris ////////// */
#if defined( __sun )

#include <sys/mnttab.h>

#else /* Solaris */

/* //////////  *BSD //////////  */
#if ( defined( BSD ) && ( BSD >= 199306 ) )

#if !defined( __NetBSD__ ) && !defined(__GNU__) || \
       ( defined( __NetBSD__) && ( __NetBSD_Version__ < 200040000 ) )
#include <sys/mount.h>
#define USE_STATFS 1
#endif

#if defined(__FreeBSD__)
#define USE_STATFS_FOR_DEV
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#  if defined(NetBSD) || defined (OpenBSD)

#    if ( __NetBSD_Version__ < 200040000 )

#include <sys/mount.h>
#define USE_STATFS_FOR_DEV
#define GETMNTINFO_USES_STATFS

#    else
#include <sys/statvfs.h>
#define USE_STATVFS_FOR_DEV
#define GETMNTINFO_USES_STATVFS

#    endif
#endif

#if defined(__FreeBSD__)
#define USE_STATFS_FOR_DEV
#include <sys/statvfs.h>
#else
#include <sys/vfs.h>
#endif

#else /* *BSD */

/* ////////// Darwin / OS X ////////// */
#if defined ( __APPLE__ )

/* ////////// Darwin ////////// */
#  if defined( __GNUC__ )

#include <sys/statvfs.h>
/*can't be both! Should be STATVFS IMHO */
/*#define USE_STATFS     1
#define USE_STATVFS     1 */
#define GETMNTINFO_USES_STATFS 1
#define USE_GETMNTINFO 1

#  endif

/* ////////// OS X ////////// */
#  if defined( __MACH__ )
/* mac osx 10.5 does not seem to like this one here */
/*#include <unistd.h>
#include <sys/vfs.h>
#include <sys/statvfs.h> */

#  endif

#else  /* Darwin / OS X */

/* ////////// GNU/Linux ////////// */
#if ( defined( __linux__ ) )

#include <mntent.h>
#include <sys/vfs.h>
#include <sys/statfs.h>

#define USE_STATFS       1
#define HAVE_GETOPT_LONG 1

#elif defined( __GLIBC__ )

#include <mntent.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>

#define HAVE_GETOPT_LONG 1

#else

#endif
#endif
#endif
#endif


/* OS/2 */
#if defined(__EMX__)
#define __off64_t __int64_t
#include <sys/vfs.h>
#include <sys/statfs.h>
#define USE_STATFS 1
#endif




#include <dvdread/dvd_reader.h>

/*for/from play_title.c*/
#include <assert.h>
/* #include "config.h" */
#include <dvdread/ifo_types.h>
#include <dvdread/ifo_read.h>
/* #include <dvdread/dvd_udf.h> */
#include <dvdread/nav_read.h>
#include <dvdread/nav_print.h>


#include "dvd.h"


void usage(char *);
int add_end_slash( char * );
off_t get_free_space( char *, int );
off_t get_used_space( char *path, int verbosity_level );
int make_output_path( char *, char *, int, char *, int, int );
int is_nav_pack( unsigned char *buffer );
void re_name( char *output_file );
int makedir( char *name );
void install_signal_handlers();
void watchdog_handler( int signal );
void shutdown_handler( int signal );
char *safestrncpy(char *dest, const char *src, size_t n);
int check_progress( void ); /* this can be removed because the one below supersedes it */
int progressUpdate( int starttime, int cur, int tot, int force );

#if defined(__APPLE__) && defined(__GNUC__)
int fdatasync( int value );
#endif
