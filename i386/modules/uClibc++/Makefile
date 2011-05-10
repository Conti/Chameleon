
MODULE_NAME = uClibc++
MODULE_VERSION = "0.2.2"
MODULE_COMPAT_VERSION = "0.2.2"
MODULE_START = _uClibcxx_start
MODULE_DEPENDENCIES = klibc

DIR = uClibc++
# sstream.o fstream.o
MODULE_OBJS = uClibc++.o abi.o algorithm.o associative_base.o bitset.o \
		complex.o del_op.o del_opnt.o del_opv.o del_opvnt.o \
		deque.o eh_alloc.o eh_globals.o exception.o \
		func_exception.o iomanip.o  char_traits.o \
		iterator.o limits.o list.o locale.o \
		map.o new_handler.o new_op.o new_opnt.o new_opv.o \
		new_opvnt.o numeric.o queue.o set.o \
		stack.o stdexcept.o streambuf.o string.o typeinfo.o \
		utility.o valarray.o vector.o support.o \
		ios.o iostream.o istream.o ostream.o 

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
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(BOOT2DIR) -Iinclude
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