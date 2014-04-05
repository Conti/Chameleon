/*
 *  cham-mklayout.c
 *  Chameleon
 *
 *  Created by JrCs on 30/08/11.
 *  Copyright 2011. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include "stdint.h"
#include "term.h"
#include "Keylayout.h"

#define PACKAGE_NAME	"chameleon"
#define PROGRAM_VERSION	"1.0"

static struct keyboard_layout default_layout = {
	.keyboard_map = {
		/* 0x00 */ 0, KEY_ESC, '1', '2', '3', '4', '5', '6',
		/* 0x08 */ '7', '8', '9', '0', '-', '=', KEY_BKSP, KEY_TAB,
		/* 0x10 */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
		/* 0x18 */ 'o', 'p', '[', ']', KEY_ENTER,  0, 'a', 's',
		/* 0x20 */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
		/* 0x28 */ '\'', '`',  0, '\\', 'z', 'x', 'c', 'v',
		/* 0x30 */ 'b', 'n', 'm', ',', '.', '/', 0, 0
	},
	.keyboard_map_shift = {
		/* 0x00 */ 0, KEY_ESC, '!', '@', '#', '$', '%', '^',
		/* 0x08 */ '&', '*', '(', ')', '_', '+', KEY_BKSP, 0,
		/* 0x10 */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
		/* 0x18 */ 'O', 'P', '{', '}', KEY_ENTER,  0, 'A', 'S',
		/* 0x20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
		/* 0x28 */ '"', '~',  0, '|', 'Z', 'X', 'C', 'V',
		/* 0x30 */ 'B', 'N', 'M', '<', '>', '?', 0, 0,
	},
	.keyboard_map_alt = {
		/* 0x00 */ 0, KEY_NOECHO, 0, 0, 0, 0, 0, 0,
		/* 0x08 */ 0, 0, 0, 0, 0, 0, KEY_NOECHO, KEY_NOECHO,
		/* 0x10 */ 0, 0, 0, 0, 0, 0, 0, 0,
		/* 0x18 */ 0, 0, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, 0, 0,
		/* 0x20 */ 0, 0, 0, 0, 0, 0, 0, KEY_NOECHO,
		/* 0x28 */ KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, 0, 0, 0, 0,
		/* 0x30 */ 0, 0, 0, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO, KEY_NOECHO,
	},
};

const char* program_name;

static struct option options[] = {
	{"input",   required_argument, 0, 'i'},
	{"output",  required_argument, 0, 'o'},
	{"keysyms", no_argument, 0, 'k'},
	{"help",    no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},
	{0, 0, 0, 0}
};

struct hash
{
  const char *keysym;
  int        code;
};

static struct hash keysym_to_code[] = {
	{"BackSpace",	KEY_BKSP},
	{"Tab",			KEY_TAB},
	{"Linefeed",	0x000a},
	{"Space",		0x0020},
	{"Exclam",		0x0021},
	{"QuoteDbl",	0x0022},
	{"NumberSign",	0x0023},
	{"Dollar",		0x0024},
	{"Percent",		0x0025},
	{"Ampersand",	0x0026},
	{"Apostrophe",	0x0027},
	{"Parenleft",	0x0028},
	{"Parenright",	0x0029},
	{"Asterisk",	0x002a},
	{"Plus",		0x002b},
	{"Comma",		0x002c},
	{"Minus",		0x002d},
	{"Period",		0x002e},
	{"Slash",		0x002f},
	{"Zero",		0x0030},
	{"One",			0x0031},
	{"Two",			0x0032},
	{"Three",		0x0033},
	{"Four",		0x0034},
	{"Five",		0x0035},
	{"Six",			0x0036},
	{"Seven",		0x0037},
	{"Eight",		0x0038},
	{"Nine",		0x0039},
	{"Colon",		0x003a},
	{"SemiColon",	0x003b},
	{"Less",		0x003c},
	{"Equal",		0x003d},
	{"Greater",		0x003e},
	{"Question",	0x003f},
	{"At",			0x0040},
	{"BracketLeft", 0x005b},
	{"Backslash",	0x005c},
	{"BracketRight",0x005d},
	{"AsciiCircum", 0x005e},
	{"Underscore",	0x005f},
	{"Grave",		0x0060},
	{"BraceLeft",	0x007b},
	{"Bar",			0x007c},
	{"BraceRight",	0x007d},
	{"AsciiTilde",	0x007e},
	{"Enter",		KEY_ENTER},
	{"Escape",		KEY_ESC},
	{"PrintScreen",	KEY_PRTSC},
	{"Left",		KEY_LEFT},
	{"Right",		KEY_RIGHT},
	{"Up",			KEY_UP},
	{"Down",		KEY_DOWN},
	{"Home",		KEY_HOME},
	{"End",			KEY_END},
	{"Center",		KEY_CENTER},
	{"Insert",		KEY_INS},
	{"Delete",		KEY_DEL},
	{"PageUp",		KEY_PGUP},
	{"PageDown",	KEY_PGDN},
	{"F1",			KEY_F1},
	{"F2",			KEY_F2},
	{"F3",			KEY_F3},
	{"F4",			KEY_F4},
	{"F5",			KEY_F5},
	{"F6",			KEY_F6},
	{"F7",			KEY_F7},
	{"F8",			KEY_F8},
	{"F9",			KEY_F9},
	{"F10",			KEY_F10},
	{"F11",			KEY_F11},
	{"F12",			KEY_F12},
	{"VoidSymbol",	KEY_NOECHO},
	{NULL, 0}
};


