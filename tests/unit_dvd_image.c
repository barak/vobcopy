#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vobcopy.h"

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

static void create_sparse_image_with_title(const char *path, const char *title, off_t sparse_size)
{
  int fd;

  write_disc_title_block(path, title);

  fd = open(path, O_WRONLY | O_BINARY);
  assert(fd >= 0);
  assert(lseek(fd, sparse_size - 1, SEEK_SET) == sparse_size - 1);
  assert(write(fd, "\0", 1) == 1);
  assert(close(fd) == 0);
}

static void test_get_dvd_name_from_small_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-small-XXXXXX";
  char title[64];
  int fd;

  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);

  write_disc_title_block(image_template, "My Disk:Title");

  assert(get_dvd_name(image_template, title) == 0);
  assert(strcmp(title, "My_Disk_Title") == 0);

  assert(unlink(image_template) == 0);
}

static void test_get_dvd_name_from_large_sparse_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-large-XXXXXX";
  char title[64];
  int fd;

  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);

  create_sparse_image_with_title(image_template, "Large Test Disc", (off_t)6 * 1024 * 1024 * 1024);

  assert(get_dvd_name(image_template, title) == 0);
  assert(strcmp(title, "Large_Test_Disc") == 0);

  assert(unlink(image_template) == 0);
}

static void test_get_dvd_name_reports_error_on_too_small_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-tiny-XXXXXX";
  char title[64];
  int fd;

  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(write(fd, "tiny", 4) == 4);
  assert(close(fd) == 0);

  assert(get_dvd_name(image_template, title) < 0);

  assert(unlink(image_template) == 0);
}

int main(void)
{
  test_get_dvd_name_from_small_image();
  test_get_dvd_name_from_large_sparse_image();
  test_get_dvd_name_reports_error_on_too_small_image();

  return 0;
}
