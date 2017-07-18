#!/bin/bash
set -x
gcc -g -static mail-file.o 'libesmtp-config --libs' -o mail-file-a
