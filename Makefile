p=RELEASE
all:
	nasm -f elf -g -F stabs  boot/boot.s -o bin/boot.o
	cd bin && gcc -c -D ${p} -m32 -g -ggdb -fcf-protection=none -mmanual-endbr -no-pie -fno-pic -nostdinc -fno-builtin -fno-stack-protector ../boot/*.c ../kernel/*.c ../common/*.c
	ld -T kernel.ld -m elf_i386 -nostdlib bin/*.o -o bin/kernel
	mkdir /g
	losetup -P /dev/loop18  hd
	mount /dev/loop18p1 /g
	cp bin/kernel /g/kernel
	umount /g
	losetup -d /dev/loop18
	rm -r /g
clean:
	rm ./bin/*
debug:
	qemu-system-i386 -S -s -hda hd
run:
	qemu-system-i386 -hda hd
bochs:
	bochs -q -f bochsrc.bxrc