/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#ifdef CONFIG_MODULES
#ifndef CONFIG_MODULE_DEBUG
#define CONFIG_MODULE_DEBUG 0
#endif

#include "boot.h"
#include "bootstruct.h"
#include "modules.h"
#include "boot_modules.h"


#if CONFIG_MODULE_DEBUG
#define DBG(x...)	printf(x);
#define DBGPAUSE()	getc()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif

// NOTE: Global so that modules can link with this
unsigned long long textAddress = 0;
unsigned long long textSection = 0;

void* symbols_module_start = (void*)0xFFFFFFFF;	// Global, value is populated by the makefile with actual address

/** Internal symbols, however there are accessor methods **/
moduleHook_t* moduleCallbacks = NULL;
moduleList_t* loadedModules = NULL;
symbolList_t* moduleSymbols = NULL;
unsigned int (*lookup_symbol)(const char*) = NULL;


/*
 * Initialize the module system by loading the Symbols.dylib module.
 * Once loaded, locate the _lookup_symbol function so that internal
 * symbols can be resolved.
 */
int init_module_system()
{
    // Start any modules that were compiled in first.
    start_built_in_modules();
    
    
	int retVal = 0;
	void (*module_start)(void) = NULL;
	char* module_data = symbols_module_start + BOOT2_ADDR;
    
	// Intialize module system
	if(symbols_module_start != (void*)0xFFFFFFFF)
	{
		// Module system  was compiled in (Symbols.dylib addr known)
		module_start = parse_mach(module_data, &load_module, &add_symbol);
		
		if(module_start && module_start != (void*)0xFFFFFFFF)
		{
			// Notify the system that it was laoded
			module_loaded(SYMBOLS_MODULE /*moduleName, moduleVersion, moduleCompat*/);
			
			(*module_start)();	// Start the module. This will point to load_all_modules due to the way the dylib was constructed.
			execute_hook("ModulesLoaded", NULL, NULL, NULL, NULL);
			DBG("Module %s Loaded.\n", SYMBOLS_MODULE);
			retVal = 1;

		}
		else
		{
            module_data -= 0x10;    // XCODE 4 HACK
            module_start = parse_mach(module_data, &load_module, &add_symbol);
            
            if(module_start && module_start != (void*)0xFFFFFFFF)
            {
                // Notify the system that it was laoded
                module_loaded(SYMBOLS_MODULE /*moduleName, moduleVersion, moduleCompat*/);
                
                (*module_start)();	// Start the module. This will point to load_all_modules due to the way the dylib was constructed.
                execute_hook("ModulesLoaded", NULL, NULL, NULL, NULL);
                DBG("Module %s Loaded.\n", SYMBOLS_MODULE);
                retVal = 1;
                
            }
            else
            {
                // The module does not have a valid start function
                printf("Unable to start %s\n", SYMBOLS_MODULE); getc();
            }		
		}		
	}
	return retVal;
}

void start_built_in_module(char* name, void(*start_function)(void))
{
    start_function();
    // Notify the module system that this module really exists, specificaly, let other module link with it
    module_loaded(name /*, moduleName, moduleVersion, moduleCompat*/);
}


/*
 * Load all modules in the /Extra/modules/ directory
 * Module depencdies will be loaded first
 * Modules will only be loaded once. When loaded  a module must
 * setup apropriete function calls and hooks as required.
 * NOTE: To ensure a module loads after another you may 
 * link one module with the other. For dyld to allow this, you must
 * reference at least one symbol within the module.
 */
void load_all_modules()
{
	char* name;
	long flags;
	long time;
	struct dirstuff* moduleDir = opendir("/Extra/modules/");
	while(readdir(moduleDir, (const char**)&name, &flags, &time) >= 0)
	{
		if(strcmp(&name[strlen(name) - sizeof("dylib")], ".dylib") == 0)
		{
			char* tmp = malloc(strlen(name) + 1);
			strcpy(tmp, name);
			
			if(!load_module(tmp))
			{
				// failed to load
				// free(tmp);
			}
		}
		else 
		{
			DBG("Ignoring %s\n", name);
		}

	}
}


/*
 * Load a module file in /Extra/modules/
 */
