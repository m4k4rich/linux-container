# We put container filesystem in a file by using loop and try to create file in your new mount point.

dd if=/dev/zero of=$(pwd)/fs.img bs=1M count=1024

losetup /dev/loop42 $(pwd)/fs.img

mkfs.ext4 $(pwd)/fs.img

