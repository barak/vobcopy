#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define DVDCSS_VERBOSE 1
#define BLOCK_COUNT 64
#define MAX_STRING  81
#define MAX_DIFFER  2000

#ifdef ENABLE_NLS
#define _(Text) gettext(Text)
#else
#define _(Text) Text
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
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

#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#define USE_STATFS
/* #define USE_STATFS_FOR_DEV */
#endif

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#ifndef USE_STATFS
#define USE_STATVFS
#ifndef USE_STATFS_FOR_DEV
#define USE_STATVFS_FOR_DEV
#endif
#endif
#endif

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef HAVE_GETMNTINFO
#define USE_GETMNTINFO
#define GETMNTINFO_USES_STATFS
#endif

#include <dvdread/dvd_reader.h>

/*for/from play_title.c*/
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

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

#ifndef HAVE_FDATASYNC
#define fdatasync(fd) 0
#endif
