/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 2003 Apple Computer, Inc.  All Rights
 * Reserved.  
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "bootstruct.h"
#include "libsaio.h"
#include "sl.h"
#include "xml.h"

string_ref *ref_strings = NULL;

/// TODO: remove below
static char* buffer_start = NULL;
// TODO: redo the next two functions
void SaveRefString(char* string, int id)
{
	//printf("Adding Ref String %d (%s)\n", id, string);
	string_ref* tmp = ref_strings;
	while(tmp)
	{
		if(tmp->id == id)
		{
			tmp->string = malloc(strlen(string)+1);
			sprintf(tmp->string, "%s", string);
			return;
		}
		tmp = tmp->next;
	}
	
	string_ref* new_ref = malloc(sizeof(string_ref));
	new_ref->string = malloc(strlen(string)+1);
	sprintf(new_ref->string, "%s", string);
	new_ref->id = id;
	new_ref->next = ref_strings;
	ref_strings = new_ref;
}

char* GetRefString(int id)
{
	string_ref* tmp = ref_strings;
	while(tmp)
	{
		if(tmp->id == id) return tmp->string;
		tmp = tmp->next;
	}
	//verbose("Unable to locate Ref String %d\n", id);
	return "";
}

struct Module {
  struct Module *nextModule;
  long          willLoad;
  TagPtr        dict;
  char          *plistAddr;
  long          plistLength;
  char          *driverPath;
};
typedef struct Module Module, *ModulePtr;

struct DriverInfo {
  char *plistAddr;
  long plistLength;
  void *moduleAddr;
  long moduleLength;
};
typedef struct DriverInfo DriverInfo, *DriverInfoPtr;

#define kDriverPackageSignature1 'MKXT'
#define kDriverPackageSignature2 'MOSX'

struct DriversPackage {
  unsigned long signature1;
  unsigned long signature2;
  unsigned long length;
  unsigned long adler32;
  unsigned long version;
  unsigned long numDrivers;
  unsigned long reserved1;
  unsigned long reserved2;
};
typedef struct DriversPackage DriversPackage;

enum {
  kCFBundleType2,
  kCFBundleType3
};


#define DOFREE 1

static long ParseTagList(char *buffer, TagPtr *tag, long type, long empty);
static long ParseTagKey(char *buffer, TagPtr *tag);
static long ParseTagString(char *buffer, TagPtr *tag);
static long ParseTagInteger(char *buffer, TagPtr *tag);
static long ParseTagData(char *buffer, TagPtr *tag);
static long ParseTagDate(char *buffer, TagPtr *tag);
static long GetNextTag(char *buffer, char **tag, long *start);
static long FixDataMatchingTag(char *buffer, char *tag);
static TagPtr NewTag(void);
static char *NewSymbol(char *string);
#if DOFREE
static void FreeSymbol(char *string);
#endif


//==========================================================================
// XMLGetProperty

TagPtr
XMLGetProperty( TagPtr dict, const char * key )
{
    TagPtr tagList, tag;

    if (dict->type != kTagTypeDict) return 0;
    
    tag = 0;
    tagList = dict->tag;
    while (tagList)
    {
        tag = tagList;
        tagList = tag->tagNext;
        
        if ((tag->type != kTagTypeKey) || (tag->string == 0)) continue;
        
        if (!strcmp(tag->string, key)) return tag->tag;
    }
    
    return 0;
}

//==========================================================================
// XMLGetProperty

TagPtr
XMLGetKey( TagPtr dict, int id )
{
    TagPtr tagList, tag;
    
    if (dict->type != kTagTypeDict) return 0;
    
    tag = 0;
    int element = 0;
    tagList = dict->tag;
    while (tagList && element != id)
    {
        tag = tagList;
        tagList = tag->tagNext;
        
        if ((tag->type != kTagTypeKey) || (tag->string == 0)) continue;
        element++;
        if(id == element) return tag;
    }
    return 0;
}

TagPtr XMLGetValueForKey(TagPtr key)
{
    if (!key ||
        key->type != kTagTypeKey) return 0;
    
    return key->tag;
}


// XMLGetTag(int index)

