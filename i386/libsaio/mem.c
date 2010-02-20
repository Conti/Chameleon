/*
 * Copyright 2010 AsereBLN. All rights reserved. <aserebln@googlemail.com>
 *
 * mem.c - obtain system memory information
 */

#include "libsaio.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"
#include "mem.h"
#include "smbios_patcher.h"

#ifndef DEBUG_MEM
#define DEBUG_MEM 0
#endif

#if DEBUG_MEM
#define DBG(x...)		printf(x)
#else
#define DBG(x...)
#endif

#define DC(c) (c >= 0x20 && c < 0x7f ? (char) c : '.')
#define STEP 16

void dumpPhysAddr(const char * title, void * a, int len)
{
    int i,j;
    u_int8_t* ad = (u_int8_t*) a;
    char buffer[80];
    char str[16];

    if(ad==NULL) return;

    printf("%s addr=0x%08x len=%04d\n",title ? title : "Dump of ", a, len);
    printf("Ofs-00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F       ASCII\n");
    i = (len/STEP)*STEP;
    for (j=0; j < i; j+=STEP)
    {
        printf("%02x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
               j, 
               ad[j], ad[j+1], ad[j+2], ad[j+3] , ad[j+4], ad[j+5], ad[j+6], ad[j+7],
               ad[j+8], ad[j+9], ad[j+10], ad[j+11] , ad[j+12], ad[j+13], ad[j+14], ad[j+15],
               DC(ad[j]), DC(ad[j+1]), DC(ad[j+2]), DC(ad[j+3]) , DC(ad[j+4]), DC(ad[j+5]), DC(ad[j+6]), DC(ad[j+7]),
               DC(ad[j+8]), DC(ad[j+9]), DC(ad[j+10]), DC(ad[j+11]) , DC(ad[j+12]), DC(ad[j+13]), DC(ad[j+14]), DC(ad[j+15])
               ); 
    }

    if (len%STEP==0) return;
    sprintf(buffer,"%02x:", i);
    for (j=0; j < STEP; j++)  {
        if (j<(len%STEP))
            sprintf(str, " %02x", ad[i+j]);
        else
            strcpy(str, "   " );  
        strncat(buffer, str, sizeof(buffer));
    }
    strncat(buffer,"  ", sizeof(buffer));
    for (j=0; j < (len%STEP); j++)  {
        sprintf(str, "%c", DC(ad[i+j]));  
        strncat(buffer, str, sizeof(buffer));
    }
    printf("%s\n",buffer);
}
void dumpAllTablesOfType(int i)
{
    char title[32];
        struct DMIHeader * dmihdr;
        for(dmihdr = FindFirstDmiTableOfType(i, 4);
            dmihdr;
            dmihdr = FindNextDmiTableOfType(i, 4)) {
            sprintf(title,"Table (type %d) :" , i); 
            dumpPhysAddr(title, dmihdr, dmihdr->length);
        }
}
void scan_memory(PlatformInfo_t *p)
{
    int i=0;
    uint8_t bc=0;
    struct SMBEntryPoint* smbios = NULL;
    struct DMIHeader * dmihdr = NULL;
    
    struct DMIMemoryControllerInfo* ctrlInfo = NULL;
    struct DMIMemoryModuleInfo* memInfo[MAX_RAM_SLOTS];
    
    /*
      struct DMIPhysicalMemoryArray* physMemArray;
      struct DMIMemoryDevice* memDev;
    */
    
    smbios = getSmbios(SMBIOS_ORIGINAL);	/* checks for _SM_ anchor and table header checksum */
    if (smbios==NULL) return ; // getSmbios() return a non null value if smbios is found
    
    ctrlInfo = (struct DMIMemoryControllerInfo*) FindFirstDmiTableOfType(5, 4);
    Platform.DMI.MaxMemorySlots = ctrlInfo ? ctrlInfo->numberOfMemorySlots :  0;
    printf("Number of smbios detected Slots: %02d\n", Platform.DMI.MaxMemorySlots);
        
    Platform.DMI.MemoryModules = 0;
    for(dmihdr = FindFirstDmiTableOfType(6, 4);
        dmihdr;
        dmihdr = FindNextDmiTableOfType(6, 4) ) {
        memInfo[i] = (struct DMIMemoryModuleInfo*) dmihdr;
        bc =  memInfo[i]->bankConnections;
        Platform.RAM.DIMM[i].BankConnCnt =  2;
        if ((bc & 0x0F) == 0x0F)  Platform.RAM.DIMM[i].BankConnCnt--; // 0xF nibble means no connection (3.3.7)
        if ((bc & 0xF0) == 0xF0)  Platform.RAM.DIMM[i].BankConnCnt--; // 0xF nibble means no connection (3.3.7)
        printf("Bank Connection code for Slot %d is %02x, so %d bank connections per slot deducted.\n", 
               i+1, bc, Platform.RAM.DIMM[i].BankConnCnt);

        if ( memInfo[i]->installedSize < 0x7D) // >= 0x7D means error / not installed
            Platform.DMI.MemoryModules++;
        i++;
    }
    Platform.DMI.CntMemorySlots = i;
    getc();
   /*
      physMemArray = (struct DMIPhysicalMemoryArray*) getSmbiosTableStructure(smbios, 16, 0x1);
      memDev = (struct DMIMemoryDevice*) getSmbiosTableStructure(smbios, 17, 0x1);
    */
    /*
      dumpAllTablesOfType(5);
      dumpAllTablesOfType(6 );
      getc();
      dumpAllTablesOfType(16);
      dumpAllTablesOfType(17);
      getc();
    */
}
