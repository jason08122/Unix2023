#!/bin/bash

# Clean the project
make clean

# Build the project
make

# Move the hellomod.ko file to a specific folder
cp kshram.ko ../dist/rootfs/
# mv kshram ../dist/rootfs/

# Remove the newrootfs.cpio.bz2 file
rm ../dist/newrootfs.cpio.bz2

cd ../dist/rootfs

find . | cpio -H newc -o | bzip2 > ../newrootfs.cpio.bz2

cd ..

sh ./qemu.sh
