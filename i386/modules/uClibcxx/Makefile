MODULE_NAME = uClibcxx
MODULE_DESCRIPTION = This module provides a minimalistic c++ runtime library for use in other modules. This does not provide functionality by itself, instead it is used to allow for the c++ language to be used in other modules. Please note that rtti and exceptions has both been disabled. 
MODULE_AUTHOR = 
MODULE_VERSION = "0.2.2"
MODULE_COMPAT_VERSION = "0.2.2"
MODULE_START = $(MODULE_NAME)_start
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

include ../MakeInc.dir