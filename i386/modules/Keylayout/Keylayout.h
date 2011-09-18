/*
 *  Keylayout.h
 *  Chameleon
 *  
 *  Created by JrCs on 28/8/11.
 *  Copyright 2011. All rights reserved.
 *
 */


#ifndef __KEYLAYOUT_H
#define __KEYLAYOUT_H

#define KEYBOARD_LAYOUTS_MAGIC "CHAMLAYT"
#define KEYBOARD_LAYOUTS_MAGIC_SIZE (sizeof(KEYBOARD_LAYOUTS_MAGIC) - 1)
#define KEYBOARD_LAYOUTS_VERSION 3
#define KEYBOARD_LAYOUTS_MAP_OFFSET 0x10 // 0x10 offset of the map in layout file

#define KEYBOARD_MAP_SIZE 0x38

struct keyboard_layout
{
	uint16_t keyboard_map[KEYBOARD_MAP_SIZE];
	uint16_t keyboard_map_shift[KEYBOARD_MAP_SIZE];
	uint16_t keyboard_map_alt[KEYBOARD_MAP_SIZE];
	uint16_t keyboard_map_shift_alt[KEYBOARD_MAP_SIZE];
};

#endif
