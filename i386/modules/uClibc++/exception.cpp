/*	Copyright (C) 2004 Garrett A. Kajmowicz

	This file is part of the uClibc++ Library.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <exception>
#include <func_exception>
#include <stdexcept>
#include <cstdlib>

//We can't do this yet because gcc is too stupid to be able to handle
//different implementations of exception class.

//#undef __UCLIBCXX_EXCEPTION_SUPPORT__

#ifdef __UCLIBCXX_EXCEPTION_SUPPORT__

namespace std{
	_UCXXEXPORT static const char * __std_exception_what_value = "exception";

	//We are providing our own versions to be sneaky


	_UCXXEXPORT exception::~exception() throw(){
		//Empty function
	}

	_UCXXEXPORT const char* exception::what() const throw(){
		return __std_exception_what_value;
	}

	_UCXXEXPORT bad_exception::~bad_exception() throw(){

	}
	
}
#else
namespace std{
	_UCXXEXPORT exception::~exception() {
		abort();
	}
	
	_UCXXEXPORT const char* exception::what() const {
		abort();
	}
	
	_UCXXEXPORT bad_exception::~bad_exception() {
		abort();
	}
	
}
#endif

void
std::terminate()
{
#ifdef __UCLIBCXX_EXCEPTION_SUPPORT__
    try
    {
#endif  // __UCLIBCXX_EXCEPTION_SUPPORT__
        (*std::terminate_handler())();
        // handler should not return
		abort ();
#ifdef __UCLIBCXX_EXCEPTION_SUPPORT__
    }
    catch (...)
    {
        // handler should not throw exception
        abort ();
    }
#endif  // __UCLIBCXX_EXCEPTION_SUPPORT__
}
