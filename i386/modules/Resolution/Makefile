MODULE_NAME = Resolution
MODULE_AUTHOR = Chameleon
MODULE_DESCRIPTION = This module reads the edid information from the monitor attached to the main display. The module will also patch the vesa modes available in pre intel hd graphics cards to provide proper resolution while booting.
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 


DIR = Resolution

MODULE_OBJS = Resolution.o edid.o 915resolution.o

include ../MakeInc.dir