#!/bin/sh
cd "$(dirname "$0")"
CROSS=powerpc-linux-gnu-
exec make CC=${CROSS}gcc LD=${CROSS}ld "$@"
