
DIR = boot1
include ../MakePaths.dir

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

#NASM = /Developer/usr/bin/nasm
NASM = $(shell which nasm)

VERSIONED_FILES = boot1h

VERS = `vers_string -f 5.0 | tr - .`
NEW_VERS = Darwin boot1h v$(VERS)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif

all embedtheme: $(DIRS_NEEDED) $(VERSIONED_FILES)

boot1h: boot1.s Makefile
	@echo "\t[NASM] boot1.s"
	@$(NASM) boot1.s -o $(SYMROOT)/boot1h
	@echo "\t[NASM] boot1hp.s"
	@$(NASM) boot1hp.s -o $(SYMROOT)/boot1hp
	@echo "\t[NASM] boot1he.s"
	@$(NASM) boot1he.s -o $(SYMROOT)/boot1he
	@echo "\t[NASM] boot1f32.s"
	@$(NASM) boot1f32.s -o $(SYMROOT)/boot1f32

install_i386:: all $(INSTALLDIR)
	cp $(SYMROOT)/boot1h $(INSTALLDIR)/
	cd $(INSTALLDIR); chmod u+w $(VERSIONED_FILES) 

include ../MakeInc.dir
