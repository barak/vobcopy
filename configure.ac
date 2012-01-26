# Process this file with autoconf to produce a configure script.

# header
AC_PREREQ([2.67])
AC_INIT([vobcopy],[1.2.0],[Robos <robos@muon.de>],[vobcopy],[http://vobcopy.org])
AC_CONFIG_SRCDIR([vobcopy.c])
AC_CONFIG_HEADERS([config.h])

# Enable automake
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([dvdread], [DVDOpen])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h libintl.h locale.h mntent.h stdint.h stdlib.h string.h sys/ioctl.h sys/mount.h sys/param.h sys/statfs.h sys/statvfs.h sys/vfs.h termios.h unistd.h])

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
AC_CHECK_FUNCS([alarm fdatasync getcwd getmntent getmntinfo memset mkdir setlocale strcasecmp strerror strpbrk strstr])

# trailer
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
