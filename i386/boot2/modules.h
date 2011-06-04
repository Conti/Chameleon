/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <saio_types.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>


#ifndef __BOOT_MODULES_H
#define __BOOT_MODULES_H

#define MODULE_PATH		"/Extra/modules/"

#define SYMBOLS_MODULE "Symbols.dylib"
#define SYMBOLS_AUTHOR "Chameloen"
#define SYMBOLS_DESCRIPTION "Chameleon symbols for linking"
#define SYMBOLS_VERSION     0
#define SYMBOLS_COMPAT      0

#define VOID_SYMBOL		"dyld_void_start"
extern UInt64 textAddress;
extern UInt64 textSection;



typedef struct symbolList_t
{
	char* symbol;
	UInt64 addr;
	struct symbolList_t* next;
} symbolList_t;


typedef struct callbackList_t
{
	void(*callback)(void*, void*, void*, void*);
	struct callbackList_t* next;
} callbackList_t;

typedef struct moduleHook_t
{
	const char* name;
	callbackList_t* callbacks;
	struct moduleHook_t* next;
} moduleHook_t;

typedef struct modulesList_t
{
	const char*				name;
    const char*             author;
    const char*             description;
	UInt32					version;
	UInt32					compat;
	struct modulesList_t* next;
} moduleList_t;



int init_module_system();
void load_all_modules();

void start_built_in_module(const char* name, 
                           const char* author, 
                           const char* description,
                           UInt32 version,
                           UInt32 compat,
                           void(*start_function)(void));

int load_module(char* module);
int is_module_loaded(const char* name);
void module_loaded(const char* name, const char* author, const char* description, UInt32 version, UInt32 compat);




/********************************************************************************/
/*	Symbol Functions															*/
/********************************************************************************/
long long		add_symbol(char* symbol, long long addr, char is64);
unsigned int	lookup_all_symbols(const char* name);



/********************************************************************************/
/*	Macho Parser																*/
/********************************************************************************/
void*			parse_mach(void* binary, 
							int(*dylib_loader)(char*),
							long long(*symbol_handler)(char*, long long, char),
                            void (*section_handler)(char* section, char* segment, long long offset, long long address)
                           );
unsigned int	handle_symtable(UInt32 base,
							 struct symtab_command* symtabCommand,
							 long long(*symbol_handler)(char*, long long, char),
							 char is64);
void			rebase_macho(void* base, char* rebase_stream, UInt32 size);
inline void		rebase_location(UInt32* location, char* base, int type);
void			bind_macho(void* base, UInt8* bind_stream, UInt32 size);
inline void		bind_location(UInt32* location, char* value, UInt32 addend, int type);




/********************************************************************************/
/*	Module Interface														*/
/********************************************************************************/
int				replace_function(const char* symbol, void* newAddress);
int				execute_hook(const char* name, void*, void*, void*, void*);
void			register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*));
moduleHook_t*	hook_exists(const char* name);

#if DEBUG_MODULES
void			print_hook_list();
#endif

/********************************************************************************/
/*	dyld Interface																*/
/********************************************************************************/
void dyld_stub_binder();

#endif /* __BOOT_MODULES_H */