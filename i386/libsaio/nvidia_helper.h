/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */


#ifndef __LIBSAIO_NVIDIA_HELPER_H
#define __LIBSAIO_NVIDIA_HELPER_H

typedef struct cardList_t
{
	char* model;
	uint32_t id;
	uint32_t subid;
	uint64_t videoRam;
	struct cardList_t* next;
} cardList_t;

void add_card(char* model, uint32_t id, uint32_t subid, uint64_t videoRam);
void fill_card_list(void);
cardList_t* FindCardWithIds(uint32_t id, uint32_t subid);

#endif //__LIBSAIO_NVIDIA_HELPER_H