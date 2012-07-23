/*
 * Copyright (c) 2010,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "acpi_codec.h"

void ACPICodec_start(void);
void ACPICodec_start(void)
{   
    replace_function("_setupAcpi",&AcpiSetup);        
}