// XMLTagCount( TagPtr dict )
int XMLTagCount( TagPtr dict )
{
	int count = 0;
	TagPtr tagList, tag;

    if (dict->type != kTagTypeDict && dict->type != kTagTypeArray) return 0;
	tag = 0;
    tagList = dict->tag;
    while (tagList)
    {
		tag = tagList;
        tagList = tag->tagNext;
		
		if (((tag->type != kTagTypeKey) && ((tag->string == 0) || (tag->string[0] == 0)))
			&& (dict->type != kTagTypeArray)	// If we are an array, any element is valid
			) continue;
		
		//if(tag->type == kTagTypeKey) printf("Located key %s\n", tag->string);

		count++;
    }
	
	return count;
}

TagPtr XMLGetElement( TagPtr dict, int id )
{
	if(dict->type != kTagTypeArray) return 0;
	
	int element = 0;
	TagPtr tmp = dict->tag;

	while(element < id)
	{
		element++;
		tmp = tmp->tagNext;
	}
	
	return tmp;
}
/* Function for basic XML character entities parsing */

char*
XMLDecode(const char* src)
{
    typedef const struct XMLEntity {
        const char* name;
        size_t nameLen;
        char value;
    } XMLEntity;
    
    /* This is ugly, but better than specifying the lengths by hand */
    #define _e(str,c) {str,sizeof(str)-1,c}
    const XMLEntity ents[] = {
        _e("quot;",'"'), _e("apos;",'\''),
        _e("lt;",  '<'), _e("gt;",  '>'),
        _e("amp;", '&')
    };
    
    size_t len;
    const char *s;
    char *out, *o;
    
    if ( !src || !(len = strlen(src)) || !(out = malloc(len+1)) )
        return 0;
    
    o = out;
    s = src;
    while (s <= src+len) /* Make sure the terminator is also copied */
    {
        if ( *s == '&' )
        {
            bool entFound = false;
            int i;
            
            s++;
            for ( i = 0; i < sizeof(ents); i++)
            {
                if ( strncmp(s, ents[i].name, ents[i].nameLen) == 0 )
                {
                    entFound = true;
                    break;
                }
            }
            if ( entFound )
            {
                *o++ = ents[i].value;
                s += ents[i].nameLen;
                continue;
            }
        }
        
        *o++ = *s++;
    }

    return out;
}                    

//#if UNUSED
//==========================================================================
// XMLParseFile
// Expects to see one dictionary in the XML file, the final pos will be returned
// If the pos is not equal to the strlen, then there are multiple dicts
// Puts the first dictionary it finds in the
// tag pointer and returns the end of the dic, or returns -1 if not found.
//
long
XMLParseFile( char * buffer, TagPtr * dict )
{
    long       length, pos;
    TagPtr     tag;
    pos = 0;
	char       *configBuffer;
	
	
	
    configBuffer = malloc(strlen(buffer)+1);
    strcpy(configBuffer, buffer);
	
	buffer_start = configBuffer;

    while (1)
    {
        length = XMLParseNextTag(configBuffer + pos, &tag);
        if (length == -1) break;
    
        pos += length;
    
        if (tag == 0) continue;
        if (tag->type == kTagTypeDict) break;
    
        XMLFreeTag(tag);
    }
	free(configBuffer);
	if (length < 0) {
        return -1;
    }
    *dict = tag;
    return pos;
}
//#endif /* UNUSED */

