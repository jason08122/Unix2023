
obj-m     += kshram.o
ccflags-y += -DEXPORT_SYMTAB

all: 
	make -C ../dist/dist/modulebuild M=$(PWD) modules

# kshram: kshram.c
# 	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static

# check_fork: check_fork.c
# 	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static
# check_msg: check_msg.c
# 	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static
# check_resize: check_resize.c
# 	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static
clean:
	rm -f kshram
	rm -f check_fork 
	rm -f check_msg 
	rm -f check_resize
	make -C ../dist/dist/modulebuild M=$(PWD) clean

install: all
	mkdir -p ../rootfs/modules
	cp kshram.ko ../rootfs/modules

# check_fork check_msg check_resize