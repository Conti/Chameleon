/*
 *  aml_generator.c
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 * additions and corrections by Slice and pcj, 2012.
 */

#include "aml_generator.h"

bool aml_add_to_parent(AML_CHUNK* parent, AML_CHUNK* node)
{
	if (parent && node)
	{
		switch (parent->Type) 
		{
			case AML_CHUNK_NONE:
			case AML_CHUNK_BYTE:
			case AML_CHUNK_WORD:
			case AML_CHUNK_DWORD:
			case AML_CHUNK_QWORD:
			case AML_CHUNK_ALIAS:
				verbose("aml_add_to_parent: Node doesn't support child nodes!\n");
				return false;
			case AML_CHUNK_NAME:
				if (parent->First) {
					verbose("aml_add_to_parent: Name node supports only one child node!\n");
					return false;
				}
				break;

			default:
				break;
		}

		if (!parent->First) {
			parent->First = node;
		}
		if (parent->Last) {
			parent->Last->Next = node;
		}
		parent->Last = node;

		return true;
	}

	return false;
}

AML_CHUNK* aml_create_node(AML_CHUNK* parent)
{
	AML_CHUNK* node = (AML_CHUNK*)malloc(sizeof(AML_CHUNK));

	aml_add_to_parent(parent, node);

	return node;
}

void aml_destroy_node(AML_CHUNK* node)
{
	// Delete child nodes
	AML_CHUNK* child = node->First;

	while (child) 
	{
		AML_CHUNK* next = child->Next;

		if (child->Buffer) {
			free(child->Buffer);
		}
		free(child);
		
		child = next;
	}

	// Free node
	if (node->Buffer) {
		free(node->Buffer);
	}

	free(node);
}

AML_CHUNK* aml_add_buffer(AML_CHUNK* parent, char* buffer, uint32_t size)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_NONE;
		node->Length = (uint16_t)size;
		node->Buffer = malloc(node->Length);
		memcpy(node->Buffer, buffer, node->Length);
	}

	return node;
}

AML_CHUNK* aml_add_byte(AML_CHUNK* parent, uint8_t value)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_BYTE;
		node->Length = 1;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value;
	}
	return node;
}

AML_CHUNK* aml_add_word(AML_CHUNK* parent, uint16_t value)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_WORD;
		node->Length = 2;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value & 0xff;
		node->Buffer[1] = value >> 8;
	}
	return node;
}

AML_CHUNK* aml_add_dword(AML_CHUNK* parent, uint32_t value)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_DWORD;
		node->Length = 4;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value & 0xff;
		node->Buffer[1] = (value >> 8) & 0xff;
		node->Buffer[2] = (value >> 16) & 0xff;
		node->Buffer[3] = (value >> 24) & 0xff;
	}
	return node;
}

AML_CHUNK* aml_add_qword(AML_CHUNK* parent, uint64_t value)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_QWORD;
		node->Length = 8;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value & 0xff;
		node->Buffer[1] = (value >> 8) & 0xff;
		node->Buffer[2] = (value >> 16) & 0xff;
		node->Buffer[3] = (value >> 24) & 0xff;
		node->Buffer[4] = (value >> 32) & 0xff;
		node->Buffer[5] = (value >> 40) & 0xff;
		node->Buffer[6] = (value >> 48) & 0xff;
		node->Buffer[7] = (value >> 56) & 0xff;
	}
	return node;
}

uint32_t aml_fill_simple_name(char* buffer, char* name)
{
	if (strlen(name) < 4) {
		verbose("aml_fill_simple_name: simple name %s has incorrect lengh! Must be 4.\n", name);
		return 0;
	}
	memcpy(buffer, name, 4);
	return 4;
}