//==========================================================================
// ParseNextTag
// TODO: cleanup
long
XMLParseNextTag( char * buffer, TagPtr * tag )
{
	long   length, pos;
	char * tagName;
	
	length = GetNextTag(buffer, &tagName, 0);
	if (length == -1) return -1;
	
	pos = length;
	if (!strncmp(tagName, kXMLTagPList, 6))
	{
		length = 0;
	}
	/***** dict ****/
	else if (!strcmp(tagName, kXMLTagDict))
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeDict, 0);
	}
	else if (!strncmp(tagName, kXMLTagDict, strlen(kXMLTagDict)) && tagName[strlen(tagName)-1] == '/')
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeDict, 1);
	}
	else if (!strncmp(tagName, kXMLTagDict " ", strlen(kXMLTagDict " ")))
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeDict, 0);
	}
	/***** key ****/
	else if (!strcmp(tagName, kXMLTagKey))
	{
		length = ParseTagKey(buffer + pos, tag);
	}
	
	/***** string ****/
	else if (!strcmp(tagName, kXMLTagString))
	{
		length = ParseTagString(buffer + pos, tag);
	}
	else if (!strncmp(tagName, kXMLTagString " ", strlen(kXMLTagString " ")))
	{
		// TODO: save tag if if found
		if(!strncmp(tagName + strlen(kXMLTagString " "), kXMLStringID, strlen(kXMLStringID)))
		{
			// ID=
			int id = 0;
			int cnt = strlen(kXMLTagString " " kXMLStringID "\"") + 1;
			while ((tagName[cnt] != '\0') && (tagName[cnt] != '"')) cnt++;
			tagName[cnt] = 0;
			char* val = tagName + strlen(kXMLTagString " " kXMLStringID "\"");
			while(*val)
			{
				if ((*val >= '0' && *val <= '9'))	// 0 - 9
				{
					id = (id * 10) + (*val++ - '0');
				}
				else
				{
					printf("ParseStringID error (0x%x)\n", *val);
					getchar();
					return -1;
				}
			}
			length = ParseTagString(buffer + pos, tag);
			
			SaveRefString(buffer + pos, id);
		}
		else if(!strncmp(tagName + strlen(kXMLTagString " "), kXMLStringIDRef, strlen(kXMLStringIDRef)))
		{
			// IDREF=
			int id = 0;
			int cnt = strlen(kXMLTagString " " kXMLStringIDRef "\"") + 1;
			while ((tagName[cnt] != '\0') && (tagName[cnt] != '"')) cnt++;
			tagName[cnt] = 0;
			char* val = tagName + strlen(kXMLTagString " " kXMLStringIDRef "\"");
			while(*val)
			{
				if ((*val >= '0' && *val <= '9'))	// 0 - 9
				{
					id = (id * 10) + (*val++ - '0');
				}
				else
				{
					printf("ParseStringIDREF error (0x%x)\n", *val);
					getchar();
					return -1;
				}
			}
			char* str = GetRefString(id);

			TagPtr tmpTag = NewTag();
			tmpTag->type = kTagTypeString;
			tmpTag->string = str;
			tmpTag->tag = 0;
			tmpTag->tagNext = 0;
			tmpTag->offset = buffer_start ? buffer - buffer_start  + pos : 0;
			*tag = tmpTag;
			
			length = 0;
			//printf("Located IDREF, id = %d, string = %s\n", id, str);
		}
	}
	
	/***** integer ****/
	else if (!strcmp(tagName, kXMLTagInteger))
	{
		length = ParseTagInteger(buffer + pos, tag);
	}
	else if (!strncmp(tagName, kXMLTagInteger " ", strlen(kXMLTagInteger " ")))
	{
		if(!strncmp(tagName + strlen(kXMLTagInteger " "), kXMLStringID, strlen(kXMLStringID)))
		{
			// ID=
			int id = 0;
			int cnt = strlen(kXMLTagInteger " " kXMLStringID "\"") + 1;
			while ((tagName[cnt] != '\0') && (tagName[cnt] != '"')) cnt++;
			tagName[cnt] = 0;
			char* val = tagName + strlen(kXMLTagInteger " " kXMLStringID "\"");
			while(*val)
			{
				if ((*val >= '0' && *val <= '9'))	// 0 - 9
				{
					id = (id * 10) + (*val++ - '0');
				}
				else
				{
					printf("ParseIntegerID error (0x%x)\n", *val);
					getchar();
					return -1;
				}
			}
			length = ParseTagInteger(buffer + pos, tag);
			
			SaveRefString((*tag)->string, id);
		}
		else if(!strncmp(tagName + strlen(kXMLTagInteger " "), kXMLStringIDRef, strlen(kXMLStringIDRef)))
		{
			// IDREF=
			int id = 0;
			int cnt = strlen(kXMLTagInteger " " kXMLStringIDRef "\"") + 1;
			while ((tagName[cnt] != '\0') && (tagName[cnt] != '"')) cnt++;
			tagName[cnt] = 0;
			char* val = tagName + strlen(kXMLTagInteger " " kXMLStringIDRef "\"");
			while(*val)
			{
				if ((*val >= '0' && *val <= '9'))	// 0 - 9
				{
					id = (id * 10) + (*val++ - '0');
				}
				else
				{
					printf("ParseStringIDREF error (0x%x)\n", *val);
					getchar();
					return -1;
				}
			}
			int integer = (int)GetRefString(id);
			
			TagPtr tmpTag = NewTag();
			tmpTag->type = kTagTypeInteger;
			tmpTag->string = (char*) integer;
			tmpTag->tag = 0;
			tmpTag->tagNext = 0;
			tmpTag->offset = buffer_start ? buffer - buffer_start + pos : 0;
			
			*tag = tmpTag;
			
			length = 0;
			//printf("Located IDREF, id = %d, string = %s\n", id, str);
		}
		else
		{
			length = ParseTagInteger(buffer + pos, tag);
		}
	}
	
	/***** data ****/
	else if (!strcmp(tagName, kXMLTagData))
	{
		length = ParseTagData(buffer + pos, tag);
	}
	else if (!strncmp(tagName, kXMLTagData " ", strlen(kXMLTagData " ")))
	{
		length = ParseTagData(buffer + pos, tag);
	}
	else if (!strcmp(tagName, kXMLTagDate))
	{
		length = ParseTagDate(buffer + pos, tag);
	}
	
	/***** date ****/
	else if (!strncmp(tagName, kXMLTagDate " ", strlen(kXMLTagDate " ")))
	{
		length = ParseTagDate(buffer + pos, tag);
	}
	
	/***** false ****/
	else if (!strcmp(tagName, kXMLTagFalse))
	{
		length = ParseTagBoolean(buffer + pos, tag, kTagTypeFalse);
	}
	/***** true ****/
	else if (!strcmp(tagName, kXMLTagTrue))
	{
		length = ParseTagBoolean(buffer + pos, tag, kTagTypeTrue);
	}
	
	/***** array ****/
	else if (!strcmp(tagName, kXMLTagArray))
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeArray, 0);
	}
	else if (!strncmp(tagName, kXMLTagArray " ", strlen(kXMLTagArray " ")))
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeArray, 0);
	}
	else if (!strcmp(tagName, kXMLTagArray "/"))
	{
		length = ParseTagList(buffer + pos, tag, kTagTypeArray, 1);
	}
	
	/***** unknown ****/
	else
	{
		*tag = 0;
		length = 0;
	}
	
	if (length == -1) return -1;
	
	return pos + length;
}

