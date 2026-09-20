# Building for Windows

This document builds 64-bit Windows binaries on Ubuntu with MinGW-w64.  It
installs the cross-compiled libdvdcss and libdvdread dependencies in
`/opt/vobcopy-win64` and builds `vobcopy.exe`.  The final section shows how to
assemble the executable and required DLLs into a runnable Windows directory.

## Prerequisites

Install the native build tools and the Windows cross compiler:

```sh
sudo apt update
sudo apt install autoconf automake gcc-mingw-w64-x86-64 meson ninja-build pkg-config
```

The commands below assume these source directories:

```text
/path/to/libdvdcss
/path/to/libdvdread
/path/to/vobcopy
```

Set the variables to different locations if necessary:

```sh
WORKDIR=/tmp/vobcopy-win64
PREFIX=/opt/vobcopy-win64
DVDCSS_SRC=/path/to/libdvdcss
DVDREAD_SRC=/path/to/libdvdread
VOBCOPY_SRC=/path/to/vobcopy
mkdir -p "$WORKDIR"
```

Create the Meson cross file:

```sh
cat > "$WORKDIR/mingw64.ini" <<'EOF'
[binaries]
c = 'x86_64-w64-mingw32-gcc'
ar = 'x86_64-w64-mingw32-ar'
strip = 'x86_64-w64-mingw32-strip'
pkg-config = 'pkg-config'

[host_machine]
system = 'windows'
cpu_family = 'x86_64'
cpu = 'x86_64'
endian = 'little'
EOF
```

## Build and install libdvdcss

`libdvdcss` is required below because `libdvdread` is built to link to it.

```sh
meson setup "$WORKDIR/libdvdcss-build" "$DVDCSS_SRC" \
  --cross-file "$WORKDIR/mingw64.ini" --prefix "$PREFIX" \
  -Ddefault_library=both
meson compile -C "$WORKDIR/libdvdcss-build"
sudo meson install -C "$WORKDIR/libdvdcss-build"
```

This installs `libdvdcss-2.dll`, static and import libraries, headers, and a
pkg-config file under `$PREFIX`.

## Build and install libdvdread

Make its cross-build find the Windows `libdvdcss`, rather than the host
library:

```sh
PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig" \
meson setup "$WORKDIR/libdvdread-build" "$DVDREAD_SRC" \
  --cross-file "$WORKDIR/mingw64.ini" --prefix "$PREFIX" \
  -Ddefault_library=both -Dlibdvdcss=enabled

GIT_DIR="$DVDREAD_SRC/.git" GIT_WORK_TREE="$DVDREAD_SRC" \
  meson compile -C "$WORKDIR/libdvdread-build"
sudo meson install -C "$WORKDIR/libdvdread-build"
```

The explicit `GIT_DIR` and `GIT_WORK_TREE` are needed by the current
`libdvdread` Meson build when it generates `ChangeLog` from an out-of-tree
build directory.

## Build vobcopy

Use a clean `vobcopy` working tree. `configure` must see the Windows headers
and import library, so pass the prefix explicitly:

```sh
cd "$VOBCOPY_SRC"
autoreconf --install --force
CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib" \
  ./configure --build="$(gcc -dumpmachine)" --host=x86_64-w64-mingw32
make
```

The result is `vobcopy.exe` in the repository root. Tests for the
cross-compiled Windows binaries are not covered by this build procedure.

## Create a runnable Windows directory

`vobcopy.exe` depends on `libdvdread-8.dll`; that DLL in turn depends on
`libdvdcss-2.dll`. Put all three in one directory before copying it to
Windows:

```sh
DIST="$WORKDIR/dist"
install -d "$DIST"
install -m 0755 "$VOBCOPY_SRC/vobcopy.exe" "$DIST/vobcopy.exe"
install -m 0755 "$PREFIX/bin/libdvdread-8.dll" "$DIST/libdvdread-8.dll"
install -m 0755 "$PREFIX/bin/libdvdcss-2.dll" "$DIST/libdvdcss-2.dll"
```

On Windows, pass the DVD drive using its Windows form, for example
`vobcopy -i E:\`.

`libdvdcss` emits the following pointer-size warnings when built for 64-bit
Windows:

```text
warning: cast to pointer from integer of different size
    DeviceIoControl((HANDLE) i_fd, ...)

warning: cast from pointer to integer of different size
    dvdcss->i_fd = (int) h_fd;

warning: cast to pointer from integer of different size
    CloseHandle((HANDLE) dvdcss->i_fd);

warning: cast to pointer from integer of different size
    ReadFile((HANDLE) dvdcss->i_fd, ...);
```

These warnings originate in the upstream libdvdcss Windows implementation,
which stores Windows handles in an `int`. The resulting binaries have been
validated by successfully ripping an encrypted DVD on Windows x86_64.
