#include "vobcopy.h"
#include "common.h"

int add_end_slash( char *path )
{  /* add a trailing '/' to path */
  char *pointer;
  if ( path[strlen( path )-1] != '/' )
    {
      pointer = path + strlen( path );
      *pointer = '/';
      pointer++;
      *pointer = '\0';
    }
  return 0;
}

/* safe strncpy: copies at most n-1 bytes and always null-terminates */
char *safestrncpy(char *dest, const char *src, size_t n)
{
  if (n == 0) return dest;
  size_t src_len = strlen(src);
  size_t copy_len = src_len < n - 1 ? src_len : n - 1;
  memcpy(dest, src, copy_len);
  dest[copy_len] = '\0';
  return dest;
}

void sanitize_dvd_name( char *name )
{
  size_t i;

  if( name == NULL )
    {
      return;
    }

  for( i = 0; name[ i ] != '\0'; i++ )
    {
      unsigned char ch = (unsigned char) name[ i ];

      /* Keep generated filenames portable by flattening path separators and
       * ASCII control bytes into underscores while leaving UTF-8 bytes alone. */
      if( name[ i ] == ' '
          || name[ i ] == '\t'
          || name[ i ] == '\n'
          || name[ i ] == '\r'
          || name[ i ] == '/'
          || name[ i ] == '\\'
          || name[ i ] == ':'
          || ch < 0x20
          || ch == 0x7f )
        {
          name[ i ] = '_';
        }
    }
}

void get_fallback_dvd_name( const char *path, char *title, size_t title_size )
{
  char path_copy[PATH_BUFFER_SIZE];
  char *component;
  size_t path_length;

  if ( title_size == 0 )
    return;
  if ( title_size == 1 )
    {
      title[0] = '\0';
      return;
    }

  safestrncpy( title, DEFAULT_DVD_NAME, title_size );
  if( !path || !*path )
    return;

  safestrncpy( path_copy, path, sizeof(path_copy) );
  path_length = strlen( path_copy );
  while( path_length > 1 && path_copy[ path_length - 1 ] == '/' )
    {
      path_copy[ path_length - 1 ] = '\0';
      path_length--;
    }

  component = strrchr( path_copy, '/' );
  component = component ? component + 1 : path_copy;
  if( !strcasecmp( component, "VIDEO_TS" ) )
    {
      if( component == path_copy )
        {
          if( getcwd( path_copy, sizeof(path_copy) ) == NULL )
            {
              fprintf( stderr, _("[Warning] Couldn't determine the current directory for a fallback dvd name.\n") );
              return;
            }
        }
      else
        {
          *( component - 1 ) = '\0';
        }

      path_length = strlen( path_copy );
      while( path_length > 1 && path_copy[ path_length - 1 ] == '/' )
        {
          path_copy[ path_length - 1 ] = '\0';
          path_length--;
        }

      component = strrchr( path_copy, '/' );
      component = component ? component + 1 : path_copy;
    }

  if( !*component )
    return;

  safestrncpy( title, component, title_size );
  sanitize_dvd_name( title );
}
