/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "bootstruct.h"
#include "libsaio.h"
#include "boot.h"
#include "xml.h"

extern char *Language;
extern char *LoadableFamilies;

bool sysConfigValid;

/*
 * Compare a string to a key with quoted characters
 */
static inline int
keyncmp(const char *str, const char *key, int n)
{
    int c;
    while (n--) {
	c = *key++;
	if (c == '\\') {
	    switch(c = *key++) {
	    case 'n':
		c = '\n';
		break;
	    case 'r':
		c = '\r';
		break;
	    case 't':
		c = '\t';
		break;
	    default:
		break;
	    }
	} else if (c == '\"') {
	    /* Premature end of key */
	    return 1;
	}
	if (c != *str++) {
	    return 1;
	}
    }
    return 0;
}

#if UNUSED

static void eatThru(char val, const char **table_p)
{
	register const char *table = *table_p;
	register bool found = false;

	while (*table && !found)
	{
		if (*table == '\\') table += 2;
		else
		{
			if (*table == val) found = true;
			table++;
		}
	}
	*table_p = table;
}

/* Remove key and its associated value from the table. */

bool
removeKeyFromTable(const char *key, char *table)
{
    register int len;
    register char *tab;
    char *buf;

    len = strlen(key);
    tab = (char *)table;
    buf = (char *)malloc(len + 3);

    sprintf(buf, "\"%s\"", key);
    len = strlen(buf);

    while(*tab) {
        if(strncmp(buf, tab, len) == 0) {
            char c;

            while((c = *(tab + len)) != ';') {
                if(c == 0) {
                    len = -1;
                    goto out;
                }
                len++;
            }
            len++;
            if(*(tab + len) == '\n') len++;
            goto out;
        }
        tab++;
    }
    len = -1;
out:
    free(buf);

    if(len == -1) return false;

    while((*tab = *(tab + len))) {
        tab++;
    }

    return true;
}

char *newStringFromList(char **list, int *size)
{
	char *begin = *list, *end;
	char *newstr;
	int newsize = *size;
	int bufsize;
    
	while (*begin && newsize && isspace(*begin)) {
		begin++;
		newsize--;
	}
	end = begin;
	while (*end && newsize && !isspace(*end)) {
		end++;
		newsize--;
	}
	if (begin == end) {
		return 0;
	}
	bufsize = end - begin + 1;
	newstr = malloc(bufsize);
	strlcpy(newstr, begin, bufsize);
	*list = end;
	*size = newsize;
	return newstr;
}

#endif

/* 
 * compress == compress escaped characters to one character
 */
int stringLength(const char *table, int compress)
{
	int ret = 0;

	while (*table) {
		if (*table == '\\') {
			table += 2;
			ret += 1 + (compress ? 0 : 1);
		} else {
			if (*table == '\"') {
				return ret;
			}
			ret++;
			table++;
		}
	}
	return ret;
}


bool getValueForConfigTableKey(config_file_t *config, const char *key, const char **val, int *size)
{
	if (config->dictionary != 0 ) {
		// Look up key in XML dictionary
		TagPtr value;
		value = XMLGetProperty(config->dictionary, key);
		if (value != 0) {
			if (value->type != kTagTypeString) {
				error("Non-string tag '%s' found in config file\n", key);
				return false;
			}
			*val = value->string;
			*size = strlen(value->string);
			return true;
		}
	} else {

		// Legacy plist-style table

	}

	return false;
}

#if UNUSED

/*
 * Returns a new malloc'ed string if one is found
 * in the string table matching 'key'.  Also translates
 * \n escapes in the string.
 */
char *newStringForStringTableKey(char *table, char *key, config_file_t *config)
{
	const char *val;
	char *newstr, *p;
	int size;
    
	if (getValueForConfigTableKey(config, key, &val, &size)) {
		newstr = (char *)malloc(size+1);
		for (p = newstr; size; size--, p++, val++) {
			if ((*p = *val) == '\\') {
				switch (*++val) {
					case 'r':
						*p = '\r';
					break;
					case 'n':
						*p = '\n';
					break;
					case 't':
						*p = '\t';
					break;
					default:
						*p = *val;
					break;
				}
				size--;
			}
		}
		*p = '\0';
		return newstr;
	} else {
		return 0;
	}
}

