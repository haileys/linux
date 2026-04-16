# Don't instrument UML-specific code
KCOV_INSTRUMENT                := n

ARCH_DIR := arch/um
HOST_DIR := arch/$(HEADER_ARCH)

obj-y = \
	boot_console.o \
	main.o \
	os.o \
	process.o \
	signal.o \
	start_up.o \
	string.o

ifdef CONFIG_WSL9X_RUNTIME
obj-y += runtime.o
user-objs-y += runtime.o
else
obj-y += vxdwraps.o
endif

win9x_user_cflags = \
	-isystem $(srctree)/tools/include/nolibc \
	-isystem $(srctree)/$(HOST_DIR)/include/generated/uapi \
	-isystem $(srctree)/$(HOST_DIR)/include/uapi \
	-isystem $(srctree)/include/uapi \
	-idirafter $(srctree)/$(ARCH_DIR)/include/generated \
	-idirafter $(srctree)/$(HOST_DIR)/include/generated

CFLAGS_runtime.o += $(win9x_user_cflags)

USER_OBJS := $(user-objs-y)

include $(srctree)/arch/um/scripts/Makefile.rules
