
#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

PNGCRUSH = `which pngcrush`
PNGOPTIONS = -rem cHRM -rem gAMA -rem iCCP -rem sRGB

DIR = boot2
include ../MakePaths.dir

OPTIM = -Os -Oz
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost -Werror \
		-fno-builtin -DSAIO_INTERNAL_USER -static $(OMIT_FRAME_POINTER_CFLAG) \
		-mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
		-march=pentium4 -msse2 -mfpmath=sse -msoft-float

DEFINES=
CONFIG = hd
SYMDIR = $(SYMROOT)
LIBSADIR = ../libsa
LIBSAIODIR = ../libsaio
UTILDIR = ../util
THEME = embed
THEMEDIR = ../../artwork/themes/$(THEME)
INC = -I. -I.. -I$(SYMDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(UTILDIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = gcc
# LIBS= -lc_static
LIBS= -L$(SYMDIR) -lsaio -lsa
LIBDEP= $(SYMDIR)/libsaio.a $(SYMDIR)/libsa.a

OTHER_FILES =

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
VPATH = $(OBJROOT):$(SYMROOT)

# The ordering is important;
# boot2.o must be first.
OBJS = boot2.o boot.o graphics.o drivers.o prompt.o options.o lzss.o mboot.o \
	ramdisk.o picopng.o resume.o bmdecompress.o graphic_utils.o gui.o modules.o modules_support.o
# button.o browser.o scrollbar.o == NOTYET	

UTILDIR = ../util
SFILES = boot2.s
CFILES = boot.c graphics.c drivers.c prompt.c options.c mboot.c picopng.c modules.c
HFILES = boot.h appleClut8.h appleboot.h mboot.h multiboot.h picopng.h modules.h
OTHERFILES = Makefile
ALLSRC = $(FOREIGNSRC) $(FOREIGNBIN) $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)
BOOT2ADDR = 20200
MAXBOOTSIZE = 458240


#
# Strings used to retrieve the start location for the Symbols.dylib module
#
SYMBOLS_MODULE = Symbols.dylib
SYMBOL_START= _symbols_module_start
SYMBOL_ADDR = $(shell printf "%d" 0x`nm -s __DATA __data $(SYMROOT)/boot.sys | grep " $(SYMBOL_START)$$" | cut -f 1 -d " "`)
DATA_OFFSET = $(shell otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | tail -n 1 | cut -f 6 -d " ")
DATA_ADDR   = $(shell printf "%d" `otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | head -n 2 | tail -n 1 | cut -f 8 -d " "`)
PATCH_ADDR  = $(shell echo ${SYMBOL_ADDR}-${DATA_ADDR}+${DATA_OFFSET} | bc)


all: $(DIRS_NEEDED) boot

embedtheme: CFLAGS += -DEMBED_THEME
embedtheme: art.h all

boot: machOconv embedded.h $(OBJS) $(LIBDEP)
	@echo "\t[LD] boot.sys"
	@$(LD) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) $(LIBS) -lcc_kext

	@cp  $(SYMROOT)/boot.sys  $(SYMROOT)/boot2.sys

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@echo "\t[LD] boot.sys"
	@$(LD) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) $(LIBS)  -lcc_kext

	@# Second pass, fixup global var locations
	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@${RM} $(SYMROOT)/boot.sys
	@echo "\t[LD] boot.sys"
	@$(LD) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) $(LIBS)  -lcc_kext

	@ld -arch i386 \
	-undefined dynamic_lookup \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-final_output Symbols \
	$(filter %.o,$^) $(LIBS) \
	-o $(OBJROOT)/Symbols_LINKER_ONLY.dylib



	@make embed_symbols		# this is done in a sub process after boot.sys exists so the strings are populated correctly

	@${RM} $(SYMROOT)/boot2.sys

	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@##size $(SYMROOT)/boot.sys
	@ls -l $(SYMROOT)/boot
	@( size=`ls -l $(SYMROOT)/boot | awk '{ print $$5}'` ; \
	  if expr "$$size" ">" "$(MAXBOOTSIZE)" > /dev/null ;\
	  then \
	    echo "Booter executable larger than $(MAXBOOTSIZE) bytes" ;\
	    rm $(SYMROOT)/boot ;\
	    exit 1;\
	  fi)

	
embed_symbols: machOconv
	@echo ================= Embedding Symbols.dylib =================
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot2.sys $(SYMROOT)/boot &> /dev/null

	@echo "\t******* Patching at $(PATCH_ADDR) ******"
	@stat -f%z $(SYMROOT)/boot | perl -ane "print pack('V',@F[0]);" | dd conv=notrunc of=${SYMROOT}/boot.sys bs=1 count=4 seek=$(PATCH_ADDR)	 &> /dev/null
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot.sys $(SYMROOT)/boot

	

prompt.o: vers.h
vers.h:
	@echo "#define I386BOOT_VERSION \"5.0.132\"" > $(SYMROOT)/vers.h
	@echo "#define I386BOOT_BUILDDATE \"`date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $(SYMROOT)/vers.h
	@echo "#define I386BOOT_CHAMELEONVERSION \"`cat ../../version`\"" >> $(SYMROOT)/vers.h
	@echo "#define I386BOOT_CHAMELEONREVISION \"`svnversion -n | tr -d [:alpha:]`\"" >> $(SYMROOT)/vers.h

art.h:
	@if [ "$(PNGCRUSH)" ]; then	\
		echo "optimizing art files ...\n$(PNGCRUSH) $(PNGOPTIONS) artwork/$(THEME)"; \
		cd $(THEMEDIR) && find . -name '*.png' -exec $(PNGCRUSH) -q $(PNGOPTIONS) -d $(SYMROOT)/embed {} \;; \
		cp theme.plist $(SYMROOT)/embed; \
	else \
		cp -R $(THEMEDIR) $(SYMROOT)/embed; \
	fi;
	@echo "#ifndef __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@echo "#define __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name 'theme.plist' -exec xxd -i >> $(SYMROOT)/art.h {} \;
	@cd $(SYMROOT)/embed && find . -name '*.png' -exec xxd -i >> $(SYMROOT)/art.h {} \;

	@echo "typedef struct {" >> $(SYMROOT)/art.h
	@echo "	char			name[32];" >> $(SYMROOT)/art.h
	@echo "	unsigned char	*pngdata;" >> $(SYMROOT)/art.h
	@echo "	unsigned int	*length;" >> $(SYMROOT)/art.h
	@echo "} embeddedpng_t;" >> $(SYMROOT)/art.h
	@echo "" >> $(SYMROOT)/art.h
	@echo "embeddedpng_t embeddedImages[] = {" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name '*.png' | sort | cut -f 2 -d '/' | cut -f 1 -d '.' | \
		awk '{ printf "\t{.name = \"%s\", .pngdata = __%s_png, .length = &__%s_png_len},\n", $$1, $$1, $$1 }' >> $(SYMROOT)/art.h
	@echo "};" >> $(SYMROOT)/art.h

	@echo "#endif /* !__BOOT2_ART_H */"  >> $(SYMROOT)/art.h ;\

embedded.h:
	@cd $(SYMROOT)/../../doc && xxd -i BootHelp.txt > $(SYMROOT)/embedded.h

install_i386:: all $(INSTALLDIR)
	cp $(SYMROOT)/boot $(OTHER_FILES) $(INSTALLDIR)
	cd $(INSTALLDIR); chmod u+w boot $(OTHER_FILES)

include ../MakeInc.dir

#dependencies
-include $(OBJROOT)/Makedep
