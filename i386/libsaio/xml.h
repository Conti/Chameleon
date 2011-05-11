/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 2003 Apple Computer, Inc.  All Rights
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

#ifndef __LIBSAIO_XML_H
#define __LIBSAIO_XML_H

enum xmltype {
  kTagTypeNone = 0,
  kTagTypeDict,
  kTagTypeKey,
  kTagTypeString,
  kTagTypeInteger,
  kTagTypeData,
  kTagTypeDate,
  kTagTypeFalse,
  kTagTypeTrue,
  kTagTypeArray
};


struct string_ref
{
	char* string;
	int id;
	struct string_ref* next;
};
typedef struct string_ref string_ref;

extern string_ref* ref_strings;

#define kXMLTagPList   "plist "
#define kXMLTagDict    "dict"
#define kXMLTagKey     "key"
#define kXMLTagString  "string"
#define kXMLTagInteger "integer"
#define kXMLTagData    "data"
#define kXMLTagDate    "date"
#define kXMLTagFalse   "false/"
#define kXMLTagTrue    "true/"
#define kXMLTagArray   "array"

#define kXMLStringID	"ID="
#define kXMLStringIDRef "IDREF="


#define kPropCFBundleIdentifier ("CFBundleIdentifier")
#define kPropCFBundleExecutable ("CFBundleExecutable")
#define kPropOSBundleRequired   ("OSBundleRequired")
#define kPropOSBundleLibraries  ("OSBundleLibraries")
#define kPropIOKitPersonalities ("IOKitPersonalities")
#define kPropIONameMatch        ("IONameMatch")

/*
struct Tag {
  long       type;
  char       *string;
  struct Tag *tag;
  struct Tag *tagNext;
};
typedef struct Tag Tag, *TagPtr;
 */

extern long  gImageFirstBootXAddr;
extern long  gImageLastKernelAddr;

TagPtr XMLGetProperty( TagPtr dict, const char * key );
TagPtr XMLGetElement( TagPtr dict, int id );
int XMLTagCount( TagPtr dict );

bool XMLIsType(TagPtr dict, enum xmltype type);

bool XMLCastBoolean( TagPtr dict );
char* XMLCastString( TagPtr dict );
long XMLCastStringOffset(TagPtr dict);
int XMLCastInteger ( TagPtr dict );
TagPtr XMLCastDict ( TagPtr dict );
TagPtr XMLCastArray( TagPtr dict );

long XMLParseNextTag(char *buffer, TagPtr *tag);
void XMLFreeTag(TagPtr tag);
char* XMLDecode(const char *in);
//==========================================================================
// XMLParseFile
// Expects to see one dictionary in the XML file.
// Puts the first dictionary it finds in the
// tag pointer and returns 0, or returns -1 if not found.
//
long XMLParseFile( char * buffer, TagPtr * dict );

#endif /* __LIBSAIO_XML_H */
