#!/bin/sh
set -eu

"$top_builddir"/vobcopy -V >/dev/null

if "$top_builddir"/vobcopy -h >/dev/null 2>&1; then
  echo "-h should exit non-zero"
  exit 1
fi

if "$top_builddir"/vobcopy -a nope >/dev/null 2>&1; then
  echo "-a with non-numeric value should fail"
  exit 1
fi
