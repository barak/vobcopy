#ifndef VOBCOPY_COMMON_H
#define VOBCOPY_COMMON_H

#include <stddef.h>

int add_end_slash( char *path );
char *safestrncpy(char *dest, const char *src, size_t n);
void sanitize_dvd_name( char *name );
void get_fallback_dvd_name( const char *path, char *title, size_t title_size );

#endif
