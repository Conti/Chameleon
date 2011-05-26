SRCROOT = $(shell pwd)/../../
OBJROOT = $(SRCROOT)/obj/i386/boot0
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = boot0
include ${SRCROOT}/Make.rules

DIRS_NEEDED = $(SYMROOT)


OBJECTS  = boot0 chain0
OBJECTS := $(addprefix $(SYMROOT)/, $(OBJECTS))

all: $(DIRS_NEEDED) $(OBJECTS)

$(OBJECTS): $(SRCROOT)/autoconf.inc
	@echo "\t[NASM] $(@F)"
	@$(NASM) $(@F).s -o $@

#dependencies