uint32_t aml_fill_name(AML_CHUNK* node, char* name)
{
	int len, offset, count;
	uint32_t root = 0;

	if (!node) {
		return 0;
	}

	len = strlen(name);
	offset = 0;
	count = len >> 2;

	if ((len % 4) > 1 || count == 0) {
		verbose("aml_fill_name: pathname %s has incorrect length! Must be 4, 8, 12, 16, etc...\n", name);
		return 0;
	}

	if (((len % 4) == 1) && (name[0] == '\\')) {
		root++;
	}

	if (count == 1) {
		node->Length = (uint16_t)(4 + root);
		node->Buffer = malloc(node->Length+4);
		memcpy(node->Buffer, name, 4 + root);
		offset += 4 + root;
		return (uint32_t)offset;
	}

	if (count == 2) {
		node->Length = 2 + 8;
		node->Buffer = malloc(node->Length+4);
		node->Buffer[offset++] = 0x5c; // Root Char
		node->Buffer[offset++] = 0x2e; // Double name
		memcpy(node->Buffer+offset, name + root, 8);
		offset += 8;
		return (uint32_t)offset;
	}

	node->Length = (uint16_t)(3 + (count << 2));
	node->Buffer = malloc(node->Length+4);
	node->Buffer[offset++] = 0x5c; // Root Char
	node->Buffer[offset++] = 0x2f; // Multi name
	node->Buffer[offset++] = (char)count; // Names count
	memcpy(node->Buffer+offset, name + root, count*4);
	offset += count*4;
	return (uint32_t)offset;
}

AML_CHUNK* aml_add_scope(AML_CHUNK* parent, char* name)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_SCOPE;

		aml_fill_name(node, name);
	}
	return node;
}

AML_CHUNK* aml_add_name(AML_CHUNK* parent, char* name)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_NAME;

		aml_fill_name(node, name);
	}
	return node;
}

AML_CHUNK* aml_add_method(AML_CHUNK* parent, char* name, uint8_t args)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		unsigned int offset = aml_fill_name(node, name);
		node->Type = AML_CHUNK_METHOD;

		node->Length++;
		node->Buffer[offset] = args;

	}
	return node;
}

AML_CHUNK* aml_add_package(AML_CHUNK* parent)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_PACKAGE;

		node->Length = 1;
		node->Buffer = malloc(node->Length);
	}
	return node;
}

AML_CHUNK* aml_add_alias(AML_CHUNK* parent, char* name1, char* name2)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_ALIAS;

		node->Length = 8;
		node->Buffer = malloc(node->Length);
		aml_fill_simple_name(node->Buffer, name1);
		aml_fill_simple_name(node->Buffer+4, name2);
	}

	return node;
}

AML_CHUNK* aml_add_return_name(AML_CHUNK* parent, char* name)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_RETURN;
		aml_fill_name(node, name);
	}

	return node;
}

AML_CHUNK* aml_add_return_byte(AML_CHUNK* parent, uint8_t value)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_RETURN;
		aml_add_byte(node, value);
	}

	return node;
}

AML_CHUNK* aml_add_device(AML_CHUNK* parent, char* name)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_DEVICE;
		aml_fill_name(node, name);
	}

	return node;
}

AML_CHUNK* aml_add_local0(AML_CHUNK* parent)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_LOCAL0;
		node->Length = 1;
	}

	return node;
}

AML_CHUNK* aml_add_store(AML_CHUNK* parent)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_STORE_OP;
		node->Length = 1;
	}

	return node;
}

AML_CHUNK* aml_add_byte_buffer(AML_CHUNK* parent, char* data, uint32_t size)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		int offset = 0;
		node->Type = AML_CHUNK_BUFFER;
		node->Length = (uint8_t)(size + 2);
		node->Buffer = malloc (node->Length);
		node->Buffer[offset++] = AML_CHUNK_BYTE;  //0x0A
		node->Buffer[offset++] = (char)size;
		memcpy(node->Buffer+offset,data, node->Length);
	}

	return node;
}

AML_CHUNK* aml_add_string_buffer(AML_CHUNK* parent, char* StringBuf)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		unsigned int offset = 0;
		unsigned int len = strlen(StringBuf);
		node->Type = AML_CHUNK_BUFFER;
		node->Length = (uint8_t)(len + 3);
		node->Buffer = malloc (node->Length);
		node->Buffer[offset++] = AML_CHUNK_BYTE;
		node->Buffer[offset++] = (char)len;
		memcpy(node->Buffer+offset, StringBuf, len);
		node->Buffer[offset+len] = '\0';
	}

	return node;
}

AML_CHUNK* aml_add_string(AML_CHUNK* parent, char* StringBuf)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		int len = strlen(StringBuf);
		node->Type = AML_CHUNK_STRING;
		node->Length = (uint8_t)(len + 1);
		node->Buffer = malloc (len);
		memcpy(node->Buffer, StringBuf, len);
		node->Buffer[len] = '\0';
	}

	return node;
}