int load_module(char* module)
{
	int retVal = 1;
	void (*module_start)(void) = NULL;
	char modString[128];
	int fh = -1;

	// Check to see if the module has already been loaded
	if(is_module_loaded(module))
	{
		return 1;
	}
	
	sprintf(modString, MODULE_PATH "%s", module);
	fh = open(modString, 0);
	if(fh < 0)
	{
		DBG("WARNING: Unable to locate module %s\n", modString); DBGPAUSE();
		return 0;
	}
	
	unsigned int moduleSize = file_size(fh);
	char* module_base = (char*) malloc(moduleSize);
	if (moduleSize && read(fh, module_base, moduleSize) == moduleSize)
	{
		// Module loaded into memory, parse it
		module_start = parse_mach(module_base, &load_module, &add_symbol);

		if(module_start && module_start != (void*)0xFFFFFFFF)
		{
			// Notify the system that it was laoded
			module_loaded(module/*moduleName, moduleVersion, moduleCompat*/);
			(*module_start)();	// Start the module
			DBG("Module %s Loaded.\n", module); DBGPAUSE();
		}
#if CONFIG_MODULE_DEBUG
		else // The module does not have a valid start function. This may be a library.
		{
			printf("WARNING: Unable to start %s\n", module);
			getc();
		}
#else
		else msglog("WARNING: Unable to start %s\n", module);
#endif
	}
	else
	{
		DBG("Unable to read in module %s\n.", module); DBGPAUSE();
		retVal = 0;
	}

	close(fh);
	return retVal;
}

/*
 * add_symbol
 * This function adds a symbol from a module to the list of known symbols 
 * possibly change to a pointer and add this to the Symbol module so that it can
 * adjust it's internal symbol list (sort) to optimize locating new symbols
 * NOTE: returns the address if the symbol is "start", else returns 0xFFFFFFFF
 */
long long add_symbol(char* symbol, long long addr, char is64)
{
	if(is64) return  0xFFFFFFFF; // Fixme

	// This only can handle 32bit symbols 
	symbolList_t* entry;
	//DBG("Adding symbol %s at 0x%X\n", symbol, addr);
	
	entry = malloc(sizeof(symbolList_t));
	entry->next = moduleSymbols;
	moduleSymbols = entry;
	
	entry->addr = (UInt32)addr;
	entry->symbol = symbol;
	
	if(strcmp(symbol, "start") == 0)
	{
		return addr;
	}
	else
	{
		return 0xFFFFFFFF; // fixme
	}
}


/*
 * print out the information about the loaded module
 */
void module_loaded(const char* name/*, UInt32 version, UInt32 compat*/)
{
	moduleList_t* new_entry = malloc(sizeof(moduleList_t));
	new_entry->next = loadedModules;

	loadedModules = new_entry;
	
	new_entry->name = (char*)name;
//	new_entry->version = version;
//	new_entry->compat = compat;
}

int is_module_loaded(const char* name)
{
	// todo sorted search
	moduleList_t* entry = loadedModules;
	while(entry)
	{
		if(strcmp(entry->name, name) == 0)
		{
			DBG("Located module %s\n", name); DBGPAUSE();
			return 1;
		}
		else
		{
			entry = entry->next;
		}

	}
	
	DBG("Module %s not found\n", name); DBGPAUSE();
	return 0;
}

/*
 *	lookup symbols in all loaded modules. Thins inludes boot syms due to Symbols.dylib construction
 *
 */
unsigned int lookup_all_symbols(const char* name)
{
	symbolList_t* entry = moduleSymbols;
	while(entry)
	{
		if(strcmp(entry->symbol, name) == 0)
		{
			//DBG("External symbol %s located at 0x%X\n", name, entry->addr);
			return entry->addr;
		}
		else
		{
			entry = entry->next;
		}
	}
	
#if CONFIG_MODULE_DEBUG
	verbose("Unable to locate symbol %s\n", name);
	getc();
#endif
	
	if(strcmp(name, VOID_SYMBOL) == 0) return 0xFFFFFFFF;
	// In the event that a symbol does not exist
	// Return a pointer to a void function.
	else return lookup_all_symbols(VOID_SYMBOL);	
}

/********************************************************************************/
/*	Macho Parser																*/
/********************************************************************************/

/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header. If the module is sucessfuly laoded
 * the module iinit address will be returned.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available.
 */
