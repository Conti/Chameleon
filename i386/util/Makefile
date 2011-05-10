#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#

DIR = util
include ../MakePaths.dir

VPATH = $(OBJROOT):$(SYMROOT)

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
LOCALBIN = $(DSTROOT)/usr/local/bin

OPTIM = -Os -Oz
CFLAGS = $(RC_CFLAGS) $(OPTIM) -Wmost -Werror -g
LDFLAGS =
CFILES = machOconv.c
ALLSRC = $(CFILES) $(MFILES) $(HFILES) $(EXPORT_HFILES)

PROGRAMS = machOconv bdmesg dyldsymboltool

OUTFILES = $(PROGRAMS)

DIRS_NEEDED = $(OBJROOT) $(SYMROOT) $(LANGDIR)

all embedtheme: $(DIRS_NEEDED) $(PROGRAMS)

dyldsymboltool: dyldsymboltool.o32 dyldsymboltool.o64
	@echo "\t[LD32] $@_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $@_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64


machOconv: machOconv.o32 machOconv.o64
	@echo "\t[LD32] $@_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $@_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64


bdmesg: bdmesg.o32 bdmesg.o64
	@echo "\t[LD32] $@_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5 \
		 -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $@_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5 \
		 -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64


include ../MakeInc.dir

#dependencies
-include $(OBJROOT)/Makedep