//==========================================================================
// ParseTagList

static long
ParseTagList( char * buffer, TagPtr * tag, long type, long empty )
{
	long   length, pos;
	TagPtr tagList, tmpTag;
  
    tagList = 0;
    pos = 0;
  
    if (!empty)
    {
        while (1)
        {
            length = XMLParseNextTag(buffer + pos, &tmpTag);
            if (length == -1) break;

            pos += length;
      
            if (tmpTag == 0) break;
            tmpTag->tagNext = tagList;
            tagList = tmpTag;
        }
    
        if (length == -1)
        {
            XMLFreeTag(tagList);
            return -1;
        }
    }
  
    tmpTag = NewTag();
    if (tmpTag == 0)
    {
        XMLFreeTag(tagList);
        return -1;
    }

    tmpTag->type = type;
    tmpTag->string = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start : 0;
    tmpTag->tag = tagList;
    tmpTag->tagNext = 0;
    
    *tag = tmpTag;
    
    return pos;
}

//==========================================================================
// ParseTagKey

static long
ParseTagKey( char * buffer, TagPtr * tag )
{
    long   length, length2;
    char   *string;
    TagPtr tmpTag, subTag;
  
    length = FixDataMatchingTag(buffer, kXMLTagKey);
    if (length == -1) return -1;
  
    length2 = XMLParseNextTag(buffer + length, &subTag);
    if (length2 == -1) return -1;
  
    tmpTag = NewTag();
    if (tmpTag == 0)
    {
        XMLFreeTag(subTag);
        return -1;
    }
  
    string = NewSymbol(buffer);
    if (string == 0)
    {
        XMLFreeTag(subTag);
        XMLFreeTag(tmpTag);
        return -1;
    }
  
    tmpTag->type = kTagTypeKey;
    tmpTag->string = string;
    tmpTag->tag = subTag;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
  
    *tag = tmpTag;
  
    return length + length2;
}

