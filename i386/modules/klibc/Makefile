MODULE_NAME = klibc
MODULE_VERSION = "1.5.20"
MODULE_COMPAT_VERSION = "1.5.20"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = klibc

MODULE_OBJS = 	klibc.o \
		__ashldi3.o __ashrdi3.o __clzsi2.o __divdi3.o __divsi3.o \
		__lshrdi3.o __moddi3.o __modsi3.o __udivdi3.o \
		__udivmoddi4.o __udivmodsi4.o __udivsi3.o \
		__umoddi3.o __umodsi3.o \
		strntoumax.o strntoimax.o atoi.o atol.o atoll.o \
		strcasecmp.o strncasecmp.o  strdup.o strlcat.o strndup.o strnlen.o \
		strsep.o strtoimax.o strtok_r.o strtok.o strtol.o strtoll.o strtotimespec.o strtotimeval.o \
		strtoul.o strtoull.o strtoumax.o strxspn.o strpbrk.o \
		bsearch.o calloc.o \
		jrand48.o lrand48.o mrand48.o srand48.o nrand48.o seed48.o \
		memccpy.o memchr.o memmem.o memmove.o memrchr.o memswap.o  \
		qsort.o sha1hash.o onexit.o atexit.o exit.o \
		snprintf.o vsnprintf.o sscanf.o vsscanf.o\

OPTIM = -Os -Oz
DEBUG = -DNOTHING
#DEBUG = -DDEBUG_HELLO_WORLD=1
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost \
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(BOOT2DIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld
# LIBS= -lc_static
LIBS=

VPATH = $(OBJROOT):$(SYMROOT)

		


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme optionrom: dylib

include ../MakeInc.dir