#!/bin/sh
set -eu

# Parse TESTFLAGS environment variable for -v/--verbose.
# (-k/--keep-temps is accepted but has no effect here: no temp files.)
verbose=0
for flag in ${TESTFLAGS:-}; do
  case "$flag" in
    -v|--verbose) verbose=1 ;;
  esac
done

vlog() {
  if [ "$verbose" -eq 1 ]; then
    printf '[verbose] %s\n' "$*"
  fi
}

vlog "checking: vobcopy -V exits zero"
"$top_builddir"/vobcopy -V >/dev/null

vlog "checking: vobcopy -h exits non-zero"
if "$top_builddir"/vobcopy -h >/dev/null 2>&1; then
  echo "-h should exit non-zero"
  exit 1
fi

vlog "checking: vobcopy -a nope exits non-zero"
if "$top_builddir"/vobcopy -a nope >/dev/null 2>&1; then
  echo "-a with non-numeric value should fail"
  exit 1
fi

vlog "all cli-smoke checks passed"