//==========================================================================
// ParseTagString

static long
ParseTagString( char * buffer, TagPtr * tag )
{
    long   length;
    char * string;
  
    length = FixDataMatchingTag(buffer, kXMLTagString);
    if (length == -1) return -1;
  
	TagPtr tmpTag = NewTag();
    if (tmpTag == 0) return -1;
  
    string = NewSymbol(buffer);
    if (string == 0)
    {
        XMLFreeTag(tmpTag);
        return -1;
    }
  
    tmpTag->type = kTagTypeString;
    tmpTag->string = string;
    tmpTag->tag = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
  
    *tag = tmpTag;
    return length;
}

//==========================================================================
// ParseTagInteger

static long
ParseTagInteger( char * buffer, TagPtr * tag )
{
    long   length, integer;
	bool negative = false;
    TagPtr tmpTag;
	char* val = buffer;
    int size;
	
	if(buffer[0] == '<')
	{
		printf("Warning integer is non existant\n");
		getchar();
		tmpTag = NewTag();
		tmpTag->type = kTagTypeInteger;
		tmpTag->string = 0;
		tmpTag->tag = 0;
		tmpTag->offset =  0;
		tmpTag->tagNext = 0;
		
		*tag = tmpTag;
		
		return 0;
	}
	
    size = length = FixDataMatchingTag(buffer, kXMLTagInteger);
    if (length == -1) return -1;
    
    tmpTag = NewTag();
    if (tmpTag == 0) return -1;
    
    integer = 0;

	if(size > 1 && (val[1] == 'x' || val[1] == 'X'))	// Hex value
	{
		val += 2;
		while(*val)
		{
			if ((*val >= '0' && *val <= '9'))	// 0 - 9
			{
				integer = (integer * 16) + (*val++ - '0');
			}
			else if ((*val >= 'a' && *val <= 'f'))	// a - f
			{
				integer = (integer * 16) + (*val++ - 'a' + 10);
			}
			else if ((*val >= 'A' && *val <= 'F'))	// A - F
			{
				integer = (integer * 16) + (*val++ - 'a' + 10);
			}
			else
			{
				printf("ParseTagInteger hex error (0x%x) in buffer %s\n", *val, buffer);
				getchar();
				return -1;
			}
		}
	}
	else if ( size )	// Decimal value
	{
		if (*val == '-')
		{
			negative = true;
			val++;
			size--;
		}
		
		for (integer = 0; size > 0; size--)
		{
			if(*val) // UGLY HACK, fix me.
			{
				if (*val < '0' || *val > '9')
				{
					printf("ParseTagInteger decimal error (0x%x) in buffer %s\n", *val, buffer);
					getchar();
					return -1;
				}
				
				integer = (integer * 10) + (*val++ - '0');
			}
		}
		
		if (negative)
			integer = -integer;
	}
		
    tmpTag->type = kTagTypeInteger;
	tmpTag->string = (char *)integer;
	tmpTag->tag = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
    
    *tag = tmpTag;
    
    return length;
}

//==========================================================================
// ParseTagData

static long
ParseTagData( char * buffer, TagPtr * tag )
{
    long   length;
    TagPtr tmpTag;

    length = FixDataMatchingTag(buffer, kXMLTagData);
    if (length == -1) return -1;
    
    tmpTag = NewTag();
    if (tmpTag == 0) return -1;
    
	//printf("ParseTagData unimplimented\n");
	//printf("Data: %s\n", buffer);
	//	getchar();
	
	// TODO: base64 decode
	
	char* string = NewSymbol(buffer);
    tmpTag->type = kTagTypeData;
    tmpTag->string = string;
    tmpTag->tag = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
    
    *tag = tmpTag;
    
    return length;
}

//==========================================================================
// ParseTagDate

static long
ParseTagDate( char * buffer, TagPtr * tag )
{
    long   length;
    TagPtr tmpTag;
    
    length = FixDataMatchingTag(buffer, kXMLTagDate);
    if (length == -1) return -1;
    
    tmpTag = NewTag();
    if (tmpTag == 0) return -1;
    
	printf("ParseTagDate unimplimented\n");
	getchar();
	
    tmpTag->type = kTagTypeDate;
    tmpTag->string = 0;
    tmpTag->tag = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
    
    *tag = tmpTag;
    
    return length;
}