#endif

char *
newStringForKey(char *key, config_file_t *config)
{
	const char *val;
	char *newstr;
	int size;
    
	if (getValueForKey(key, &val, &size, config) && size) {
		newstr = (char *)malloc(size + 1);
		strlcpy(newstr, val, size + 1);
		return newstr;
	} else {
		return 0;
	}
}

/* parse a command line
 * in the form: [<argument> ...]  [<option>=<value> ...]
 * both <option> and <value> must be either composed of
 * non-whitespace characters, or enclosed in quotes.
 */

static const char *getToken(const char *line, const char **begin, int *len)
{
	if (*line == '\"') {
		*begin = ++line;
		while (*line && *line != '\"') {
			line++;
		}
		*len = line++ - *begin;
	} else {
		*begin = line;
		while (*line && !isspace(*line) && *line != '=') {
			line++;
		}
		*len = line - *begin;
	}
	return line;
}

bool getValueForBootKey(const char *line, const char *match, const char **matchval, int *len)
{
	const char *key, *value;
	int key_len, value_len;
	bool retval = false;
    
	while (*line) {
		/* look for keyword or argument */
		while (isspace(*line)) {
			line++;
		}

		/* now look for '=' or whitespace */
		line = getToken(line, &key, &key_len);
		/* line now points to '=' or space */
		if (*line && !isspace(*line)) {
			line = getToken(++line, &value, &value_len);
		} else {
			value = line;
			value_len = 0;
		}
		if ((strlen(match) == key_len) && strncmp(match, key, key_len) == 0) {
			// create a new string
			char* newstr = malloc(value_len + 1);
			strncpy(newstr, value, value_len);
			newstr[value_len] = 0;
		
			*matchval = newstr;
			*len = value_len;
			retval = true;
			/* Continue to look for this key; last one wins. */
		}
	}

	return retval;
}

/* Return NULL if no option has been successfully retrieved, or the string otherwise */
const char * getStringForKey(const char * key,  config_file_t *config)
{
	static const char* value =0;
	int len=0;
	if(!getValueForKey(key, &value, &len, config)) {
		value = 0;
	}
	return value;
}


/* Returns TRUE if a value was found, FALSE otherwise.
 * The boolean value of the key is stored in 'val'.
 */

bool getBoolForKey( const char *key, bool *result_val, config_file_t *config )
{
    const char *key_val;
    int size;
    
	if (getValueForKey(key, &key_val, &size, config)) {
		if ( (size >= 1) && (key_val[0] == 'Y' || key_val[0] == 'y') ) {
			*result_val = true;
		} else {
			*result_val = false;
		}
		return true;
	}
	return false;
}

bool getIntForKey( const char *key, int *value, config_file_t *config )
{
	const char *val;
	int size, sum;
	bool negative = false;
    
	if (getValueForKey(key, &val, &size, config)) {
		if ( size ) {
			if (*val == '-') {
				negative = true;
				val++;
				size--;
			}
			
			for (sum = 0; size > 0; size--) {
				if (*val < '0' || *val > '9') {
					return false;
				}
				
				sum = (sum * 10) + (*val++ - '0');
			}
			
			if (negative) {
				sum = -sum;
			}
			*value = sum;
			return true;
		}
	}
	return false;
}

/*
 *
 */

bool getDimensionForKey( const char *key, unsigned int *value, config_file_t *config, unsigned int dimension_max, unsigned int object_size )
{
	const char *val;

	int size = 0;
	int sum = 0;

	bool negative = false;
	bool percentage = false;

	if (getValueForKey(key, &val, &size, config)) {
		if ( size ) {
			if (*val == '-') {
				negative = true;
				val++;
				size--;
			}

			if (val[size-1] == '%') {
				percentage = true;
				size--;
			}

			// convert string to integer
			for (sum = 0; size > 0; size--) {
				if (*val < '0' || *val > '9') {
					return false;
				}

				sum = (sum * 10) + (*val++ - '0');
			}

			if (percentage) {
				sum = ( dimension_max * sum ) / 100;
			}

			// calculate offset from opposite origin
			if (negative) {
				sum =  ( ( dimension_max - object_size ) - sum );
			}

		} else {

			// null value calculate center
			sum = ( dimension_max - object_size ) / 2;

		}

		*value = (uint16_t) sum;
		return true;
	}

	// key not found
	return false;
}

