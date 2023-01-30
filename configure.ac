# Process this file with autoconf to produce a configure script.

# header
AC_PREREQ([2.69])
AC_INIT([vobcopy],[1.2.1],[Robos <robos@muon.de>],[vobcopy],[https://github.com/barak/vobcopy])
AC_CONFIG_SRCDIR([vobcopy.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# Enable automake
AM_INIT_AUTOMAKE([foreign -Wall])

# Checks for programs.
AC_PROG_CC
AX_CFLAGS_WARN_ALL
AC_PROG_LN_S
AC_PROG_MKDIR_P

# Checks for libraries.
AC_SEARCH_LIBS([DVDOpen], [dvdread],,[AC_MSG_ERROR([libdvdread not found])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h libintl.h features.h locale.h mntent.h stdint.h stdlib.h string.h sys/ioctl.h sys/mount.h sys/param.h sys/statfs.h sys/statvfs.h sys/vfs.h termios.h unistd.h getopt.h sys/mnttab.h assert.h dirent.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE
AC_FUNC_GETMNTENT
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm fdatasync getcwd getmntent getmntinfo memset mkdir setlocale strcasecmp strerror strpbrk strstr getopt_long])

# trailer
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
