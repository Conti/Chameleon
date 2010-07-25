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

#define	AML_CHUNK_NONE		0xff
#define	AML_CHUNK_ZERO		0x00
#define	AML_CHUNK_ONE		0x01
#define	AML_CHUNK_ALIAS		0x06
#define	AML_CHUNK_NAME		0x08
#define	AML_CHUNK_BYTE		0x0A
#define	AML_CHUNK_WORD		0x0B
#define	AML_CHUNK_DWORD		0x0C
#define	AML_CHUNK_STRING	0x0D
#define	AML_CHUNK_QWORD		0x0E
#define	AML_CHUNK_SCOPE		0x10
#define	AML_CHUNK_PACKAGE	0x12

struct aml_chunk 
{
	unsigned char		Type;
	unsigned int		Length;
	char*				Buffer;
	
	unsigned int		Size;
	
	struct aml_chunk*	Next;
	struct aml_chunk*	First;
	struct aml_chunk*	Last;
};

static inline bool aml_isvalidchar(char c)
{
	return isupper(c) || isdigit(c) || c == '_';
};

bool aml_add_to_parent(struct aml_chunk* parent, struct aml_chunk* node);
struct aml_chunk* aml_create_node(struct aml_chunk* parent);
void aml_destroy_node(struct aml_chunk* node);
struct aml_chunk* aml_add_buffer(struct aml_chunk* parent, const char* buffer, unsigned int size);
struct aml_chunk* aml_add_byte(struct aml_chunk* parent, unsigned char value);
struct aml_chunk* aml_add_word(struct aml_chunk* parent, unsigned int value);
struct aml_chunk* aml_add_dword(struct aml_chunk* parent, unsigned long value);
struct aml_chunk* aml_add_qword(struct aml_chunk* parent, unsigned long long value);
struct aml_chunk* aml_add_scope(struct aml_chunk* parent, const char* name);
struct aml_chunk* aml_add_name(struct aml_chunk* parent, const char* name);
struct aml_chunk* aml_add_package(struct aml_chunk* parent);
struct aml_chunk* aml_add_alias(struct aml_chunk* parent, const char* name1, const char* name2);
unsigned int aml_calculate_size(struct aml_chunk* node);
unsigned int aml_write_node(struct aml_chunk* node, char* buffer, unsigned int offset);

#endif /* !__LIBSAIO_AML_GENERATOR_H */