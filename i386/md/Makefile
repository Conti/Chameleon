#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/util
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot


DIR = util
include ${SRCROOT}/Make.rules

PROGRAMS = md 
OBJS = md.o32_nodep #md.o64_nodep 


OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

LDFLAGS := $(LDFALGS) -mdynamic-no-pic -Wall -dead_strip -mmacosx-version-min=10.5

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMPROG) all-recursive

$(SYMPROG): $(OBJS)
	@echo "\t[LD32] $(@F)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F) $(OBJROOT)/$(@F).o32_nodep	

#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@for p in $(SYMPROG); do if [ -f "$${p}" ];then echo "\t[RM] $${p}"; fi; done
	@rm -f $(SYMPROG) $(OBJS)
