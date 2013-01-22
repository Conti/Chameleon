//
//  FileNVRAM.h
//  FileNVRAM
//
//  Created by Evan Lojewski on 1/18/13.
//  Copyright (c) 2013 xZenue LLC. All rights reserved.
//

#ifndef FILENVRAM_H
#define FILENVRAM_H

#include <libsaio.h>
#include <xml.h>

TagPtr getNVRAMVariable(char* key);
void addNVRAMVariable(char* key, TagPtr entry);
void removeNVRAMVariable(char* key);


#endif /* FILENVRAM_H */