void* parse_mach(void* binary, int(*dylib_loader)(char*), long long(*symbol_handler)(char*, long long, char))
{	
	char is64 = false;
	void (*module_start)(void) = NULL;
	
	// Module info
	/*char* moduleName = NULL;
	 UInt32 moduleVersion = 0;
	 UInt32 moduleCompat = 0;
	 */
	// TODO convert all of the structs to a union
	struct load_command *loadCommand = NULL;
	struct dylib_command* dylibCommand = NULL;
	struct dyld_info_command* dyldInfoCommand = NULL;
	
	struct symtab_command* symtabCommand = NULL;
	struct segment_command *segCommand = NULL;
	struct segment_command_64 *segCommand64 = NULL;
	
	//struct dysymtab_command* dysymtabCommand = NULL;
	UInt32 binaryIndex = 0;
	UInt16 cmd = 0;
	
	textSection = 0;
	textAddress = 0;	// reinitialize text location in case it doesn't exist;
	
	// Parse through the load commands
	if(((struct mach_header*)binary)->magic == MH_MAGIC)
	{
		is64 = false;
		binaryIndex += sizeof(struct mach_header);
	}
	else if(((struct mach_header_64*)binary)->magic == MH_MAGIC_64)
	{
		// NOTE: modules cannot be 64bit...
		is64 = true;
		binaryIndex += sizeof(struct mach_header_64);
	}
	else
	{
		verbose("Invalid mach magic 0x%X\n", ((struct mach_header*)binary)->magic);
		//getc();
		return NULL;
	}
	
	
	
	/*if(((struct mach_header*)binary)->filetype != MH_DYLIB)
	 {
	 printf("Module is not a dylib. Unable to load.\n");
	 getc();
	 return NULL; // Module is in the incorrect format
	 }*/
	
	while(cmd < ((struct mach_header*)binary)->ncmds)
	{
		cmd++;
		
		loadCommand = binary + binaryIndex;
		UInt32 cmdSize = loadCommand->cmdsize;
		
		
		switch ((loadCommand->cmd & 0x7FFFFFFF))
		{
			case LC_SYMTAB:
				symtabCommand = binary + binaryIndex;
				break;
				
			case LC_SEGMENT: // 32bit macho
				segCommand = binary + binaryIndex;
				
				if(strcmp("__TEXT", segCommand->segname) == 0)
				{
					UInt32 sectionIndex;
					
					sectionIndex = sizeof(struct segment_command);
					
					struct section *sect;
					
					while(sectionIndex < segCommand->cmdsize)
					{
						sect = binary + binaryIndex + sectionIndex;
						
						sectionIndex += sizeof(struct section);
						
						
						if(strcmp("__text", sect->sectname) == 0)
						{
							// __TEXT,__text found, save the offset and address for when looking for the calls.
							textSection = sect->offset;
							textAddress = sect->addr;
							break;
						}					
					}
				}
				
				break;
			case LC_SEGMENT_64:	// 64bit macho's
				segCommand64 = binary + binaryIndex;				
				if(strcmp("__TEXT", segCommand64->segname) == 0)
				{
					UInt32 sectionIndex;
					
					sectionIndex = sizeof(struct segment_command_64);
					
					struct section_64 *sect;
					
					while(sectionIndex < segCommand64->cmdsize)
					{
						sect = binary + binaryIndex + sectionIndex;
						
						sectionIndex += sizeof(struct section_64);
						
						
						if(strcmp("__text", sect->sectname) == 0)
						{
							// __TEXT,__text found, save the offset and address for when looking for the calls.
							textSection = sect->offset;
							textAddress = sect->addr;
							
							break;
						}					
					}
				}				
				
				break;
				
				
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB ^ LC_REQ_DYLD:
				dylibCommand  = binary + binaryIndex;
				char* module  = binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				// Possible enhancments: verify version
				// =	dylibCommand->dylib.current_version;
				// =	dylibCommand->dylib.compatibility_version;
				if(dylib_loader)
				{
					char* name = malloc(strlen(module) + strlen(".dylib") + 1);
					sprintf(name, "%s.dylib", module);
					
					if (!dylib_loader(name))
					{
						// NOTE: any symbols exported by dep will be replace with the void function
						free(name);
					}
				}
				
				break;
				
			case LC_ID_DYLIB:
				dylibCommand = binary + binaryIndex;
				/*moduleName =	binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				 moduleVersion =	dylibCommand->dylib.current_version;
				 moduleCompat =	dylibCommand->dylib.compatibility_version;
				 */
				break;
				
			case LC_DYLD_INFO:
			//case LC_DYLD_INFO_ONLY:	// compressed info, 10.6+ macho files, already handeled
				// Bind and rebase info is stored here
				dyldInfoCommand = binary + binaryIndex;
				break;
				
			case LC_DYSYMTAB:
			case LC_UUID:
			case LC_UNIXTHREAD:
				break;
				
			default:
				DBG("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
				break;
				
		}
		
		binaryIndex += cmdSize;
	}

	// bind_macho uses the symbols, if the textAdd does not exist (Symbols.dylib, no code), addresses are static and not relative
	module_start = (void*)handle_symtable((UInt32)binary, symtabCommand, symbol_handler, is64);
	
	if(dyldInfoCommand)
	{
		// Rebase the module before binding it.
		if(dyldInfoCommand->rebase_off)		rebase_macho(binary, (char*)dyldInfoCommand->rebase_off,	dyldInfoCommand->rebase_size);
		// Bind all symbols. 
		if(dyldInfoCommand->bind_off)		bind_macho(binary,   (char*)dyldInfoCommand->bind_off,		dyldInfoCommand->bind_size);
		if(dyldInfoCommand->weak_bind_off)	bind_macho(binary,   (char*)dyldInfoCommand->weak_bind_off,	dyldInfoCommand->weak_bind_size);
		if(dyldInfoCommand->lazy_bind_off)	bind_macho(binary,   (char*)dyldInfoCommand->lazy_bind_off,	dyldInfoCommand->lazy_bind_size);
	}
	
	return module_start;
	
}

/*
 * parse the symbol table
 * Lookup any undefined symbols
 */

unsigned int handle_symtable(UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, long long, char), char is64)
{
	unsigned int module_start	= 0xFFFFFFFF;	
	UInt32 symbolIndex			= 0;
	char* symbolString			= base + (char*)symtabCommand->stroff;

	if(!is64)
	{
		struct nlist* symbolEntry = (void*)base + symtabCommand->symoff;
		while(symbolIndex < symtabCommand->nsyms)
		{
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, textAddress ? (long long)base + symbolEntry->n_value : symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = textAddress ? base + symbolEntry->n_value : symbolEntry->n_value;
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
	}
	else
	{
		struct nlist_64* symbolEntry = (void*)base + symtabCommand->symoff;
		// NOTE First entry is *not* correct, but we can ignore it (i'm getting radar:// right now, verify later)	
		while(symbolIndex < symtabCommand->nsyms)
		{
			
			
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, textAddress ? (long long)base + symbolEntry->n_value : symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = textAddress ? base + symbolEntry->n_value : symbolEntry->n_value;
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
	}
	return module_start;
}

// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
void rebase_macho(void* base, char* rebase_stream, UInt32 size)
{
	rebase_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = 0;
	UInt32 segmentAddress = 0;
	
	
	UInt32 tmp  = 0;
	UInt32 tmp2  = 0;
	UInt8 bits = 0;
	int index = 0;
	unsigned int i = 0;
	
	while(i < size)
	{
		immediate = rebase_stream[i] & REBASE_IMMEDIATE_MASK;
		opcode = rebase_stream[i] & REBASE_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case REBASE_OPCODE_DONE:
				// Rebase complete, reset vars
				immediate = 0;
				opcode = 0;
				type = 0;
				segmentAddress = 0;
			default:
				break;
				
				
			case REBASE_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
				
				
			case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				// Locate address to begin rebasing
				segmentAddress = 0;
				struct segment_command* segCommand = NULL; // NOTE: 32bit only
				
				unsigned int binIndex = 0;
				index = 0;
				do
				{
					segCommand = base + sizeof(struct mach_header) +  binIndex;
					
					
					binIndex += segCommand->cmdsize;
					index++;
				}
				while(index <= immediate);
				
				
				segmentAddress = segCommand->fileoff;
				
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				segmentAddress += tmp;
				break;
				
				
			case REBASE_OPCODE_ADD_ADDR_ULEB:
				// Add value to rebase address
				tmp = 0;
				bits = 0;
				do
				{
					tmp <<= bits;
					tmp |= rebase_stream[++i] & 0x7f;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				segmentAddress +=	tmp; 
				break;
				
			case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
				segmentAddress += immediate * sizeof(void*);
				break;
				
				
			case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
				index = 0;
				for (index = 0; index < immediate; ++index) {
					rebase_location(base + segmentAddress, (char*)base, type);
					segmentAddress += sizeof(void*);
				}
				break;
				
				
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					//DBG("\tRebasing 0x%X\n", segmentAddress);
					rebase_location(base + segmentAddress, (char*)base, type);					
					segmentAddress += sizeof(void*);
				}
				break;
				
			case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				rebase_location(base + segmentAddress, (char*)base, type);
				
				segmentAddress += tmp + sizeof(void*);
				break;
				
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				
				tmp2 =  0;
				bits = 0;
				do
				{
					tmp2 |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					
					rebase_location(base + segmentAddress, (char*)base, type);
					
					segmentAddress += tmp2 + sizeof(void*);
				}
				break;
		}
		i++;
	}
}

inline void rebase_location(UInt32* location, char* base, int type)
{	
	switch(type)
	{
		case REBASE_TYPE_POINTER:
		case REBASE_TYPE_TEXT_ABSOLUTE32:
			*location += (UInt32)base;
			break;
			
		default:
			break;
	}
}


// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
// NOTE: this uses 32bit values, and not 64bit values. 
// There is a possibility that this could cause issues,
// however the modules are 32 bits, so it shouldn't matter too much
void bind_macho(void* base, char* bind_stream, UInt32 size)
{	
	bind_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = BIND_TYPE_POINTER;
	
	UInt32 segmentAddress = 0;
	
	UInt32 address = 0;
	
	SInt32 addend = 0;
	SInt32 libraryOrdinal = 0;
	
	const char* symbolName = NULL;
	UInt8 symboFlags = 0;
	UInt32 symbolAddr = 0xFFFFFFFF;
	
	// Temperary variables
	UInt8 bits = 0;
	UInt32 tmp = 0;
	UInt32 tmp2 = 0;
	UInt32 index = 0;
	unsigned int i = 0;
	
	while(i < size)
	{
		immediate = bind_stream[i] & BIND_IMMEDIATE_MASK;
		opcode = bind_stream[i] & BIND_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case BIND_OPCODE_DONE:
				// reset vars
				type = BIND_TYPE_POINTER;
				segmentAddress = 0;
				address = 0;
				addend = 0;
				libraryOrdinal = 0;
				symbolAddr = 0xFFFFFFFF;
			default:
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				libraryOrdinal = immediate;
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				libraryOrdinal = 0;
				bits = 0;
				do
				{
					libraryOrdinal |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				break;
				
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				libraryOrdinal = immediate ? (SInt8)(BIND_OPCODE_MASK | immediate) : immediate;				
				break;
				
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				symboFlags = immediate;
				symbolName = (char*)&bind_stream[++i];
				i += strlen((char*)&bind_stream[i]);

				symbolAddr = lookup_all_symbols(symbolName);
				break;
				
			case BIND_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
				
			case BIND_OPCODE_SET_ADDEND_SLEB:
				addend = 0;
				bits = 0;
				do
				{
					addend |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				if(!(bind_stream[i-1] & 0x40)) addend *= -1;
				break;
				
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segmentAddress = 0;
				
				// Locate address
				struct segment_command* segCommand = NULL;	// NOTE: 32bit only
				
				unsigned int binIndex = 0;
				index = 0;
				do
				{
					segCommand = base + sizeof(struct mach_header) +  binIndex;
					binIndex += segCommand->cmdsize;
					index++;
				}
				while(index <= immediate);
				
				segmentAddress = segCommand->fileoff;
				
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				segmentAddress += tmp;
				break;
				
			case BIND_OPCODE_ADD_ADDR_ULEB:
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				segmentAddress += tmp;
				break;
				
			case BIND_OPCODE_DO_BIND:
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
						
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getc();
				}
				
				segmentAddress += sizeof(void*);
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);

				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getc();
				}

				segmentAddress += tmp + sizeof(void*);
				
				
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getc();
				}
				segmentAddress += (immediate * sizeof(void*)) + sizeof(void*);
				
				
				break;
				
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				
				tmp2  = 0;
				bits = 0;
				do
				{
					tmp2 |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				
				if(symbolAddr != 0xFFFFFFFF)
				{
					for(index = 0; index < tmp; index++)
					{
						
						address = segmentAddress + (UInt32)base;
						bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
						segmentAddress += tmp2 + sizeof(void*);
					}
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getc();
				}
				break;
		}
		i++;
	}
}

