#
# Copyright (C) 2000 Jeff Dike (jdike@karaya.com)
# Licensed under the GPL
#

hostfs-objs := hostfs_kern.o

ifndef CONFIG_WIN9X
hostfs-builtin-$(CONFIG_HOSTFS) += hostfs_user.o hostfs_user_exp.o
endif

obj-y := $(hostfs-builtin-y) $(hostfs-builtin-m)
obj-$(CONFIG_HOSTFS) += hostfs.o

include $(srctree)/arch/um/scripts/Makefile.rules
