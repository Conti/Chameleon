/*
 *  aml_generator.c
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 */

#include "aml_generator.h"

bool aml_add_to_parent(struct aml_chunk* parent, struct aml_chunk* node)
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
				verbose("aml_add_to_parent: Node isn't supports child nodes!");
				return false;
			case AML_CHUNK_NAME:
				if (parent->First) 
				{
					verbose("aml_add_to_parent: Name node could have only one child node!");
					return false;
				}
				break;

			default:
				break;
		}
		
		if (!parent->First)
			parent->First = node;
		
		if (parent->Last)
			parent->Last->Next = node;
		
		parent->Last = node;
		
		return true;
	}
	
	return false;
}

struct aml_chunk* aml_create_node(struct aml_chunk* parent)
{
	struct aml_chunk* node = (struct aml_chunk*)malloc(sizeof(struct aml_chunk));
	
	aml_add_to_parent(parent, node);
	
	return node;
}

void aml_destroy_node(struct aml_chunk* node)
{
	// Delete child nodes
	struct aml_chunk* child = node->First;
	
	while (child) 
	{
		struct aml_chunk* next = child->Next;
		
		if (child->Buffer)
			free(child->Buffer);
		
		free(child);
		
		child = next;
	}
	
	// Free node
	if (node->Buffer)
		free(node->Buffer);
	
	free(node);
}

struct aml_chunk* aml_add_buffer(struct aml_chunk* parent, const char* buffer, unsigned int size)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_NONE;
		node->Length = size;
		node->Buffer = malloc(node->Length);
		memcpy(node->Buffer, buffer, node->Length);
	}
	
	return node;
}

struct aml_chunk* aml_add_byte(struct aml_chunk* parent, unsigned char value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_BYTE;
		
		node->Length = 1;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value;
	}
	
	return node;
}

struct aml_chunk* aml_add_word(struct aml_chunk* parent, unsigned int value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_WORD;
		node->Length = 2;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value & 0xff;
		node->Buffer[1] = value >> 8;
	}
	
	return node;
}

struct aml_chunk* aml_add_dword(struct aml_chunk* parent, unsigned long value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
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

struct aml_chunk* aml_add_qword(struct aml_chunk* parent, unsigned long long value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
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

unsigned int aml_fill_simple_name(char* buffer, const char* name)
{
	if (strlen(name) < 4) 
	{
		verbose("aml_fill_simple_name: simple name %s has incorrect lengh! Must be 4", name);
		return 0;
	}
	
	memcpy(buffer, name, 4);
	return 4;
}

unsigned int aml_fill_name(struct aml_chunk* node, const char* name)
{
	if (!node) 
		return 0;
	
	int len = strlen(name), offset = 0, count = len / 4;
	
	if ((len % 4) > 1 || count == 0) 
	{
		verbose("aml_fill_name: pathname %s has incorrect length! Must be 4, 8, 12, 16 etc.", name);
		return 0;
	}
	
	unsigned int root = 0;
	
	if ((len % 4) == 1 && name[0] == '\\')
		root++;
			
	if (count == 1) 
	{
		node->Length = 4 + root;
		node->Buffer = malloc(node->Length);
		memcpy(node->Buffer, name, 4 + root);
		return node->Length;
	}
	
	if (count == 2) 
	{
		node->Length = 2 + 8;
		node->Buffer = malloc(node->Length);
		node->Buffer[offset++] = 0x5c; // Root Char
		node->Buffer[offset++] = 0x2e; // Double name
		memcpy(node->Buffer+offset, name + root, 8);
		return node->Length;
	}
	
	node->Length = 3 + count*4;
	node->Buffer = malloc(node->Length);
	node->Buffer[offset++] = 0x5c; // Root Char
	node->Buffer[offset++] = 0x2f; // Multi name
	node->Buffer[offset++] = count; // Names count
	memcpy(node->Buffer+offset, name + root, count*4);
	
	return node->Length;
}

struct aml_chunk* aml_add_scope(struct aml_chunk* parent, const char* name)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_SCOPE;
		
		aml_fill_name(node, name);
	}
	
	return node;
}

struct aml_chunk* aml_add_name(struct aml_chunk* parent, const char* name)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_NAME;
		
		aml_fill_name(node, name);
	}
	
	return node;
}

struct aml_chunk* aml_add_package(struct aml_chunk* parent)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_PACKAGE;
		
		node->Length = 1;
		node->Buffer = malloc(node->Length);
	}
	
	return node;
}

struct aml_chunk* aml_add_alias(struct aml_chunk* parent, const char* name1, const char* name2)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_ALIAS;
		
		node->Length = 8;
		node->Buffer = malloc(node->Length);
		aml_fill_simple_name(node->Buffer, name1);
		aml_fill_simple_name(node->Buffer+4, name2);
	}
	
	return node;
}

