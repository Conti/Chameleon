/*
 * Copyright (c) 2010 Evan Lojewski. All rights reserved.
 *	
 *	dyldsymboltool
 *
 *		Generates a dylib file for the dyld implimentation in chameleon
 *	to load and link. This is used to import the boot symbols into the
 *	module system. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <sys/file.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>


#define DYLIB_NAME "Symbols"
#define VOID_SYMBOL "_load_all_modules"
#define START_SYMBOL "start"

typedef struct symbols_dylib
{
	struct mach_header		header;
	struct dylib_command	dylib_info;
	char					module_name[sizeof(DYLIB_NAME)];	
	struct symtab_command	symtab;
} symbols_dylib_t;


typedef struct symbolList_t
{
	char*					name;
	uint32_t				addr;
	int						pos;
	struct symbolList_t*	next;
} symbolList_t;


int num_symbols(symbolList_t* list);
int string_size(symbolList_t* list);
void add_symbol(symbolList_t** list, char* name, uint32_t addr);


int main(int argc, char *argv[])
{
	if(argc < 3) 
	{
		fprintf(stderr, "usage: dyldsymboltool obj1 [obj2 ...] outfile\n");

		exit(-1);
	}

	

	symbols_dylib_t dylib;
	symbolList_t*	symbols = NULL;

	uint32_t start_addr = 0;
	
	
	
	
    int i;
	for(i = 1; i < argc-1; i++)
    {
        char line[256];
        char* command = malloc(strlen(argv[1]) + sizeof("nm -g "));
        FILE *fpipe;

        // Parse boot.sys (arg1) to get symtab
        sprintf(command, "nm -g %s", argv[i]);	// TODO: read boot.sym directly, no need for nm
        
        if ( !(fpipe = (FILE*)popen(command,"r")) )
        {  // If fpipe is NULL
            perror("Problems with pipe");
            exit(1);
        }
        
        
        while ( fgets( line, sizeof line, fpipe))
        {
            if((strlen(line) < strlen(argv[i]) ||
               strncmp(line, argv[i], strlen(argv[i])) != 0)
                && line[0] != ' ')
            {
                uint32_t address = 0;
                char* addr = strtok(line, " ");
                strtok(NULL, " ");
                char* name = strtok(NULL, " ");
                name[strlen(name)-1] = 0;	// remove newline
                sscanf(addr, "%x", &address);
                if(strcmp(name, VOID_SYMBOL) == 0) 
				{
					start_addr = address;
				}
				if(strcmp(name, START_SYMBOL) == 0)
				{
					if(!start_addr) start_addr = address;
				}
                else add_symbol(&symbols, name, address);
            }
        }        
        
        pclose(fpipe);
        
        free(command);
    }
    
    
	
	if(start_addr == 0)
	{
		fprintf(stderr, "Unable to locate Symbol.dylib start function\n");
        
        //exit(1);
	}
    else
    {
        add_symbol(&symbols, START_SYMBOL, start_addr);
	}
	
	/* Header command info */
	dylib.header.ncmds = 2;
	dylib.header.sizeofcmds = sizeof(dylib) - sizeof(struct mach_header);// +  dylib.symtab.nsyms * sizeof(struct nlist) + dylib.symtab.strsize;
	
	dylib.header.magic = MH_MAGIC;
	dylib.header.cputype = CPU_TYPE_X86;
	dylib.header.cpusubtype = /*CPUSUBTYPE_I386*/ 3;
	dylib.header.filetype = MH_DYLIB;
	dylib.header.flags = MH_NOUNDEFS | MH_DYLDLINK | MH_NO_REEXPORTED_DYLIBS;
		
	/* Load Commands - dylib id */
	dylib.dylib_info.cmd = LC_ID_DYLIB;
	dylib.dylib_info.cmdsize = sizeof(struct dylib_command) + sizeof(dylib.module_name);	// todo: verify
	dylib.dylib_info.dylib.name.offset = sizeof(struct dylib_command);
	dylib.dylib_info.dylib.timestamp = 0;	// TODO: populate with time
	dylib.dylib_info.dylib.current_version = 0;			// TODO
	dylib.dylib_info.dylib.compatibility_version = 0;	// TODO
	
	
	//int offset = dylib.dylib_info.cmdsize%4 ? 4 - (dylib.dylib_info.cmdsize % 4) : 0;
	//dylib.dylib_info.cmdsize += offset;
	//dylib.header.sizeofcmds += offset;
	
	sprintf(dylib.module_name, "%s", DYLIB_NAME);
	
	/* Load Commands - Symtable */
	dylib.symtab.cmd = LC_SYMTAB;
	dylib.symtab.symoff = sizeof(dylib);	
	dylib.symtab.nsyms = num_symbols(symbols);
	dylib.symtab.stroff = sizeof(dylib) + dylib.symtab.nsyms * sizeof(struct nlist);
	dylib.symtab.strsize = string_size(symbols);
	dylib.symtab.cmdsize = sizeof(struct symtab_command);
	
	
    
	FILE* outfile = fopen(argv[argc-1], "w");
	fwrite(&dylib,	sizeof(dylib)	/* Sizeof header + module name */
					, 1, outfile);
	
	char* symtab = malloc(dylib.symtab.stroff + dylib.symtab.strsize - sizeof(dylib) + 1); // Add extra 1 for last symbol
	bzero(symtab, dylib.symtab.nsyms * sizeof(struct nlist) + dylib.symtab.strsize + 1);
	char* orig = symtab;
	
	//symtab += offset;
	
	
	while(symbols)
	{
		
		((struct nlist*)symtab)->n_un.n_strx = symbols->pos;
		((struct nlist*)symtab)->n_type = 0xF;	// TODO: read from boot.sys
		((struct nlist*)symtab)->n_sect = 0;
		((struct nlist*)symtab)->n_desc = REFERENCE_FLAG_DEFINED;
		((struct nlist*)symtab)->n_value = (uint32_t)symbols->addr;
		symtab+= sizeof(struct nlist);
		
		strcpy(orig + dylib.symtab.stroff - sizeof(dylib) + symbols->pos, symbols->name);

		symbols = symbols->next;
	}

	fwrite(orig,	 
					dylib.symtab.stroff	+					// Sizeof symbol nlists 
					dylib.symtab.strsize - sizeof(dylib) + 1	// sizeof symbol strings
					, 1, outfile);

	
	fclose(outfile);
	
	exit(0);
}

int num_symbols(symbolList_t* list)
{
	int retVal = 0;
	while(list)
	{
		retVal++;
		list = list->next;
	}
	return retVal;
}

int string_size(symbolList_t* list)
{
	int retVal = 0;
	while(list)
	{
		retVal += strlen(list->name)+1;
		list = list->next;
	}
	return retVal;
	
}

void add_symbol(symbolList_t** list, char* name, uint32_t addr)
{
	symbolList_t* entry = malloc(sizeof(symbolList_t));
	entry->next = (*list);
	
	if(*list) entry->pos = (*list)->pos + strlen((*list)->name) + 1;
	else entry->pos = 1;
	*list = entry;
	
	entry->addr = addr;
	entry->name = malloc(strlen(name)+1);
	strcpy(entry->name, name);
}
