#!/bin/bash
clang -O3 -target x86_64-apple-macos10.12 -o  ofd_x64 entry_point.c
clang -O3 -target arm64-apple-macos11     -o  ofd_arm entry_point.c
lipo -create -output ofd ofd_x64 ofd_arm
rm ofd_x64
rm ofd_arm