//==========================================================================
// ParseTagBoolean

long
ParseTagBoolean( char * buffer, TagPtr * tag, long type )
{
    TagPtr tmpTag;
    
    tmpTag = NewTag();
    if (tmpTag == 0) return -1;
    
    tmpTag->type = type;
    tmpTag->string = 0;
    tmpTag->tag = 0;
	tmpTag->offset = buffer_start ? buffer - buffer_start: 0;
    tmpTag->tagNext = 0;
    
    *tag = tmpTag;
    
    return 0;
}

//==========================================================================
// GetNextTag

static long
GetNextTag( char * buffer, char ** tag, long * start )
{
    long cnt, cnt2;

    if (tag == 0) return -1;
    
    // Find the start of the tag.
    cnt = 0;
    while ((buffer[cnt] != '\0') && (buffer[cnt] != '<')) cnt++;
    if (buffer[cnt] == '\0') return -1;
    
    // Find the end of the tag.
    cnt2 = cnt + 1;
    while ((buffer[cnt2] != '\0') && (buffer[cnt2] != '>')) cnt2++;
    if (buffer[cnt2] == '\0') return -1;

    // Fix the tag data.
    *tag = buffer + cnt + 1;
    buffer[cnt2] = '\0';
    if (start) *start = cnt;
    
    return cnt2 + 1;
}

//==========================================================================
// FixDataMatchingTag
// Modifies 'buffer' to add a '\0' at the end of the tag matching 'tag'.
// Returns the length of the data found, counting the end tag,
// or -1 if the end tag was not found.

static long
FixDataMatchingTag( char * buffer, char * tag )
{
    long   length, start, stop;
    char * endTag;
    
    start = 0;
    while (1)
    {
        length = GetNextTag(buffer + start, &endTag, &stop);
        if (length == -1) return -1;
        
        if ((*endTag == '/') && !strcmp(endTag + 1, tag)) break;
        start += length;
    }
    
    buffer[start + stop] = '\0';
    
    return start + length;
}

//==========================================================================
// NewTag

#define kTagsPerBlock (0x1000)

static TagPtr gTagsFree;

static TagPtr
NewTag( void )
{
	long   cnt;
	TagPtr tag;
  
    if (gTagsFree == 0)
    {
        tag = (TagPtr)malloc(kTagsPerBlock * sizeof(Tag));
        if (tag == 0) return 0;
        
        // Initalize the new tags.
        for (cnt = 0; cnt < kTagsPerBlock; cnt++)
        {
            tag[cnt].type = kTagTypeNone;
            tag[cnt].string = 0;
            tag[cnt].tag = 0;
            tag[cnt].tagNext = tag + cnt + 1;
        }
        tag[kTagsPerBlock - 1].tagNext = 0;

        gTagsFree = tag;
    }

    tag = gTagsFree;
    gTagsFree = tag->tagNext;
    
    return tag;
}

//==========================================================================
// XMLFreeTag

void
XMLFreeTag( TagPtr tag )
{
#if DOFREE
    if (tag == 0) return;
  
    if (tag->string) FreeSymbol(tag->string);
  
    XMLFreeTag(tag->tag);
    XMLFreeTag(tag->tagNext);
  
    // Clear and free the tag.
    tag->type = kTagTypeNone;
    tag->string = 0;
    tag->tag = 0;
	tag->offset = 0;
    tag->tagNext = gTagsFree;
    gTagsFree = tag;
#else
    return;
#endif
}

//==========================================================================
// Symbol object.

struct Symbol
{
  long          refCount;
  struct Symbol *next;
  char          string[];
};
typedef struct Symbol Symbol, *SymbolPtr;

static SymbolPtr FindSymbol(char * string, SymbolPtr * prevSymbol);

static SymbolPtr gSymbolsHead;

//==========================================================================
// NewSymbol

