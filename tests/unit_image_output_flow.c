#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define main vobcopy_main
#include "../vobcopy.c"
#undef main

/* testflags.h must come after the vobcopy.c include so it does not interfere
   with vobcopy's own compilation; all the system headers it needs are already
   pulled in transitively. */
#define TEST_HAS_TEMPS
#define TEST_HAS_TEMP_DIRS
#include "testflags.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

static void write_disc_title_block(const char *path, const char *title)
{
  int fd;
  unsigned char block[2048];
  size_t len;

  memset(block, 0, sizeof(block));
  memset(block + 40, ' ', 32);
  len = strlen(title);
  if (len > 32) len = 32;
  memcpy(block + 40, title, len);

  fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0600);
  assert(fd >= 0);
  assert(lseek(fd, 32768, SEEK_SET) == 32768);
  assert(write(fd, block, sizeof(block)) == (ssize_t) sizeof(block));
  assert(close(fd) == 0);
}

static void test_dummy_image_title_to_output_path(void)
{
  char image_template[] = "/tmp/vobcopy-flow-image-XXXXXX";
  char output_dir_template[] = "/tmp/vobcopy-flow-output-XXXXXX";
  char dvd_title[64];
  char pwd[PATH_BUFFER_SIZE];
  char out_name[PATH_BUFFER_SIZE];
  char expected[PATH_BUFFER_SIZE];
  int fd;

  vlog("test_dummy_image_title_to_output_path: creating temp image");
  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);
  vlog("  temp image: %s", image_template);

  write_disc_title_block(image_template, "Flow Test Disc");
  vlog("  wrote title \"Flow Test Disc\"");

  assert(get_dvd_name(image_template, dvd_title) == 0);
  vlog("  get_dvd_name() => \"%s\"", dvd_title);
  assert(strcmp(dvd_title, "Flow_Test_Disc") == 0);

  assert(mkdtemp(output_dir_template) != NULL);
  vlog("  output dir: %s", output_dir_template);
  snprintf(pwd, sizeof(pwd), "%s/", output_dir_template);

  assert(make_output_path(pwd, out_name, dvd_title, 2, 3) == 0);
  vlog("  make_output_path(title=2 part=3) => \"%s\"", out_name);
  snprintf(expected, sizeof(expected), "%s/%s2-3.vob", output_dir_template, dvd_title);
  assert(strcmp(out_name, expected) == 0);

  maybe_unlink(image_template);
  maybe_rmdir(output_dir_template);
}

static void test_dummy_image_output_file_rename(void)
{
  char image_template[] = "/tmp/vobcopy-flow-image-rename-XXXXXX";
  char output_dir_template[] = "/tmp/vobcopy-flow-output-rename-XXXXXX";
  char dvd_title[64];
  char pwd[PATH_BUFFER_SIZE];
  char out_name[PATH_BUFFER_SIZE];
  char partial_name[PATH_BUFFER_SIZE + 16];
  char content[6];
  int missing_fd;
  int fd;

  vlog("test_dummy_image_output_file_rename: creating temp image");
  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);
  vlog("  temp image: %s", image_template);

  write_disc_title_block(image_template, "Rename Disc");
  vlog("  wrote title \"Rename Disc\"");

  assert(get_dvd_name(image_template, dvd_title) == 0);
  vlog("  get_dvd_name() => \"%s\"", dvd_title);

  assert(mkdtemp(output_dir_template) != NULL);
  vlog("  output dir: %s", output_dir_template);
  snprintf(pwd, sizeof(pwd), "%s/", output_dir_template);
  assert(make_output_path(pwd, out_name, dvd_title, 1, 0) == 0);
  vlog("  output path: %s", out_name);

  snprintf(partial_name, sizeof(partial_name), "%s.partial", out_name);
  vlog("  creating partial file: %s", partial_name);
  fd = open(partial_name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0600);
  assert(fd >= 0);
  assert(write(fd, "dummy", 5) == 5);
  assert(close(fd) == 0);

  vlog("  calling re_name() to promote .partial to final name");
  re_name(partial_name);

  errno = 0;
  missing_fd = open(partial_name, O_RDONLY | O_BINARY);
  assert(missing_fd < 0);
  assert(errno == ENOENT);
  vlog("  .partial file gone as expected");

  fd = open(out_name, O_RDONLY | O_BINARY);
  assert(fd >= 0);
  assert(read(fd, content, 5) == 5);
  content[5] = '\0';
  vlog("  final file contents: \"%s\"", content);
  assert(strcmp(content, "dummy") == 0);
  assert(close(fd) == 0);

  maybe_unlink(out_name);
  maybe_unlink(image_template);
  maybe_rmdir(output_dir_template);
}

int main(int argc, char **argv)
{
  parse_testflags(argc, argv);

  test_dummy_image_title_to_output_path();
  test_dummy_image_output_file_rename();
  return 0;
}
