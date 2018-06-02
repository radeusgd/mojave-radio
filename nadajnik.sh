#!/usr/bin/env bash
sox -S "Big Iron.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | pv -q -L $((44100*4)) | ./cmake-build-debug/nadajnik -a 239.10.11.12 -n "Mojave Music Radio"