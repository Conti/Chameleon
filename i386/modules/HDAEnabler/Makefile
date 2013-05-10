MODULE_NAME = HDAEnabler
MODULE_AUTHOR = "ErmaC"
MODULE_DESCRIPTION = This module provides a remplacement for the DSDT edits like HDEF/HDAU
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES =

DIR = HDAEnabler



include ../MakeInc.dir

$(SYMROOT)/modules/$(MODULE_NAME).dylib:
	@echo "\t[CP] $(MODULE_NAME).dylib"
	@cp $(MODULE_NAME).dylib $(SRCROOT)/sym/i386/modules/$(MODULE_NAME).dylib

$(SRCROOT)/sym/i386/boot_modules.c:
	@
