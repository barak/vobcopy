#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define main vobcopy_embedded_main
#include "../vobcopy.c"
#undef main
#include "../dvd.c"

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

static void test_is_nav_pack_signature(void)
{
  unsigned char buffer[2048];

  memset(buffer, 0, sizeof(buffer));
  buffer[41] = 0xbf;
  buffer[1027] = 0xbf;
  assert(is_nav_pack(buffer) == 1);

  buffer[1027] = 0;
  assert(is_nav_pack(buffer) == 0);
}

static void test_advance_sector_range_position(void)
{
  sector_range_t ranges[2];
  int range_index = 0;
  uint32_t sector;

  ranges[0].first_sector = 10;
  ranges[0].last_sector = 12;
  ranges[1].first_sector = 20;
  ranges[1].last_sector = 22;

  sector = ranges[0].first_sector;
  assert(advance_sector_range_position(ranges, 2, &range_index, &sector, 2) == 0);
  assert(range_index == 0);
  assert(sector == 12);

  assert(advance_sector_range_position(ranges, 2, &range_index, &sector, 1) == 0);
  assert(range_index == 1);
  assert(sector == 20);

  assert(advance_sector_range_position(ranges, 2, &range_index, &sector, 10) == -1);
}

static void test_normalize_sector_range_position(void)
{
  sector_range_t ranges[2];
  int range_index = 0;
  uint32_t sector;

  ranges[0].first_sector = 100;
  ranges[0].last_sector = 105;
  ranges[1].first_sector = 200;
  ranges[1].last_sector = 210;

  sector = 106;
  normalize_sector_range_position(ranges, 2, &range_index, &sector);
  assert(range_index == 1);
  assert(sector == 200);
}

int main(void)
{
  test_safestrncpy_truncates_and_terminates();
  test_add_end_slash();
  test_sanitize_dvd_name();
  test_get_fallback_dvd_name_from_video_ts_path();
  test_get_fallback_dvd_name_uses_default_on_root();
  test_is_nav_pack_signature();
  test_advance_sector_range_position();
  test_normalize_sector_range_position();

  return 0;
}
