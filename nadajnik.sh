#!/usr/bin/env bash
sox -q "$1" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./cmake-build-debug/nadajnik -a 239.10.11.12 -n "$1" -P $(( ((RANDOM<<15)|RANDOM) % 63001 + 2000 )) -f 994288
#sox -q "$1" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./cmake-build-debug/nadajnik -a 239.10.11.12 -n "$1" -P $(( ((RANDOM<<15)|RANDOM) % 63001 + 2000 )) -f 994288 -R 60
