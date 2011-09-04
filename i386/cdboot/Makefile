SRCROOT = $(CURDIR)/../../
OBJROOT = $(SRCROOT)/obj/i386/cdboot
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = cdboot
include ${SRCROOT}/Make.rules

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
DIRS_NEEDED = $(SYMROOT)

all embedtheme optionrom: $(DIRS_NEEDED) $(SYMROOT)/cdboot

$(SYMROOT)/cdboot:
	@echo "\t[NASM] cdboot.s"	
	@$(NASM) cdboot.s -o $(SYMROOT)/cdboot
	@dd if=$(SYMROOT)/boot of=$(SYMROOT)/cdboot conv=sync bs=2k seek=1 &> /dev/null

	@# Update cdboot with boot file size info
	@stat -f%z $(SYMROOT)/boot \
		| perl -ane "print pack('V',@F[0]);" \
		| dd of=$(SYMROOT)/cdboot bs=1 count=4 seek=2044 conv=notrunc &> /dev/null

#dependencies
