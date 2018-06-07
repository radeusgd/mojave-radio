#!/usr/bin/env bash
cat /dev/stdin | pv -q -L $((44100*4)) | ./cmake-build-debug/nadajnik -a 239.10.11.12 -n "$1" -P $(( ((RANDOM<<15)|RANDOM) % 63001 + 2000 ))