/*
 *	get color value from plist format #RRGGBB
 */

bool getColorForKey( const char *key, unsigned int *value, config_file_t *config )
{
	const char *val;
	int size;

	if (getValueForKey(key, &val, &size, config))
	{
		if (*val == '#') {
			val++;
			*value = strtol(val, NULL, 16);
			return true;
		}
	}
	return false;
}

bool getValueForKey( const char *key, const char **val, int *size, config_file_t *config )
{
	const char *overrideVal;
	int overrideSize;
	bool override, ret;

	if (getValueForBootKey(bootArgs->CommandLine, key, val, size)) {
		return true;
	}

	ret = getValueForConfigTableKey(config, key, val, size);

	// Try to find alternate keys in bootInfo->chameleonConfig (if config can be overriden)
	// and prefer its values with the exceptions for
	// "Kernel"="mach_kernel" and "Kernel Flags"="".

	if (config->canOverride) {
		if (getValueForConfigTableKey(&bootInfo->chameleonConfig, key, &overrideVal, &overrideSize)) {
			override = true;

			// NOTE: Values are defined by apple as being in com.apple.Boot.plist
			//        kHelperRootUUIDKey, kKernelArchKey, kMKextCacheKey, kKernelCacheKey, kKernelNameKey, kKernelFlagsKey
			if (ret && (strcmp(key, kKernelNameKey) == 0) && (overrideSize == 0)) {
				override = false;
			}

			if (ret && (strcmp(key, kKernelFlagsKey) == 0) && (overrideSize == 0)) {
				override = false;
			}

			if (override) {
				*val = overrideVal;
				*size = overrideSize;
				return true;
			}
		}
	}
	return ret;
}


#if UNUSED
void
printSystemConfig(char *p1)
{
	char *p2 = p1, tmp;

	while (*p1 != '\0') {
		while (*p2 != '\0' && *p2 != '\n') {
			p2++;
		}
	tmp = *p2;
	*p2 = '\0';
	printf("%s\n", p1);
	*p2 = tmp;
	if (tmp == '\0') {
		break;
	}
	p1 = ++p2;
    }
}
#endif

//==========================================================================
// ParseXMLFile
// Modifies the input buffer.
// Expects to see one dictionary in the XML file.
// Puts the first dictionary it finds in the
// tag pointer and returns 0, or returns -1 if not found
// (and does not modify dict pointer).
// Prints an error message if there is a parsing error.
//
int ParseXMLFile( char * buffer, TagPtr * dict )
{
	long       length, pos;
	TagPtr     tag;
	pos = 0;
	char       *configBuffer;
  
	configBuffer = malloc(strlen(buffer)+1);
	strcpy(configBuffer, buffer);

	while (1) {
		length = XMLParseNextTag(configBuffer + pos, &tag);
			if (length == -1) {
				break;
			}

		pos += length;

		if (tag == 0) {
			continue;
		}
		if (tag->type == kTagTypeDict) {
			break;
		}

		XMLFreeTag(tag);
	}
	free(configBuffer);
	if (length < 0) {
		error ("Error parsing plist file\n");
		return -1;
	}
	*dict = tag;
	return 0;
}

