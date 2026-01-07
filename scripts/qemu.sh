qemu-system-aarch64 -M raspi4b -serial null -serial stdio -kernel kernel8.img \
-drive file=sdcard.img,format=raw,if=sd