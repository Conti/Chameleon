#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(shell pwd)/../../
OBJROOT = $(SRCROOT)/obj/i386/config
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = util
include ${SRCROOT}/Make.rules



OBJECTS = cconfig.o32 cconfig.o64 zconf.tab.o32 zconf.tab.o64 yesno.o32 yesno.o64 textbox.o32 textbox.o64 menubox.o32 menubox.o64 checklist.o32 checklist.o64 inputbox.o32 inputbox.o64

DEFINES = -DKBUILD_NO_NLS -DCURSES_LOC=\<ncurses.h\> -DPATH_MAX=256 -DPACKAGE=\"chameleon\" 
LDFLAGS = -lncurses -lmenu


PROGRAMS = cconfig



SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMPROG)




$(SYMPROG): $(addprefix $(OBJROOT)/, $(OBJECTS))
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/*.o32
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/*.o64
	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64

config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig

.PHONY: config

#dependencies
-include $(OBJROOT)/Makedep