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

//
// Primary OpCode
//
#define	AML_CHUNK_ZERO          0x00 // AML_ZERO_OP
#define	AML_CHUNK_ONE           0x01 // AML_ONE_OP
#define	AML_CHUNK_ALIAS         0x06 // AML_ALIAS_OP
#define	AML_CHUNK_NAME          0x08 // AML_NAME_OP
#define	AML_CHUNK_BYTE          0x0A // AML_BYTE_PREFIX
#define	AML_CHUNK_WORD          0x0B // AML_WORD_PREFIX
#define	AML_CHUNK_DWORD         0x0C // AML_DWORD_PREFIX
#define	AML_CHUNK_STRING        0x0D // AML_STRING_PREFIX
#define	AML_CHUNK_QWORD         0x0E // AML_QWORD_PREFIX
#define	AML_CHUNK_SCOPE         0x10 // AML_SCOPE_OP
#define AML_CHUNK_BUFFER        0x11 // AML_BUFFER_OP
#define	AML_CHUNK_PACKAGE       0x12 // AML_PACKAGE_OP
#define AML_CHUNK_VAR_PACKAGE   0x13 // AML_VAR_PACKAGE_OP
#define	AML_CHUNK_METHOD        0x14 // AML_METHOD_OP
#define AML_CHUNK_STRING_BUFFER	0x15 //
#define AML_CHUNK_LOCAL0        0x60 // AML_LOCAL0
#define AML_CHUNK_LOCAL1        0x61 // AML_LOCAL1
#define AML_CHUNK_LOCAL2        0x62 // AML_LOCAL2
#define AML_CHUNK_LOCAL3        0x63 // AML_LOCAL3
#define AML_CHUNK_LOCAL4        0x64 // AML_LOCAL4
#define AML_CHUNK_LOCAL5        0x65 // AML_LOCAL5
#define AML_CHUNK_LOCAL6        0x66 // AML_LOCAL6
#define AML_CHUNK_LOCAL7        0x67 // AML_LOCAL7
#define AML_CHUNK_ARG0          0x68 // AML_ARG0
#define AML_CHUNK_ARG1          0x69 // AML_ARG1
#define AML_CHUNK_ARG2          0x6A // AML_ARG2
#define AML_CHUNK_ARG3          0x6B // AML_ARG3
#define AML_CHUNK_ARG4          0x6C // AML_ARG4
#define AML_CHUNK_ARG5          0x6D // AML_ARG5
#define AML_CHUNK_ARG6          0x6E // AML_ARG6
#define AML_STORE_OP            0x70 // AML_STORE_OP
#define AML_CHUNK_REFOF         0x71 // AML_REF_OF_OP
#define AML_CHUNK_RETURN        0xA4 // AML_RETURN_OP
#define AML_CHUNK_BRECK         0xA5 // AML_BREAK_OP
#define	AML_CHUNK_NONE          0xff // AML_ONES_OP
//
// Extended OpCode
//
#define AML_CHUNK_OP            0x5B // AML_EXT_OP
#define AML_CHUNK_DEVICE        0x82 // AML_EXT_DEVICE_OP
#define AML_CHUNK_PROCESSOR     0x83 // AML_EXT_PROCESSOR_OP

struct aml_chunk {
	uint8_t			Type;
	uint16_t		Length;
	char*			Buffer;
	uint16_t		Size;
	struct aml_chunk*	Next;
	struct aml_chunk*	First;
	struct aml_chunk*	Last;
};

typedef struct aml_chunk AML_CHUNK;

static inline bool aml_isvalidchar(char c)
{
	return isupper(c) || isdigit(c) || c == '_';
};

bool aml_add_to_parent(AML_CHUNK* parent, AML_CHUNK* node);
AML_CHUNK* aml_create_node(AML_CHUNK* parent);
void aml_destroy_node(AML_CHUNK* node);
AML_CHUNK* aml_add_buffer(AML_CHUNK* parent, char* buffer, uint32_t size);
AML_CHUNK* aml_add_byte(AML_CHUNK* parent, uint8_t value);
AML_CHUNK* aml_add_word(AML_CHUNK* parent, uint16_t value);
AML_CHUNK* aml_add_dword(AML_CHUNK* parent, uint32_t value);
AML_CHUNK* aml_add_qword(AML_CHUNK* parent, uint64_t value);
AML_CHUNK* aml_add_scope(AML_CHUNK* parent, char* name);
AML_CHUNK* aml_add_name(AML_CHUNK* parent, char* name);
AML_CHUNK* aml_add_method(AML_CHUNK* parent, char* name, uint8_t args);
AML_CHUNK* aml_add_return_name(AML_CHUNK* parent, char* name);
AML_CHUNK* aml_add_return_byte(AML_CHUNK* parent, uint8_t value);
AML_CHUNK* aml_add_package(AML_CHUNK* parent);
AML_CHUNK* aml_add_alias(AML_CHUNK* parent, char* name1, char* name2);
uint32_t aml_calculate_size(AML_CHUNK* node);
uint32_t aml_write_node(AML_CHUNK* node, char* buffer, uint32_t offset);
uint32_t aml_write_size(uint32_t size, char* buffer, uint32_t offset);

AML_CHUNK* aml_add_string(AML_CHUNK* parent, char* string);
AML_CHUNK* aml_add_byte_buffer(AML_CHUNK* parent, char* data, uint32_t size);
AML_CHUNK* aml_add_string_buffer(AML_CHUNK* parent, char* string);
AML_CHUNK* aml_add_device(AML_CHUNK* parent, char* name);
AML_CHUNK* aml_add_local0(AML_CHUNK* parent);
AML_CHUNK* aml_add_store(AML_CHUNK* parent);
AML_CHUNK* aml_add_return(AML_CHUNK* parent);

int32_t FindBin (uint8_t *dsdt, uint32_t len, uint8_t *bin, unsigned int N);
uint32_t get_size(uint8_t* Buffer, uint32_t adr);

#endif /* !__LIBSAIO_AML_GENERATOR_H */