inline void bind_location(UInt32* location, char* value, UInt32 addend, int type)
{	
	// do actual update
	char* newValue = value + addend;
	
	switch (type) {
		case BIND_TYPE_POINTER:
		case BIND_TYPE_TEXT_ABSOLUTE32:
			break;
			
		case BIND_TYPE_TEXT_PCREL32:
			newValue -=  ((UInt32)location + 4);
			
			break;
		default:
			return;
	}
	//DBG("Binding 0x%X to 0x%X (was 0x%X)\n", location, newValue, *location);
	*location = (UInt32)newValue;
}

/********************************************************************************/
/*	Module Hook Interface														*/
/********************************************************************************/

/*
* Locate the symbol for an already loaded function and modify the beginning of
* the function to jump directly to the new one
* example: replace_function("_HelloWorld_start", &replacement_start);
*/
int replace_function(const char* symbol, void* newAddress)
{
	UInt32* jumpPointer = malloc(sizeof(UInt32*));	 
	UInt32 addr = lookup_all_symbols(symbol);
	
	char* binary = (char*)addr;
	if(addr != 0xFFFFFFFF)
	{
		//DBG("Replacing %s to point to 0x%x\n", symbol, newAddress);
		*binary++ = 0xFF;	// Jump
		*binary++ = 0x25;	// Long Jump
		*((UInt32*)binary) = (UInt32)jumpPointer;
		
		*jumpPointer = (UInt32)newAddress;
		return 1;
	}
	return 0;
}


