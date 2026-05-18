/*
 * tests/testflags.h - shared debugging flags for vobcopy unit tests
 *
 * Include this header in each test program.  When a test creates temporary
 * files, define TEST_HAS_TEMPS before including this header to enable the
 * --keep-temps flag and maybe_unlink().
 * If a test also creates temp directories, additionally define
 * TEST_HAS_TEMP_DIRS to enable maybe_rmdir().
 *
 * Recognised flags (passed via argv or the TESTFLAGS environment variable):
 *
 *   -v / --verbose      Print [verbose] progress lines while running tests.
 *   -k / --keep-temps   Do not delete temp files/dirs after the test; print
 *                       their paths so they can be inspected manually.
 *                       Only meaningful in tests compiled with TEST_HAS_TEMPS.
 *
 * Usage in Makefile:
 *   make check TESTFLAGS="-v"
 *   make check TESTFLAGS="-v -k"
 */

#ifndef TESTFLAGS_H
#define TESTFLAGS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Set by -v / --verbose. */
static int verbose = 0;

#ifdef TEST_HAS_TEMPS
#include <assert.h>
#include <errno.h>
#include <unistd.h>

/* Set by -k / --keep-temps (only when TEST_HAS_TEMPS is defined). */
static int keep_temps = 0;

/*
 * Remove a temporary file, or keep it if --keep-temps was given.
 * In keep mode the path is printed so the user can inspect it.
 */
static void maybe_unlink(const char *path)
{
  if (keep_temps)
    printf("[keep-temps] keeping file %s\n", path);
  else
    assert(unlink(path) == 0);
}

#ifdef TEST_HAS_TEMP_DIRS
/*
 * Remove a temporary directory, or keep it if --keep-temps was given.
 */
static void maybe_rmdir(const char *path)
{
  if (keep_temps)
    printf("[keep-temps] keeping dir  %s\n", path);
  else
    assert(rmdir(path) == 0);
}
#endif /* TEST_HAS_TEMP_DIRS */
#endif /* TEST_HAS_TEMPS */

/*
 * Parse debugging flags from argv and from the TESTFLAGS environment variable.
 * Call this at the top of main().
 *
 * argv flags: errors on unknown flags (so typos are caught when running the
 * binary directly).
 * TESTFLAGS env: unknown tokens are silently ignored (so TESTFLAGS="-v -k"
 * works uniformly for all test binaries even those without temp files).
 */
static void parse_testflags(int argc, char **argv)
{
  int i;
  const char *env;
  char buf[512];
  char *tok;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      verbose = 1;
    } else if (strcmp(argv[i], "-k") == 0 ||
               strcmp(argv[i], "--keep-temps") == 0) {
#ifdef TEST_HAS_TEMPS
      keep_temps = 1;
#endif
      /* silently accepted as a no-op in binaries without TEST_HAS_TEMPS */
    } else {
      fprintf(stderr, "unknown test flag: %s\n", argv[i]);
      exit(1);
    }
  }

  env = getenv("TESTFLAGS");
  if (env != NULL) {
    strncpy(buf, env, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    tok = strtok(buf, " \t");
    while (tok != NULL) {
      if (strcmp(tok, "-v") == 0 || strcmp(tok, "--verbose") == 0) {
        verbose = 1;
      } else if (strcmp(tok, "-k") == 0 ||
                 strcmp(tok, "--keep-temps") == 0) {
#ifdef TEST_HAS_TEMPS
        keep_temps = 1;
#endif
      }
      /* other tokens in the env var are silently ignored */
      tok = strtok(NULL, " \t");
    }
  }
}

/* Emit a formatted progress line when --verbose is active. */
static void vlog(const char *fmt, ...)
{
  va_list ap;
  if (!verbose) return;
  printf("[verbose] ");
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  putchar('\n');
  fflush(stdout);
}

#endif /* TESTFLAGS_H */
