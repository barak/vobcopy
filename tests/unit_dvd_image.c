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

#define TEST_HAS_TEMPS
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

static int get_dvd_name_with_captured_stderr(const char *image_path, char *title, char *captured, size_t captured_size)
{
  FILE *tmp_stderr;
  int saved_stderr_fd;
  int rc;
  size_t nread;

  tmp_stderr = tmpfile();
  assert(tmp_stderr != NULL);

  saved_stderr_fd = dup(STDERR_FILENO);
  assert(saved_stderr_fd >= 0);
  assert(dup2(fileno(tmp_stderr), STDERR_FILENO) >= 0);

  rc = get_dvd_name(image_path, title);

  assert(fflush(stderr) == 0);
  assert(dup2(saved_stderr_fd, STDERR_FILENO) >= 0);
  assert(close(saved_stderr_fd) == 0);

  if (captured_size > 0) {
    rewind(tmp_stderr);
    nread = fread(captured, 1, captured_size - 1, tmp_stderr);
    captured[nread] = '\0';
  }

  assert(fclose(tmp_stderr) == 0);
  return rc;
}

static void test_get_dvd_name_from_small_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-small-XXXXXX";
  char title[64];
  int fd;

  vlog("test_get_dvd_name_from_small_image: creating temp image");
  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);
  vlog("  temp image: %s", image_template);

  write_disc_title_block(image_template, "My Disk:Title");
  vlog("  wrote title \"My Disk:Title\"");

  assert(get_dvd_name(image_template, title) == 0);
  vlog("  get_dvd_name() => \"%s\"", title);
  assert(strcmp(title, "My_Disk_Title") == 0);

  maybe_unlink(image_template);
}

static void test_get_dvd_name_from_large_sparse_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-large-XXXXXX";
  char title[64];
  int fd;

  vlog("test_get_dvd_name_from_large_sparse_image: creating sparse temp image");
  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(close(fd) == 0);
  vlog("  temp image: %s", image_template);

  create_sparse_image_with_title(image_template, "Large Test Disc", (off_t)6 * 1024 * 1024 * 1024);
  vlog("  wrote title \"Large Test Disc\" (sparse 6 GB image)");

  assert(get_dvd_name(image_template, title) == 0);
  vlog("  get_dvd_name() => \"%s\"", title);
  assert(strcmp(title, "Large_Test_Disc") == 0);

  maybe_unlink(image_template);
}

static void test_get_dvd_name_reports_error_on_too_small_image(void)
{
  char image_template[] = "/tmp/vobcopy-image-tiny-XXXXXX";
  char title[64];
  char stderr_output[512];
  int fd;

  vlog("test_get_dvd_name_reports_error_on_too_small_image: creating 4-byte image");
  fd = mkstemp(image_template);
  assert(fd >= 0);
  assert(write(fd, "tiny", 4) == 4);
  assert(close(fd) == 0);
  vlog("  temp image: %s", image_template);

  assert(get_dvd_name_with_captured_stderr(image_template, title, stderr_output, sizeof(stderr_output)) < 0);
  vlog("  get_dvd_name() correctly returned error");
  vlog("  captured stderr: %s", stderr_output);
  assert(strstr(stderr_output, "only read ") != NULL);
  assert(strstr(stderr_output, "bytes instead of 2048") != NULL);
  assert(strstr(stderr_output, "error: Success") == NULL);

  maybe_unlink(image_template);
}

int main(int argc, char **argv)
{
  parse_testflags(argc, argv);

  test_get_dvd_name_from_small_image();
  test_get_dvd_name_from_large_sparse_image();
  test_get_dvd_name_reports_error_on_too_small_image();

  return 0;
}
