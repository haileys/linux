# Don't instrument UML-specific code
KCOV_INSTRUMENT                := n

ARCH_DIR := arch/um
HOST_DIR := arch/$(HEADER_ARCH)

obj-y = \
	main.o \
	mem.o \
	os.o \
	process.o \
	runtime.o \
	signal.o \
	start_up.o \
	string.o \
	tls.o \

win9x_user_cflags = \
	-isystem $(srctree)/tools/include/nolibc \
	-isystem $(srctree)/$(HOST_DIR)/include/generated/uapi \
	-isystem $(srctree)/$(HOST_DIR)/include/uapi \
	-isystem $(srctree)/include/uapi \
	-idirafter $(srctree)/$(ARCH_DIR)/include/generated \
	-idirafter $(srctree)/$(HOST_DIR)/include/generated

CFLAGS_main.o += -Wno-frame-larger-than $(win9x_user_cflags)
CFLAGS_runtime.o += $(win9x_user_cflags)
CFLAGS_os.o += $(win9x_user_cflags)

USER_OBJS := $(user-objs-y) main.o runtime.o os.o

include $(srctree)/arch/um/scripts/Makefile.rules