AML_CHUNK* aml_add_return(AML_CHUNK* parent)
{
	AML_CHUNK* node = aml_create_node(parent);

	if (node) {
		node->Type = AML_CHUNK_RETURN;
		//aml_add_byte(node, value);
	}

	return node;
}

uint8_t aml_get_size_length(uint32_t size)
{
	if (size + 1 <= 0x3f)
		return 1;
	else if (size + 2 <= 0xfff) /* Encode in 4 bits and 1 byte */
		return 2;
	else if (size + 3 <= 0xfffff) /* Encode in 4 bits and 2 bytes */
		return 3;

	return 4; /* Encode 0xfffffff in 4 bits and 2 bytes */
}

uint32_t aml_calculate_size(AML_CHUNK* node)
{
	if (node) {
		// Calculate child nodes size
		AML_CHUNK* child = node->First;
		uint8_t child_count = 0;

		node->Size = 0;
		while (child) {
			child_count++;

			node->Size += (uint16_t)aml_calculate_size(child);

			child = child->Next;
		}

		switch (node->Type) {
			case AML_CHUNK_NONE:
			case AML_STORE_OP:
			case AML_CHUNK_LOCAL0:
				node->Size += node->Length;
				break;

			case AML_CHUNK_METHOD:
			case AML_CHUNK_SCOPE:
			case AML_CHUNK_BUFFER:
				node->Size += 1 + node->Length;
				node->Size += aml_get_size_length(node->Size);
				break;

			case AML_CHUNK_DEVICE:
				node->Size += 2 + node->Length;
				node->Size += aml_get_size_length(node->Size);
				break;

			case AML_CHUNK_PACKAGE:
				node->Buffer[0] = child_count;
				node->Size += 1 + node->Length;
				node->Size += aml_get_size_length(node->Size);
				break;
				
			case AML_CHUNK_BYTE:
				if (node->Buffer[0] == 0x0 || node->Buffer[0] == 0x1) {
					node->Size += node->Length;
				} else {
					node->Size += 1 + node->Length;
				}
				break;
				
			case AML_CHUNK_WORD:
			case AML_CHUNK_DWORD:
			case AML_CHUNK_QWORD:
			case AML_CHUNK_ALIAS:
			case AML_CHUNK_NAME:
			case AML_CHUNK_RETURN:
			case AML_CHUNK_STRING:
				node->Size += 1 + node->Length;
				break;
		}
		return node->Size;
	}
	return 0;
}

uint32_t aml_write_byte(uint8_t value, char* buffer, uint32_t offset)
{
	buffer[offset++] = value;

	return offset;
}

uint32_t aml_write_word(uint16_t value, char* buffer, uint32_t offset)
{
	buffer[offset++] = value & 0xff;
	buffer[offset++] = value >> 8;

	return offset;
}

uint32_t aml_write_dword(uint32_t value, char* buffer, uint32_t offset)
{
	buffer[offset++] = value & 0xff;
	buffer[offset++] = (value >> 8) & 0xff;
	buffer[offset++] = (value >> 16) & 0xff;
	buffer[offset++] = (value >> 24) & 0xff;

	return offset;
}

uint32_t aml_write_qword(uint64_t value, char* buffer, uint32_t offset)
{
	buffer[offset++] = value & 0xff;
	buffer[offset++] = (value >> 8) & 0xff;
	buffer[offset++] = (value >> 16) & 0xff;
	buffer[offset++] = (value >> 24) & 0xff;
	buffer[offset++] = (value >> 32) & 0xff;
	buffer[offset++] = (value >> 40) & 0xff;
	buffer[offset++] = (value >> 48) & 0xff;
	buffer[offset++] = (value >> 56) & 0xff;

	return offset;
}

uint32_t aml_write_buffer(const char* value, uint32_t size, char* buffer, uint32_t offset)
{
	if (size > 0) {
		memcpy(buffer + offset, value, size);
	}

	return offset + size;
}