static char *
NewSymbol( char * string )
{
static SymbolPtr lastGuy = 0;
	SymbolPtr symbol;
  
    // Look for string in the list of symbols.
    symbol = FindSymbol(string, 0);
  
    // Add the new symbol.
    if (symbol == 0)
    {
        symbol = (SymbolPtr)malloc(sizeof(Symbol) + 1 + strlen(string));
        if (symbol == 0) //return 0;
            stop("NULL symbol!");
    
        // Set the symbol's data.
        symbol->refCount = 0;
        strcpy(symbol->string, string);
    
        // Add the symbol to the list.
        symbol->next = gSymbolsHead;
        gSymbolsHead = symbol;
    }
  
    // Update the refCount and return the string.
    symbol->refCount++;

 if (lastGuy && lastGuy->next != 0) stop("last guy not last!");
    return symbol->string;
}

//==========================================================================
// FreeSymbol

#if DOFREE
static void
FreeSymbol( char * string )
{
    SymbolPtr symbol, prev;
	prev = 0;
  
    // Look for string in the list of symbols.
    symbol = FindSymbol(string, &prev);
    if (symbol == 0) return;
    
    // Update the refCount.
    symbol->refCount--;
    
    if (symbol->refCount != 0) return;
    
    // Remove the symbol from the list.
    if (prev != 0) prev->next = symbol->next;
    else gSymbolsHead = symbol->next;
    
    // Free the symbol's memory.
    free(symbol);
}
#endif

//==========================================================================
// FindSymbol

static SymbolPtr
FindSymbol( char * string, SymbolPtr * prevSymbol )
{
    SymbolPtr symbol, prev;
  
    symbol = gSymbolsHead;
    prev = 0;
  
    while (symbol != 0) {
        if (!strcmp(symbol->string, string)) break;
    
        prev = symbol;
        symbol = symbol->next;
    }
  
    if ((symbol != 0) && (prevSymbol != 0)) *prevSymbol = prev;
  
    return symbol;
}

bool XMLIsType(TagPtr dict, enum xmltype type)
{
	if(!dict) return (type == kTagTypeNone);
	return (dict->type == type);
}

/*** Cast functions ***/
TagPtr XMLCastArray(TagPtr dict)
{
	if(!dict) return NULL;
	if(dict->type == kTagTypeArray) return dict;
	else return NULL;
}

TagPtr XMLCastDict(TagPtr dict)
{
	if(!dict) return NULL;
	if(dict->type == kTagTypeDict) return dict;
	else return NULL;
}

char* XMLCastString(TagPtr dict)
{
	if(!dict) return NULL;

	if((dict->type == kTagTypeString) ||
	   (dict->type == kTagTypeKey)) return dict->string;
	
	return NULL;
}

long XMLCastStringOffset(TagPtr dict)
{
	if(dict &&
	   ((dict->type == kTagTypeString) ||
	   (dict->type == kTagTypeKey)))
	{
		return dict->offset;
	}
	else 
	{
		return -1;
	}
}

bool XMLCastBoolean(TagPtr dict)
{
	if(!dict) return false;
	if(dict->type == kTagTypeTrue) return true;
	return false;
}

int XMLCastInteger(TagPtr dict)
{
	if(!dict)
	{
		//printf("XMLCastInteger: null dict\n");
		return 0;
	}
	if(dict->type == kTagTypeInteger) return (int)(dict->string);
	return 0;
}

bool XMLAddTagToDictionary(TagPtr dict, char* key, TagPtr value)
{
    if (!dict || dict->type != kTagTypeDict) return false;

    TagPtr tmpTag;
    char* string;

    tmpTag = NewTag();
    if (tmpTag == 0)
    {
        return false;
    }
    
    string = NewSymbol(key);
    if (string == 0)
    {
        XMLFreeTag(tmpTag);
        return false;
    }
    
    tmpTag->type = kTagTypeKey;
    tmpTag->string = string;
    tmpTag->tag = value;
	tmpTag->offset = 0;
    tmpTag->tagNext = 0;
    
    TagPtr tagList = dict->tag;
    if(!tagList)
    {
        // First tag
        dict->tag = tmpTag;
        return true;
    }
    while(tagList && tagList->tagNext) tagList = tagList->tagNext;
    if(tagList)
    {
        tagList->tagNext = tmpTag;
        return true;
    }
    return false;
}