static void usage (int status) {
  if (status)
    fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
  else
    printf ("\
Usage: %s [OPTIONS]\n\
  -i, --input		set input filename.\n\
  -o, --output		set output filename.\n\
  -k, --keysyms		list recognized keysyms.\n\
  -h, --help		display this message and exit.\n\
  -V, --version		print version information and exit.\n\
\n", program_name);

  exit (status);
}

static void list_keysyms (void) {
	int i;
	for (i = 0; keysym_to_code[i].keysym != NULL; i++) {
		printf("0x%04x\t %s\n", keysym_to_code[i].code,  keysym_to_code[i].keysym);
	}
}

static int hash_lookup (const char *keysym, unsigned int *code) {
	int i;
	for (i = 0; keysym_to_code[i].keysym != NULL; i++) {
		if (strcasecmp (keysym, keysym_to_code[i].keysym) == 0) {
			*code = keysym_to_code[i].code;
			return 0;
		}
	}
	return -1;
}

static int parse_keysym(const char* str, unsigned int *code) {
	// Single character
	if (strlen(str) == 1) {
		*code=(unsigned char)(str[0]);
		if (*code > ASCII_KEY_MASK)
			return -1;
		return 0;
	}

	// Hex character code
	if (sscanf(str, "0x%x", code))
		return 0;

	// Keysym
	if (hash_lookup(str, code) == 0)
		return 0;

	return -1;
}

int parse_scancode(int linenum, char* code, unsigned int* scancode) {

	char* tmp;
	errno = 0; // clear errno

	float parse_value = strtof(code, &tmp);
	int rest = strlen(code) - ((void*) tmp - (void*)code);
	if (rest != 0 || errno != 0) {
		printf("Ignoring line %d: invalid scancode `%s'\n", linenum, code);
		return -1;
	}

	*scancode = parse_value;

	// Only the first scancodes can be translated
	if (*scancode >= KEYBOARD_MAP_SIZE) {
		printf("Ignoring line %d: invalid scancode 0x%02x. Scancode must be <= 0x%02x.\n", linenum, *scancode,
			   KEYBOARD_MAP_SIZE);
		return -1;
	}
	return 0;
}

void assign_keycode(int linenum, unsigned int scancode, const char* arg, uint16_t* map) {
	unsigned int value;
	if (parse_keysym(arg, &value) == -1) {
		printf("Warning line %d (keycode 0x%02x): invalid symbol %s (must be a true ascii character)\n",
			   linenum, scancode, arg);
		value = KEY_NOECHO; // VoidSymbol
	}
	map[scancode] = value;
}

struct keyboard_layout* create_keylayout(FILE* in) {
	char line[128], code[sizeof(line)];
	char arg1[sizeof(line)], arg2[sizeof(line)], arg3[sizeof(line)],
		 arg4[sizeof(line)];
	int  n, linenum = 0;
	unsigned int scancode;

