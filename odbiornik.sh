#./cmake-build-debug/odbiornik -b 200000 -R 60 | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -q -
./cmake-build-debug/odbiornik -b 524288 $@ | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 16394 -q -
#./cmake-build-debug/odbiornik -b 524288 -R 60 | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -q -
#./cmake-build-debug/odbiornik -b 524288 -R 60 | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 16394 -q -
