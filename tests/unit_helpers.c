#include <assert.h>
#include <string.h>

#include "common.h"

#ifndef DEFAULT_DVD_NAME
#define DEFAULT_DVD_NAME "insert_name_here"
#endif

static void test_safestrncpy_truncates_and_terminates(void)
{
  char out[5];

  memset(out, 'X', sizeof(out));
  safestrncpy(out, "abcdef", sizeof(out));
  assert(strcmp(out, "abcd") == 0);
  assert(out[4] == '\0');
}

static void test_add_end_slash(void)
{
  char path[32];

  strcpy(path, "/tmp/vobcopy-tests");
  add_end_slash(path);
  assert(strcmp(path, "/tmp/vobcopy-tests/") == 0);

  add_end_slash(path);
  assert(strcmp(path, "/tmp/vobcopy-tests/") == 0);
}

static void test_sanitize_dvd_name(void)
{
  char name[] = "Movie Name:\\Disc/One\n";

  sanitize_dvd_name(name);
  assert(strcmp(name, "Movie_Name__Disc_One_") == 0);
}

static void test_get_fallback_dvd_name_from_video_ts_path(void)
{
  char title[64];

  get_fallback_dvd_name("/media/My Great Disc/VIDEO_TS/", title, sizeof(title));
  assert(strcmp(title, "My_Great_Disc") == 0);
}

static void test_get_fallback_dvd_name_uses_default_on_root(void)
{
  char title[64];

  get_fallback_dvd_name("/", title, sizeof(title));
  assert(strcmp(title, DEFAULT_DVD_NAME) == 0);
}

int main(void)
{
  test_safestrncpy_truncates_and_terminates();
  test_add_end_slash();
  test_sanitize_dvd_name();
  test_get_fallback_dvd_name_from_video_ts_path();
  test_get_fallback_dvd_name_uses_default_on_root();

  return 0;
}
