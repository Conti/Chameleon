/*
 *  aml_generator.c
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 */

#include "aml_generator.h"

unsigned char aml_get_length_size(long length)
{
	if (length > 0x3F)
		return 2;
	else if (length > 0x3FFF) 
		return 3;
		
	return 1;
}

void aml_add_to_parent(struct aml_chunk* parent, struct aml_chunk* node)
{
	if (parent && node)
	{
		if (!parent->First)
			parent->First = node;
		
		if (parent->Last)
			parent->Last->Next = node;
		
		parent->Last = node;
	}
}

struct aml_chunk* aml_create_node(struct aml_chunk* parent)
{
	struct aml_chunk* node = (void*)malloc(sizeof(struct aml_chunk));
	
	aml_add_to_parent(parent, node);
	
	return node;
}

int aml_add_buffer(struct aml_chunk* parent, const char* buffer, unsigned int size)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_NONE;
		node->Length = size;
		node->Buffer = malloc(node->Length);
		memcpy(node->Buffer, buffer, size);
		
		return node->Length;
	}
	
	return -1;
}

int aml_add_byte(struct aml_chunk* parent, unsigned char value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_BYTE;
		node->Length = 1;
		node->Buffer = malloc(node->Length);
		
		if (value == 0) 
			node->Buffer[0] = 0x00;
		else if (value == 1)
			node->Buffer[0] = 0x01;
		else 
			node->Buffer[0] = value;
		
		return node->Length;
	}
	
	return -1;
}

int aml_add_word(struct aml_chunk* parent, unsigned int value)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node) 
	{
		node->Type = AML_CHUNK_WORD;
		node->Length = 2;
		node->Buffer = malloc(node->Length);
		node->Buffer[0] = value & 0xff;
		node->Buffer[1] = value >> 8;
		
		return node->Length;
	}
	
	return -1;
}

int aml_add_dword(struct aml_chunk* parent, unsigned long value)
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
		
		return node->Length;
	}
	
	return -1;
}

int aml_add_qword(struct aml_chunk* parent, unsigned long long value)
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
		
		return node->Length;
	}
	
	return -1;
}

int aml_fill_simple_name(char* buffer, const char* name)
{
	int i, len = strlen(name), count = 0;
	
	for (i = 0; i < 4; i++) 
	{
		if (i < len && aml_isvalidchar(name[i])) 
		{
			buffer[count++] = name[i];
		}
		else 
		{
			buffer[3-i] = '_';
		}
	}
	
	return 4;
}

int aml_get_names_count(const char* name)
{
	int i, len = strlen(name), count = 0;
	
	for (i = 0; i < len; i++)
	{
		if (name[i] == '.') 
		{
			count++;
		}
		else if (!aml_isvalidchar(name[i]))
		{
			len = i;
			break;
		}
	}
	
	if (count == 0 && len > 0) 
		count++;
	
	return count;
}

int aml_fill_name(struct aml_chunk* node, const char* name)
{
	int i, len = strlen(name), count = 0;
	
	for (i = 0; i < len; i++)
	{
		if (name[i] == '.') 
		{
			count++;
		}
		else if (!aml_isvalidchar(name[i]))
		{
			len = i;
			break;
		}
	}
	
	if (count == 0 && len > 0) 
		count++;
	
	int offset = 0;
	
	if (count == 1) 
	{
		node->Length = 4;
		node->Buffer = malloc(node->Length);
		aml_fill_simple_name(node->Buffer, name);
		return node->Length;
	}
	
	if (count == 2) 
	{
		node->Length = 2 + 8;
		node->Buffer = malloc(node->Length);
		node->Buffer[offset++] = '\\'; // Root
		node->Buffer[offset++] = 0x2e; // Double name
	}
	else 
	{
		node->Length = 3 + count*4;
		node->Buffer[offset++] = '\\'; // Root
		node->Buffer[offset++] = 0x2f; // Multi name
		node->Buffer[offset++] = count; // Names count
	}

	int j = 0;
	
	for (i = 0; i < count; i++) 
	{
		while (name[j] != '.') 
		{
			if (j < len)
			{
				j++;
			}
			else 
			{
				verbose("aml_fill_name: unexpected end of names path!");
				return -1;
			}
		}

		offset += aml_fill_simple_name(node->Buffer + offset, name + j);
	}
	
	return offset;
}

int aml_add_name(struct aml_chunk* parent, const char* name, int count, ...)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_NAME;
			
		aml_fill_name(node, name);
			
		return node->Length;
	}
	
	return -1;
}

int aml_add_scope(struct aml_chunk* parent, const char* name)
{
	struct aml_chunk* node = aml_create_node(parent);
	
	if (node)
	{
		node->Type = AML_CHUNK_SCOPE;
		
		aml_fill_name(node, name);
		
		return node->Length;
	}
	
	return -1;
}