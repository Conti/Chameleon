#ifdef CONFIG_MODULES
#include <architecture/i386/asm_help.h>

LABEL(dyld_stub_binder)
	jmp		_dyld_stub_binder
	
LABEL(dyld_void_start)
	ret

#endif