unsigned char aml_get_size_length(unsigned int size)
{
	if (size + 1 <= 0x3f)
		return 1;
	else if (size + 2 <= 0x3fff)
		return 2;
	else if (size + 3 <= 0x3fffff)
		return 3;
	
	return 4;
}

unsigned int aml_calculate_size(struct aml_chunk* node)
{
	if (node)
	{
		node->Size = 0;
		
		// Calculate child nodes size
		struct aml_chunk* child = node->First;
		unsigned char child_count = 0;
		
		while (child) 
		{
			child_count++;
			
			node->Size += aml_calculate_size(child);
			
			child = child->Next;
		}
		
		switch (node->Type) 
		{
			case AML_CHUNK_NONE:
				node->Size += node->Length;
				break;
			case AML_CHUNK_SCOPE:
				node->Size += 1 + node->Length;
				node->Size += aml_get_size_length(node->Size);
				break;
			case AML_CHUNK_PACKAGE:
				node->Buffer[0] = child_count;
				node->Size += 1 + node->Length;
				node->Size += aml_get_size_length(node->Size);
				break;
				
			case AML_CHUNK_BYTE:
				if (node->Buffer[0] == 0x0 || node->Buffer[0] == 0x1) 
				{
					node->Size += node->Length;
				}
				else 
				{
					node->Size += 1 + node->Length;
				}
				
				break;
				
			case AML_CHUNK_WORD:
			case AML_CHUNK_DWORD:
			case AML_CHUNK_QWORD:
			case AML_CHUNK_ALIAS:
			case AML_CHUNK_NAME:
				node->Size += 1 + node->Length;
				break;
		}
		
		return node->Size;
	}
	
	return 0;
}

unsigned int aml_write_byte(unsigned char value, char* buffer, unsigned int offset)
{
	buffer[offset++] = value;
	
	return offset;
}

unsigned int aml_write_word(unsigned int value, char* buffer, unsigned int offset)
{
	buffer[offset++] = value & 0xff;
	buffer[offset++] = value >> 8;
	
	return offset;
}

unsigned int aml_write_dword(unsigned long value, char* buffer, unsigned int offset)
{
	buffer[offset++] = value & 0xff;
	buffer[offset++] = (value >> 8) & 0xff;
	buffer[offset++] = (value >> 16) & 0xff;
	buffer[offset++] = (value >> 24) & 0xff;
	
	return offset;
}

unsigned int aml_write_qword(unsigned long long value, char* buffer, unsigned int offset)
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

unsigned int aml_write_buffer(const char* value, unsigned int size, char* buffer, unsigned int offset)
{
	if (size > 0)
	{
		memcpy(buffer + offset, value, size);
	}
	
	return offset + size;
}

unsigned int aml_write_size(unsigned int size, char* buffer, unsigned int offset)
{
	if (size <= 0x3f)
	{
		buffer[offset++] = size;
	}
	else if (size <= 0x3fff) 
	{
		buffer[offset++] = 0x40 | (size & 0xf);
		buffer[offset++] = (size >> 4) & 0xff;
	}
	else if (size <= 0x3fffff) 
	{
		buffer[offset++] = 0x80 | (size & 0xf);
		buffer[offset++] = (size >> 4) & 0xff;
		buffer[offset++] = (size >> 12) & 0xff;
	}
    else 
	{
		buffer[offset++] = 0xc0 | (size & 0xf);
		buffer[offset++] = (size >> 4) & 0xff;
		buffer[offset++] = (size >> 12) & 0xff;
		buffer[offset++] = (size >> 20) & 0xff;
	}
	
	return offset;
}

unsigned int aml_write_node(struct aml_chunk* node, char* buffer, unsigned int offset)
{
	if (node && buffer) 
	{
		unsigned int old = offset;
		
		switch (node->Type) 
		{
			case AML_CHUNK_NONE:
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;

			case AML_CHUNK_SCOPE:
			case AML_CHUNK_PACKAGE:
				offset = aml_write_byte(node->Type, buffer, offset);
				offset = aml_write_size(node->Size-1, buffer, offset);
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;
				
			case AML_CHUNK_BYTE:
				if (node->Buffer[0] == 0x0 || node->Buffer[0] == 0x1) 
				{
					offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				}
				else 
				{
					offset = aml_write_byte(node->Type, buffer, offset);
					offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				}
				break;
				
			case AML_CHUNK_WORD:
			case AML_CHUNK_DWORD:
			case AML_CHUNK_QWORD:
			case AML_CHUNK_ALIAS:
			case AML_CHUNK_NAME:
				offset = aml_write_byte(node->Type, buffer, offset);
				offset = aml_write_buffer(node->Buffer, node->Length, buffer, offset);
				break;
				
			default:
				break;
		}

		struct aml_chunk* child = node->First;
		
		while (child) 
		{
			offset = aml_write_node(child, buffer, offset);
			
			child = child->Next;
		}
		
		if (offset - old != node->Size) 
			verbose("Node size incorrect: 0x%x\n", node->Type);
	}
	
	return offset;
}