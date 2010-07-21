/*
 *  aml_generator.h
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 */

#ifndef __LIBSAIO_AML_GENERATOR_H
#define __LIBSAIO_AML_GENERATOR_H

#include "libsaio.h"

enum aml_chunk_type 
{
	AML_CHUNK_NONE		= -1,
	AML_CHUNK_ZERO		= 0x00,
	AML_CHUNK_ONE		= 0x01,
	AML_CHUNK_ALIAS		= 0x06,
	AML_CHUNK_NAME		= 0x08,
	AML_CHUNK_BYTE		= 0x0A,
	AML_CHUNK_WORD		= 0x0B,
	AML_CHUNK_DWORD		= 0x0C,
	AML_CHUNK_STRING	= 0x0D,
	AML_CHUNK_QWORD		= 0x0E,
	AML_CHUNK_SCOPE		= 0x10,
	AML_CHUNK_PACKAGE	= 0x12,
};

struct aml_chunk 
{
	enum aml_chunk_type	Type;
	unsigned long		Length;
	char*				Buffer;
	struct aml_chunk*	Next;
	struct aml_chunk*	First;
	struct aml_chunk*	Last;
};

static inline bool aml_isvalidchar(char c)
{
	return isupper(c) || isdigit(c) || c == '_';
};

#endif /* !__LIBSAIO_AML_GENERATOR_H */