/*
 *	execute_hook(  const char* name )
 *		name - Name of the module hook
 *			If any callbacks have been registered for this hook
 *			they will be executed now in the same order that the
 *			hooks were added.
*/
int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
	DBG("Attempting to execute hook '%s'\n", name); DBGPAUSE();
	moduleHook_t* hook = hook_exists(name);
	
	if(hook)
	{
		// Loop through all callbacks for this module
		callbackList_t* callbacks = hook->callbacks;

		while(callbacks)
		{
			// Execute callback
			callbacks->callback(arg1, arg2, arg3, arg4);
			callbacks = callbacks->next;
		}
		DBG("Hook '%s' executed.\n", name); DBGPAUSE();
		return 1;
	}
	else
	{
		// Callback for this hook doesn't exist;
		DBG("No callbacks for '%s' hook.\n", name);
		return 0;
	}
}



/*
 *	register_hook_callback(  const char* name,  void(*callback)())
 *		name - Name of the module hook to attach to.
 *		callbacks - The funciton pointer that will be called when the
 *			hook is executed. When registering a new callback name, the callback is added sorted.
 *			NOTE: the hooks take four void* arguments.
 */
void register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*))
{	
	DBG("Adding callback for '%s' hook.\n", name); DBGPAUSE();
	
	moduleHook_t* hook = hook_exists(name);
	
	if(hook)
	{
		// append
		callbackList_t* newCallback = malloc(sizeof(callbackList_t));
		newCallback->next = hook->callbacks;
		hook->callbacks = newCallback;
		newCallback->callback = callback;
	}
	else
	{
		// create new hook
		moduleHook_t* newHook = malloc(sizeof(moduleHook_t));		
		newHook->name = name;
		newHook->callbacks = malloc(sizeof(callbackList_t));
		newHook->callbacks->callback = callback;
		newHook->callbacks->next = NULL;
		
		newHook->next = moduleCallbacks;
		moduleCallbacks = newHook;
		
	}
	
#if CONFIG_MODULE_DEBUG
	//print_hook_list();
	//getc();
#endif
	
}


moduleHook_t* hook_exists(const char* name)
{
	moduleHook_t* hooks = moduleCallbacks;
	
	// look for a hook. If it exists, return the moduleHook_t*,
	// If not, return NULL.
	while(hooks)
	{
		if(strcmp(name, hooks->name) == 0)
		{
			//DBG("Located hook %s\n", name);
			return hooks;
		}
		hooks = hooks->next;
	}
	//DBG("Hook %s does not exist\n", name);
	return NULL;
	
}

#if CONFIG_MODULE_DEBUG
void print_hook_list()
{
	printf("---Hook Table---\n");
	
	moduleHook_t* hooks = moduleCallbacks;
	while(hooks)
	{
		printf("Hook: %s\n", hooks->name);
		hooks = hooks->next;
	}
}
#endif

/********************************************************************************/
/*	dyld / Linker Interface																*/
/********************************************************************************/

void dyld_stub_binder()
{
	printf("ERROR: dyld_stub_binder was called, should have been take care of by the linker.\n");
	getc();
}

#else /* CONFIG_MODULES */

int init_module_system()
{
    return 0;
}

void load_all_modules()
{
    
}

int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
    return 0;
}
#endif