/* loadConfigFile
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadConfigFile (const char *configFile, config_file_t *config)
{
	int fd, count;

	if ((fd = open_bvdev("bt(0,0)", configFile, 0)) < 0) {
		return -1;
	}
	// read file
	count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
	close(fd);

	// build xml dictionary
	ParseXMLFile(config->plist, &config->dictionary);
	return 0;
}


/* loadSystemConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadSystemConfig(config_file_t *config)
{
	char *dirspec[] = {
		"/Library/Preferences/SystemConfiguration/com.apple.Boot.plist",
		"/OS X Install Data/com.apple.Boot.plist",
	};

	int i, fd, count, ret=-1;

	for(i = 0; i< sizeof(dirspec)/sizeof(dirspec[0]); i++) {
		if ((fd = open(dirspec[i], 0)) >= 0) {
			// read file
			count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
			close(fd);

			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;	
			ret=0;

			break;
		}
	}

	if(ret == -1) ret = loadHelperConfig(config);

	// Always enable canOverride flag (for SystemConfig)
	config->canOverride = true;

	return ret;
}

/* loadChameleonConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadChameleonConfig(config_file_t *config)
{
	char *dirspec[] = {
		"rd(0,0)/Extra/org.chameleon.Boot.plist",
		"/Extra/org.chameleon.Boot.plist",
		"bt(0,0)/Extra/org.chameleon.Boot.plist",
		
		"rd(0,0)/Extra/com.apple.Boot.plist",   /* DEPRECIATED */
		"/Extra/com.apple.Boot.plist",           /* DEPRECIATED */
		"bt(0,0)/Extra/com.apple.Boot.plist",   /* DEPRECIATED */
	};

	int i, fd, count, ret=-1;

	for(i = 0; i< sizeof(dirspec)/sizeof(dirspec[0]); i++) {
		if ((fd = open(dirspec[i], 0)) >= 0) {
			// Check for depreciated file names and annoy the user about it.
			if(strstr(dirspec[i], "com.apple.Boot.plist")) {
				printf("%s is depreciated.\n", dirspec[i]);
				dirspec[i][strlen(dirspec[i]) - strlen("com.apple.Boot.plist")] = 0;
				printf("Please use the file %sorg.chameleon.Boot.plist instead.\n", dirspec[i]);
				pause();
			}
			// read file
			count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
			close(fd);
			
			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;	
			ret=0;
			break;
		}
	}
	return ret;
}

/* loadHelperConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadHelperConfig(config_file_t *config)
{
	char *dirspec[] = {
		"/com.apple.boot.P/Library/Preferences/SystemConfiguration/com.apple.Boot.plist",
		"/com.apple.boot.R/Library/Preferences/SystemConfiguration/com.apple.Boot.plist",
		"/com.apple.boot.S/Library/Preferences/SystemConfiguration/com.apple.Boot.plist"
	};

	int i, fd, count, ret=-1;

	for(i = 0; i< sizeof(dirspec)/sizeof(dirspec[0]); i++) {
		if ((fd = open(dirspec[i], 0)) >= 0) {
			// read file
			count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
			close(fd);
			
			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;	
			ret=0;
			break;
		}
	}
	return ret;
}

char * newString(const char * oldString)
{
	if ( oldString ) {
		return strcpy(malloc(strlen(oldString)+1), oldString);
	} else {
		return NULL;
	}
}

/*
 * Extracts the next argument from the command line, double quotes are allowed here.
 */
char * getNextArg(char ** argPtr, char * val)
{
	char * ptr = *argPtr;
	const char * strStart;
	int len = 0;
	bool isQuoted = false;

	*val = '\0';

	// Scan for the next non-whitespace character.
	while ( *ptr && (*ptr == ' ' || *ptr == '=') ) {
		ptr++;
	}
  
	strStart = ptr;

	// Skip the leading double quote character.
	if (*ptr == '\"') {
		isQuoted = true;
		ptr++;
		strStart++;
	}

	// Scan for the argument terminator character.
	// This can be either a NULL character - in case we reach the end of the string,
	// a double quote in case of quoted argument,
	// or a whitespace character (' ' or '=') for non-quoted argument.
	while (*ptr && !( (isQuoted && (*ptr == '\"')) ||
                    (!isQuoted && (*ptr == ' ' || *ptr == '=')) )
        ) {
		ptr++;
	}

	len = ptr - strStart;

	// Skip the closing double quote character and adjust
	// the starting pointer for the next getNextArg call.
	if (*ptr && isQuoted && *ptr == '\"') {
		ptr++;
	}

	// Copy the extracted argument to val.
	strncat(val, strStart, len);

	// Set command line pointer.
	*argPtr = ptr;

	return ptr;
}
