#pragma once

#include <linux/compiler_attributes.h>
#ifndef VXD
# ifndef __UM_HOST__
#  include <linux/types.h>
# else
#  include <stdint.h>
# endif
#endif