	struct keyboard_layout* new_layout= malloc(sizeof(*new_layout));
	if (!new_layout)
		return NULL;

	// Initialize new keybord layout
	memcpy(new_layout, &default_layout, sizeof(*new_layout));

	while(fgets(line, sizeof(line), in)) {
		linenum++;
		n = sscanf (line, "keycode %s = %s %s %s %s", code, arg1, arg2, arg3, arg4);
		if (n > 1) {
			if (parse_scancode(linenum, code, &scancode) == -1)
				continue;
			if (n >= 2)
				assign_keycode(linenum, scancode, arg1,
							   new_layout->keyboard_map);
			if (n >= 3)
				assign_keycode(linenum, scancode, arg2,
							   new_layout->keyboard_map_shift);
			if (n >= 4)
				assign_keycode(linenum, scancode, arg3,
							   new_layout->keyboard_map_alt);
			if (n >= 5)
				assign_keycode(linenum, scancode, arg4,
							   new_layout->keyboard_map_shift_alt);
		} else if (sscanf (line, "shift keycode %s = %s", code, arg1) == 2) {
			if (parse_scancode(linenum, code, &scancode) == -1)
				continue;
			assign_keycode(linenum, scancode, arg1,
						   new_layout->keyboard_map_shift);
		} else if (sscanf (line, "alt keycode %s = %s", code, arg1) == 2) {
			if (parse_scancode(linenum, code, &scancode) == -1)
				continue;
			assign_keycode(linenum, scancode, arg1,
						   new_layout->keyboard_map_alt);
		} else if (sscanf (line, "shift alt keycode %s = %s", code, arg1) == 2) {
			if (parse_scancode(linenum, code, &scancode) == -1)
				continue;
			assign_keycode(linenum, scancode, arg1,
						   new_layout->keyboard_map_shift_alt);
		}
	}
	return new_layout;
}

void write_layout(struct keyboard_layout* layout, FILE* out) {
	// Create the header
	uint32_t version = KEYBOARD_LAYOUTS_VERSION;
	fwrite(KEYBOARD_LAYOUTS_MAGIC, KEYBOARD_LAYOUTS_MAGIC_SIZE, 1, out);
	fwrite(&version, 1, sizeof(version), out);
	// Seek to start of layout
	fseek(out, KEYBOARD_LAYOUTS_MAP_OFFSET, SEEK_SET);
	// Write layout
	fwrite(layout, sizeof(*layout), 1, out);
}

void set_program_name(const char* arg) {
	const char* last_slash;
	last_slash = strrchr(arg, '/');
	program_name = (last_slash != NULL) ? last_slash + 1 : arg;
}

int main (int argc, char *argv[]) {
	int ch;
	char *infile_name  = NULL;
	char *outfile_name = NULL;
	FILE *in, *out;

	set_program_name (argv[0]);

	/* Check for options.  */
	while ((ch = getopt_long (argc, argv, "i:o:hkV", options, 0)) != -1) {
		switch (ch) {
			case 'h':	
				usage (0);
				break;
				
			case 'k':
				list_keysyms();
				exit(0);
				break;

			case 'i':
				infile_name = optarg;
				break;

			case 'o':
				outfile_name = optarg;
				break;
				
			case 'V':
				printf ("%s v%s (%s)\n", program_name, PROGRAM_VERSION, PACKAGE_NAME);
				return 0;
				
			default:
				usage (1);
				break;
		}
	}
	
	if (infile_name == NULL) {
		fprintf(stderr, "You must specify an input file\n");
		usage(1);
	}

	if (outfile_name == NULL) {
		fprintf(stderr, "You must specify an output file\n");
		usage(1);
	}
	
	in = fopen (infile_name, "r");
	if (!in) {
		fprintf(stderr, "Couldn't open input file `%s': %s\n",
				infile_name, strerror (errno));
		exit(1);
    }

	out = fopen (outfile_name, "wb");
	if (!out) {
		fprintf(stderr, "Couldn't open output file `%s': %s\n",
				outfile_name, strerror (errno));
		exit(1);
    }

	struct keyboard_layout* new_layout = create_keylayout(in);
	if (new_layout) {
		write_layout(new_layout, out);
        free(new_layout);
        new_layout = NULL;
    }

    fclose(out);
	fclose(in);

	return 0;
}