uint32_t aml_write_size(uint32_t size, char* buffer, uint32_t offset)
{
	if (size <= 0x3f) { /* simple 1 byte length in 6 bits */
		buffer[offset++] = (char)size;
	} else if (size <= 0xfff) {
		buffer[offset++] = 0x40 | (size & 0xf); /* 0x40 is type, 0x0X is first nibble of length */
		buffer[offset++] = (size >> 4) & 0xff; /* +1 bytes for rest length */
	} else if (size <= 0xfffff) {
		buffer[offset++] = 0x80 | (size & 0xf); /* 0x80 is type, 0x0X is first nibble of length */
		buffer[offset++] = (size >> 4) & 0xff; /* +2 bytes for rest length */
		buffer[offset++] = (size >> 12) & 0xff;
	} else {
		buffer[offset++] = 0xc0 | (size & 0xf); /* 0xC0 is type, 0x0X is first nibble of length */
		buffer[offset++] = (size >> 4) & 0xff;  /* +3 bytes for rest length */
		buffer[offset++] = (size >> 12) & 0xff;
		buffer[offset++] = (size >> 20) & 0xff;
	}

	return offset;
}

uint32_t aml_write_node(AML_CHUNK* node, char* buffer, uint32_t offset)
{
	if (node && buffer) {
		uint32_t old = offset;
		AML_CHUNK* child = node->First;

		switch (node->Type) {
			case AML_CHUNK_NONE:
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;

			case AML_CHUNK_LOCAL0:
			case AML_STORE_OP:
				offset = aml_write_byte(node->Type, buffer, offset);
				break;

			case AML_CHUNK_DEVICE:
				offset = aml_write_byte(AML_CHUNK_OP, buffer, offset);
				offset = aml_write_byte(node->Type, buffer, offset);
				offset = aml_write_size(node->Size-2, buffer, offset);
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;

			case AML_CHUNK_SCOPE:
			case AML_CHUNK_METHOD:
			case AML_CHUNK_PACKAGE:
			case AML_CHUNK_BUFFER:
				offset = aml_write_byte(node->Type, buffer, offset);
				offset = aml_write_size(node->Size-1, buffer, offset);
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;

			case AML_CHUNK_BYTE:
				if (node->Buffer[0] == 0x0 || node->Buffer[0] == 0x1) {
					offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				} else {
					offset = aml_write_byte(node->Type, buffer, offset);
					offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				}
				break;

			case AML_CHUNK_WORD:
			case AML_CHUNK_DWORD:
			case AML_CHUNK_QWORD:
			case AML_CHUNK_ALIAS:
			case AML_CHUNK_NAME:
			case AML_CHUNK_RETURN:
			case AML_CHUNK_STRING:
				offset = aml_write_byte(node->Type, buffer, offset);
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;

			default:
				break;
		}

		while (child) {
			offset = aml_write_node(child, buffer, offset);

			child = child->Next;
		}

		if (offset - old != node->Size) {
			verbose("Node size incorrect: type=0x%x size=%x offset=%x\n",
				node->Type, node->Size, (offset - old));
		}
	}

	return offset;
}

//the procedure can find array char sizeof N inside part of large array "dsdt" size of len
int32_t FindBin (uint8_t *dsdt, uint32_t len, uint8_t *bin, unsigned int N)
{
	uint32_t i, j;
	bool eq;

	for (i=0; i<len-N; i++) {
	eq = true;
		for (j=0; j<N; j++) {
			if (dsdt[i+j] != bin[j]) {
				eq = false;
				break;
			}
		}
		if (eq) {
			return i;
		}
	}
	return 0;
}

uint32_t get_size(uint8_t* Buffer, uint32_t adr)
{
	uint32_t temp;

	temp = Buffer[adr] & 0xF0; //keep bits 0x30 to check if this is valid size field

	if(temp <= 0x30) {	    // 0
		temp = Buffer[adr];
	} else if(temp == 0x40)	{	// 4
		temp =  (Buffer[adr]   - 0x40)  << 0|
		Buffer[adr+1]          << 4;
	} else if(temp == 0x80)	{	// 8
		temp = (Buffer[adr]   - 0x80)  <<  0|
		Buffer[adr+1]          <<  4|
		Buffer[adr+2]          << 12;
	} else if(temp == 0xC0)	{	// C
		temp = (Buffer[adr]   - 0xC0) <<  0|
		Buffer[adr+1]         <<  4|
		Buffer[adr+2]         << 12|
		Buffer[adr+3]         << 20;
	} else {
		verbose("wrong pointer to size field at %x\n", adr);
		return 0;  //this means wrong pointer to size field
	}
	return temp;
}
