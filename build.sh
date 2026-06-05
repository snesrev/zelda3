#!/usr/bin/env bash
# Config-aware build helper for the Zelda3 co-op project (Linux/gcc).
#   ./build.sh vanilla   -> zelda3        (no multiplayer define; regression baseline)
#   ./build.sh coop      -> zelda3_coop   (-DZELDA3_MULTIPLAYER=1)
#   ./build.sh harness   -> zelda3_harness(-DZELDA3_MULTIPLAYER=1 -DZELDA3_HEADLESS_TEST=1, excludes main.c)
# Objects go to obj/<cfg>/ so the three configs never clobber each other.
# Incremental: recompiles a .c when it (or any header, or this script) is newer than its .o.
# NOTE: never run `make clean` here -- it deletes zelda3_assets.dat. This script never touches assets.
set -uo pipefail
cd "$(dirname "$0")"
CFG="${1:-coop}"
CC="${CC:-gcc}"
JOBS="${JOBS:-$(nproc)}"
CFLAGS="-O2 -Werror -I. $(sdl2-config --cflags) -D_REENTRANT -DSYSTEM_VOLUME_MIXER_AVAILABLE=0"
LIBS="$(sdl2-config --libs) -lm"

case "$CFG" in
  vanilla) DEF="";                                              OUT="zelda3";         EXCLUDE="src/test_harness.c";;
  coop)    DEF="-DZELDA3_MULTIPLAYER=1";                        OUT="zelda3_coop";    EXCLUDE="src/test_harness.c";;
  harness) DEF="-DZELDA3_MULTIPLAYER=1 -DZELDA3_HEADLESS_TEST=1"; OUT="zelda3_harness"; EXCLUDE="src/main.c";;
  *) echo "usage: $0 [vanilla|coop|harness]" >&2; exit 2;;
esac

OBJDIR="obj/$CFG"; mkdir -p "$OBJDIR"
SELF="${BASH_SOURCE[0]}"
NEWEST_HDR="$(ls -t src/*.h snes/*.h third_party/*/*.h 2>/dev/null | head -1)"
SRCS="$(ls src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c 2>/dev/null | grep -vF "$EXCLUDE")"
FAILFLAG="$OBJDIR/.failed"; rm -f "$FAILFLAG"

OBJS=""; n=0
for src in $SRCS; do
  obj="$OBJDIR/$(echo "$src" | tr '/' '_').o"
  OBJS="$OBJS $obj"
  if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ] || [ "$SELF" -nt "$obj" ] || { [ -n "$NEWEST_HDR" ] && [ "$NEWEST_HDR" -nt "$obj" ]; }; then
    ( $CC $CFLAGS $DEF -c "$src" -o "$obj" || { echo "FAILED: $src"; touch "$FAILFLAG"; } ) &
    n=$((n+1)); if [ "$n" -ge "$JOBS" ]; then wait -n 2>/dev/null || wait; n=$((n-1)); fi
  fi
done
wait
if [ -f "$FAILFLAG" ]; then echo "BUILD FAILED ($CFG)"; rm -f "$FAILFLAG"; exit 1; fi
$CC $OBJS -o "$OUT" $LIBS || { echo "LINK FAILED ($CFG)"; exit 1; }
echo "BUILT $OUT ($CFG)"
