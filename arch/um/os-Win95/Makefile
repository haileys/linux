# Don't instrument UML-specific code
KCOV_INSTRUMENT                := n

obj-y = main.o os.o signal.o start_up.o process.o

CFLAGS_main.o += -Wno-frame-larger-than

USER_OBJS := $(user-objs-y) main.o os.o signal.o start_up.o process.o

include $(srctree)/arch/um/scripts/Makefile.rules
