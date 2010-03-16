
/* Copied from 915 resolution created by steve tomljenovic
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 *   - 855resolution by Alain Poirier
 *
 * This source code is into the public domain.
 */

#include "libsaio.h"
#include "915resolution.h"
#include "../boot2/graphics.h"

char * chipset_type_names[] = {
	"UNKNOWN", "830",  "845G", "855GM", "865G", "915G", "915GM", "945G", "945GM", "945GME",
	"946GZ",   "955X", "G965", "Q965", "965GM", "975X",
	"P35", "X48", "B43", "Q45", "P45", "GM45", "G41", "G31", "G45", "500"
};

char * bios_type_names[] = {"UNKNOWN", "TYPE 1", "TYPE 2", "TYPE 3"};

int freqs[] = { 60, 75, 85 };
	

UInt32 get_chipset_id(void) {
	outl(0xcf8, 0x80000000);
	return inl(0xcfc);
}

chipset_type get_chipset(UInt32 id) {
	chipset_type type;
	
	switch (id) {
		case 0x35758086:
			type = CT_830;
			break;
		
		case 0x25608086:
			type = CT_845G;
			break;
				
		case 0x35808086:
			type = CT_855GM;
			break;
				
		case 0x25708086:
			type = CT_865G;
			break;
		
		case 0x25808086:
			type = CT_915G;
			break;
			
		case 0x25908086:
			type = CT_915GM;
			break;
			
		case 0x27708086:
			type = CT_945G;
			break;
			
		case 0x27748086:
			type = CT_955X;
			break;
			
		case 0x277c8086:
			type = CT_975X;
			break;
		
		case 0x27a08086:
			type = CT_945GM;
			break;
			
		case 0x27ac8086:
			type = CT_945GME;
			break;
			
		case 0x29708086:
			type = CT_946GZ;
			break;
			
		case 0x29a08086:
			type = CT_G965;
			break;
			
		case 0x29908086:
			type = CT_Q965;
			break;
			
		case 0x2a008086:
			type = CT_965GM;
			break;
			
		case 0x29e08086:
			type = CT_X48;
			break;			
			
		case 0x2a408086:
			type = CT_GM45;
			break;
			
		case 0x2e108086:
		case 0X2e908086:
			type = CT_B43;
			break;
			
		case 0x2e208086:
			type = CT_P45;
			break;
			
		case 0x2e308086:
			type = CT_G41;
			break;
			
		case 0x29c08086:
			type = CT_G31;
			break;
			
		case 0x29208086:
			type = CT_G45;
			break;
			
		case 0x81008086:
			type = CT_500;
			break;
			
		default:
			type = CT_UNKWN;
			break;
	}
	return type;
}

vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type1 * ptr = ((vbios_resolution_type1*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type2 * ptr = ((vbios_resolution_type2*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type3 * ptr = ((vbios_resolution_type3*)(map->bios_ptr + res)); 
	return ptr;
}

char detect_bios_type(vbios_map * map, char modeline, int entry_size) {
	UInt32 i;
	UInt16 r1, r2;
	    
	r1 = r2 = 32000;
	
	for (i=0; i < map->mode_table_size; i++) {
		if (map->mode_table[i].resolution <= r1) {
			r1 = map->mode_table[i].resolution;
		}
		else {
			if (map->mode_table[i].resolution <= r2) {
				r2 = map->mode_table[i].resolution;
			}
		}
		
		/*printf("r1 = %d  r2 = %d\n", r1, r2);*/
	}

	return (r2-r1-6) % entry_size == 0;
}

char detect_ati_bios_type(vbios_map * map) {	
	return map->mode_table_size % sizeof(ATOM_MODE_TIMING) == 0;
}

void close_vbios(vbios_map * map);

vbios_map * open_vbios(chipset_type forced_chipset) {
	UInt32 z;
	vbios_map * map = NEW(vbios_map);
	for(z=0; z<sizeof(vbios_map); z++) ((char*)map)[z]=0;
	/*
	 * Determine chipset
		  +     */
	
	if (forced_chipset == CT_UNKWN) {
		map->chipset_id = get_chipset_id();
		map->chipset = get_chipset(map->chipset_id);
	}
	else if (forced_chipset != CT_UNKWN) {
		map->chipset = forced_chipset;
	}
	else {
		map->chipset = CT_915GM;
	}
	    
	/*
	 *  Map the video bios to memory
	 */
	
	map->bios_ptr = (unsigned char *)VBIOS_START;
	
	/*
	 * check if we have ATI Radeon
	 */
	
	map->ati_tables.base = map->bios_ptr;
	map->ati_tables.AtomRomHeader = (ATOM_ROM_HEADER *) (map->bios_ptr + *(unsigned short *) (map->bios_ptr + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER)); 
	if (strcmp ((char *) map->ati_tables.AtomRomHeader->uaFirmWareSignature, "ATOM") != 0) {
			printf("Not an AtomBios Card\n");
	} else {
		map->bios = BT_ATI_1;
	}


	/*
	 * check if we have NVidia
	 */
	if (map->bios != BT_ATI_1) {
		int i = 0;
		while (i < 512) { // we don't need to look through the whole bios, just the firs 512 bytes
			if ((map->bios_ptr[i] == 'N') 
				&& (map->bios_ptr[i+1] == 'V') 
				&& (map->bios_ptr[i+2] == 'I') 
				&& (map->bios_ptr[i+3] == 'D')) 
			{
				map->bios = BT_NVDA;
				break;
			}
			i++;
		}
	}
	
	/*
	 * check if we have Intel
	 */
	    
	/*if (map->chipset == CT_UNKWN && memmem(map->bios_ptr, VBIOS_SIZE, INTEL_SIGNATURE, strlen(INTEL_SIGNATURE))) {
		printf( "Intel chipset detected.  However, 915resolution was unable to determine the chipset type.\n");
	
		printf("Chipset Id: %x\n", map->chipset_id);
		
		printf("Please report this problem to stomljen@yahoo.com\n");
		
			close_vbios(map);
			return 0;
		}*/
	
		/*
		 * check for others
		 */
	
	if (map->chipset == CT_UNKWN) {
		printf("Unknown chipset type and unrecognized bios.\n");
		        
		printf("915resolution only works with Intel 800/900 series graphic chipsets.\n");
	
		printf("Chipset Id: %x\n", map->chipset_id);
		close_vbios(map);
		return 0;
	}

	/*
	 * Figure out where the mode table is 
	 */
	
	if ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA)) 
	{
		unsigned char* p = map->bios_ptr + 16;
		unsigned char* limit = map->bios_ptr + VBIOS_SIZE - (3 * sizeof(vbios_mode));
			
		while (p < limit && map->mode_table == 0) {
			vbios_mode * mode_ptr = (vbios_mode *) p;
			            
			if (((mode_ptr[0].mode & 0xf0) == 0x30) && ((mode_ptr[1].mode & 0xf0) == 0x30) &&
				((mode_ptr[2].mode & 0xf0) == 0x30) && ((mode_ptr[3].mode & 0xf0) == 0x30)) {
			
				map->mode_table = mode_ptr;
			}
			            
			p++;
		}
		
		if (map->mode_table == 0) {
			printf("Unable to locate the mode table.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			printf("Chipset: %s\n", chipset_type_names[map->chipset]);
			close_vbios(map);
			return 0;
		}
	} 
	else if (map->bios == BT_ATI_1)
	{
		map->ati_tables.MasterDataTables = (unsigned short *) &((ATOM_MASTER_DATA_TABLE *) (map->bios_ptr + map->ati_tables.AtomRomHeader->usMasterDataTableOffset))->ListOfDataTables;
		unsigned short std_vesa_offset = (unsigned short) ((ATOM_MASTER_LIST_OF_DATA_TABLES *)map->ati_tables.MasterDataTables)->StandardVESA_Timing;
		ATOM_STANDARD_VESA_TIMING * std_vesa = (ATOM_STANDARD_VESA_TIMING *) (map->bios_ptr + std_vesa_offset);
			
		map->ati_mode_table = (char *) &std_vesa->aModeTimings;
		if (map->ati_mode_table == 0) {
			printf("Unable to locate the mode table.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			printf("Chipset: %s\n", chipset_type_names[map->chipset]);
			close_vbios(map);
			return 0;
		}
		map->mode_table_size = std_vesa->sHeader.usStructureSize - sizeof(ATOM_COMMON_TABLE_HEADER);
		
		if (!detect_ati_bios_type(map)) map->bios = BT_ATI_2;
	}
	else if (map->bios == BT_NVDA)
	{
		unsigned short nv_data_table_offset = 0;
		unsigned short nv_modeline_2_offset = 0;
		unsigned short * nv_data_table;
		NV_VESA_TABLE * std_vesa;
		
		int i = 0;
		
		while (i < 0x300) { //We don't need to look for the table in the whole bios, the 768 first bytes only
			if ((map->bios_ptr[i] == 0x44) 
				&& (map->bios_ptr[i+1] == 0x01) 
				&& (map->bios_ptr[i+2] == 0x04) 
				&& (map->bios_ptr[i+3] == 0x00)) {
				nv_data_table_offset = (unsigned short) (map->bios_ptr[i+4] | (map->bios_ptr[i+5] << 8));
				break;
			}
			i++;
		}
		
		while (i < VBIOS_SIZE) { //We don't know how to locate it other way
			if ((map->bios_ptr[i] == 0x00) && (map->bios_ptr[i+1] == 0x04) //this is the first 1024 modeline.
				&& (map->bios_ptr[i+2] == 0x00) && (map->bios_ptr[i+3] == 0x03)
				&& (map->bios_ptr[i+4] == 0x80)
				&& (map->bios_ptr[i+5] == 0x2F)
				&& (map->bios_ptr[i+6] == 0x10)
				&& (map->bios_ptr[i+7] == 0x10)
				&& (map->bios_ptr[i+8] == 0x05)) {
				nv_modeline_2_offset = (unsigned short) i;
				break;
			}
			i++;
		}
		
		nv_data_table = (unsigned short *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
		std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
		
		map->nv_mode_table = (char *) std_vesa->sModelines;
		if (nv_modeline_2_offset == (VBIOS_SIZE-1) || nv_modeline_2_offset == 0) {
			map->nv_mode_table_2 = NULL;
		} else {
			map->nv_mode_table_2 = (char*) map->bios_ptr + nv_modeline_2_offset;
		}
		if (map->nv_mode_table == 0) {
			printf("Unable to locate the mode table.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			printf("Chipset: %s\n", chipset_type_names[map->chipset]);
			close_vbios(map);
			return 0;
		}
		map->mode_table_size = std_vesa->sHeader.usTable_Size;
	}
		
	
	/*
	 * Determine size of mode table
	 */
	    
	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA)) {
		vbios_mode * mode_ptr = map->mode_table;
			
		while (mode_ptr->mode != 0xff) {
			map->mode_table_size++;
			mode_ptr++;
		}
	}
	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
	if ((map->bios != BT_ATI_1) && (map->bios != BT_ATI_2) && (map->bios != BT_NVDA)) {
		if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type3))) {
			map->bios = BT_3;
		}
		else if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type2))) {
			map->bios = BT_2;
		}
		else if (detect_bios_type(map, FALSE, sizeof(vbios_resolution_type1))) {
			map->bios = BT_1;
		}
		else {
			printf("Unable to determine bios type.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			
			printf("Chipset: %s\n", chipset_type_names[map->chipset]);
			printf("Mode Table Offset: $C0000 + $%x\n", ((UInt32)map->mode_table) - ((UInt32)map->bios_ptr));
		
			printf("Mode Table Entries: %u\n", map->mode_table_size);
			return 0;
		}
	}
	
	return map;
}

void close_vbios(vbios_map * map) {
	FREE(map);
}

void unlock_vbios(vbios_map * map) {

	map->unlocked = TRUE;
	    
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			map->b1 = inb(0xcfe);
				
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, 0x33);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_955X:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_G45:
		case CT_500:

			outl(0xcf8, 0x80000090);
			map->b1 = inb(0xcfd);
			map->b2 = inb(0xcfe);
			outl(0xcf8, 0x80000090);
			outb(0xcfd, 0x33);
			outb(0xcfe, 0x33);
		break;
	}
	
	#if DEBUG
	{
		UInt32 t = inl(0xcfc);
		printf("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

void relock_vbios(vbios_map * map) {

	map->unlocked = FALSE;
	
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, map->b1);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_955X:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_G45:
		case CT_500:
			
			outl(0xcf8, 0x80000090);
			outb(0xcfd, map->b1);
			outb(0xcfe, map->b2);
			break;
	}
	
	#if DEBUG
	{
        UInt32 t = inl(0xcfc);
		printf("relock PAM: (0x%08x)\n", t);
	}
	#endif
}

void save_vbios(vbios_map * map)
{
	map->bios_backup_ptr = malloc(VBIOS_SIZE);
	bcopy((const unsigned char *)0xC0000, map->bios_backup_ptr, VBIOS_SIZE);
}

void restore_vbios(vbios_map * map)
{
	bcopy(map->bios_backup_ptr,(unsigned char *)0xC0000, VBIOS_SIZE);
}

   

static void gtf_timings(UInt32 x, UInt32 y, UInt32 freq,
						unsigned long *clock,
						UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
						UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank)
{
	UInt32 hbl, vbl, vfreq;
	
	vbl = y + (y+1)/(20000/(11*freq) - 1) + 1;
	
	vfreq = vbl * freq;
	hbl = 16 * (int)(x * (30 - 300000 / vfreq) /
					 +            (70 + 300000 / vfreq) / 16 + 0);
	
	*vsyncstart = y;
	*vsyncend = y + 3;
	*vblank = vbl - 1;	
	*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1;	
	*hsyncend = x + hbl / 2 - 1;	
	*hblank = x + hbl - 1;	
	*clock = (x + hbl) * vfreq / 1000;
}

void cvt_timings(UInt32 x, UInt32 y, UInt32 freq,
				 unsigned long *clock,
				 UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
				 UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank, bool reduced)
{
	UInt32 hbl, hbp, vbl, vsync, hperiod;
	
	if (!(y % 3) && ((y * 4 / 3) == x))
        vsync = 4;
    else if (!(y % 9) && ((y * 16 / 9) == x))
        vsync = 5;
    else if (!(y % 10) && ((y * 16 / 10) == x))
        vsync = 6;
    else if (!(y % 4) && ((y * 5 / 4) == x))
        vsync = 7;
    else if (!(y % 9) && ((y * 15 / 9) == x))
        vsync = 7;
    else /* Custom */
        vsync = 10;
	
	if (!reduced) {
		hperiod = (1000000/freq - 550) / (y + 3);
		vbl = y + (550/hperiod) + 3;
		hbp = 30 - ((300*hperiod)/1000);
		hbl = (x * hbp) / (100 - hbp);
		
		*vsyncstart = y + 6;
		*vsyncend = *vsyncstart + vsync;
		*vblank = vbl - 1;	
		*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1;	
		*hsyncend = x + hbl / 2 - 1;	
		*hblank = x + hbl - 1;
		
	} else {
		hperiod = (1000000/freq - 460) / y;
		vbl = y + 460/hperiod + 1;
		hbl = 160;
		
		*vsyncstart = y + 3;
		*vsyncend = *vsyncstart + vsync;
		*vblank = vbl - 1;	
		*hsyncstart = x + hbl / 2 - 32;	
		*hsyncend = x + hbl / 2 - 1;	
		*hblank = x + hbl - 1;
		
	}
	*clock = (x + hbl) * 1000 / hperiod;
}

void set_mode(vbios_map * map, UInt32 x, UInt32 y, UInt32 bp, UInt32 htotal, UInt32 vtotal) {
	UInt32 xprev, yprev;
	UInt32 i = 0, j;	// patch first available mode

//	for (i=0; i < map->mode_table_size; i++) {
//		if (map->mode_table[0].mode == mode) {
			switch(map->bios) {
				case BT_1:
					{
						vbios_resolution_type1 * res = map_type1_resolution(map, map->mode_table[i].resolution);
						
						if (bp) {
							map->mode_table[i].bits_per_pixel = bp;
						}
						
						res->x2 = (htotal?(((htotal-x) >> 8) & 0x0f) : (res->x2 & 0x0f)) | ((x >> 4) & 0xf0);
						res->x1 = (x & 0xff);
						
						res->y2 = (vtotal?(((vtotal-y) >> 8) & 0x0f) : (res->y2 & 0x0f)) | ((y >> 4) & 0xf0);
						res->y1 = (y & 0xff);
						if (htotal)
							res->x_total = ((htotal-x) & 0xff);
						
						if (vtotal)
							res->y_total = ((vtotal-y) & 0xff);
					}
					break;
				case BT_2:
					{
						vbios_resolution_type2 * res = map_type2_resolution(map, map->mode_table[i].resolution);
						
						res->xchars = x / 8;
						res->ychars = y / 16 - 1;
						xprev = res->modelines[0].x1;
						yprev = res->modelines[0].y1;
						
						for(j=0; j < 3; j++) {
							vbios_modeline_type2 * modeline = &res->modelines[j];
							
							if (modeline->x1 == xprev && modeline->y1 == yprev) {
								modeline->x1 = modeline->x2 = x-1;
								modeline->y1 = modeline->y2 = y-1;
				
								gtf_timings(x, y, freqs[j], &modeline->clock,
											&modeline->hsyncstart, &modeline->hsyncend,
											&modeline->hblank, &modeline->vsyncstart,
											&modeline->vsyncend, &modeline->vblank);
								
								if (htotal)
									modeline->htotal = htotal;
								else
									modeline->htotal = modeline->hblank;
								
								if (vtotal)
									modeline->vtotal = vtotal;
								else
									modeline->vtotal = modeline->vblank;
							}
						}
					}
					break;
				case BT_3:
					{
						vbios_resolution_type3 * res = map_type3_resolution(map, map->mode_table[i].resolution);
						
						xprev = res->modelines[0].x1;
						yprev = res->modelines[0].y1;
				
						for (j=0; j < 3; j++) {
							vbios_modeline_type3 * modeline = &res->modelines[j];
							                        
							if (modeline->x1 == xprev && modeline->y1 == yprev) {
								modeline->x1 = modeline->x2 = x-1;
								modeline->y1 = modeline->y2 = y-1;
								                            
								gtf_timings(x, y, freqs[j], &modeline->clock,
											&modeline->hsyncstart, &modeline->hsyncend,
											&modeline->hblank, &modeline->vsyncstart,
											&modeline->vsyncend, &modeline->vblank);
								if (htotal)
									modeline->htotal = htotal;
								else
									modeline->htotal = modeline->hblank;
								if (vtotal)
									modeline->vtotal = vtotal;
								else
									modeline->vtotal = modeline->vblank;
						
								modeline->timing_h   = y-1;
								modeline->timing_v   = x-1;
							}
						}
					}
					break;
				case BT_ATI_1:
				{
					edid_mode mode;
					VBEModeInfoBlock  minfo;
					unsigned short    mode_n;
					unsigned short    vesaVersion;
					
					ATOM_MODE_TIMING *mode_timing = (ATOM_MODE_TIMING *) map->ati_mode_table;
					
					mode_n = getVESAModeWithProperties( x, y, 32, maColorModeBit             |
													   maModeIsSupportedBit       |
													   maGraphicsModeBit          |
													   maLinearFrameBufferAvailBit,
													   0,
													   &minfo, &vesaVersion );
					
					if ( mode_n == modeEndOfList )
					{
						minfo.XResolution = 1024;
						minfo.YResolution = 768;
					}
					
					
					int m_status = getMode(&mode);
					
					if (m_status || (mode.h_active != x)) {
						vbios_modeline_type2 * modeline = malloc(sizeof(vbios_modeline_type2));
						bzero(modeline, sizeof(vbios_modeline_type2));
						
						cvt_timings(x, y, 60, &modeline->clock,
									&modeline->hsyncstart, &modeline->hsyncend,
									&modeline->hblank, &modeline->vsyncstart,
									&modeline->vsyncend, &modeline->vblank, FALSE);
						
						mode.pixel_clock = modeline->clock /10;
						mode.h_active = x;
						mode.h_sync_offset = modeline->hsyncstart - x;
						mode.h_sync_width = modeline->hsyncend - modeline->hsyncstart;
						mode.h_blanking = modeline->hblank - x;
						mode.v_active = y;
						mode.v_sync_offset = modeline->vsyncstart - y;
						mode.v_sync_width = modeline->vsyncend - modeline->vsyncstart;
						mode.v_blanking = modeline->vblank - y;
						
						free(modeline);
						m_status = 0;
					}
					
					if (!m_status) {						
						while (i < (map->mode_table_size / sizeof(ATOM_MODE_TIMING)))
						{
							if (mode_timing[i].usCRTC_H_Disp == minfo.XResolution) {								
								mode_timing[i].usCRTC_H_Total = mode.h_active + mode.h_blanking;
								mode_timing[i].usCRTC_H_Disp = mode.h_active;
								mode_timing[i].usCRTC_H_SyncStart = mode.h_active + mode.h_sync_offset;
								mode_timing[i].usCRTC_H_SyncWidth = mode.h_sync_width;
								
								mode_timing[i].usCRTC_V_Total = mode.v_active + mode.v_blanking;
								mode_timing[i].usCRTC_V_Disp = mode.v_active;
								mode_timing[i].usCRTC_V_SyncStart = mode.v_active + mode.v_sync_offset;
								mode_timing[i].usCRTC_V_SyncWidth = mode.v_sync_width;
								
								mode_timing[i].usPixelClock = mode.pixel_clock;
							}
							i++;
						}
					}					
				}
					break;
				case BT_ATI_2:
				{
					edid_mode mode;
					VBEModeInfoBlock  minfo;
					unsigned short    mode_n;
					unsigned short    vesaVersion;
					
					ATOM_DTD_FORMAT *mode_timing = (ATOM_DTD_FORMAT *) map->ati_mode_table;
					
					mode_n = getVESAModeWithProperties( x, y, 32, maColorModeBit             |
													   maModeIsSupportedBit       |
													   maGraphicsModeBit          |
													   maLinearFrameBufferAvailBit,
													   0,
													   &minfo, &vesaVersion );
					
					if ( mode_n == modeEndOfList )
					{
						minfo.XResolution = 1024;
						minfo.YResolution = 768;
					}
					
					
					int m_status = getMode(&mode);
					
					if (m_status || (mode.h_active != x)) {
						vbios_modeline_type2 * modeline = malloc(sizeof(vbios_modeline_type2));
						bzero(modeline, sizeof(vbios_modeline_type2));
						
						cvt_timings(x, y, 60, &modeline->clock,
									&modeline->hsyncstart, &modeline->hsyncend,
									&modeline->hblank, &modeline->vsyncstart,
									&modeline->vsyncend, &modeline->vblank, FALSE);
						
						mode.pixel_clock = modeline->clock /10;
						mode.h_active = x;
						mode.h_sync_offset = modeline->hsyncstart - x;
						mode.h_sync_width = modeline->hsyncend - modeline->hsyncstart;
						mode.h_blanking = modeline->hblank - x;
						mode.v_active = y;
						mode.v_sync_offset = modeline->vsyncstart - y;
						mode.v_sync_width = modeline->vsyncend - modeline->vsyncstart;
						mode.v_blanking = modeline->vblank - y;
						
						free(modeline);
						m_status = 0;
					}
					
					if (!m_status) {						
						while (i < (map->mode_table_size / sizeof(ATOM_DTD_FORMAT)))
						{
							if (mode_timing[i].usHActive == minfo.XResolution) {
								
								mode_timing[i].usHBlanking_Time = mode.h_blanking;
								mode_timing[i].usHActive = mode.h_active;
								mode_timing[i].usHSyncOffset = mode.h_sync_offset;
								mode_timing[i].usHSyncWidth = mode.h_sync_width;
								
								mode_timing[i].usVBlanking_Time = mode.v_blanking;
								mode_timing[i].usVActive = mode.v_active;
								mode_timing[i].usVSyncOffset = mode.v_sync_offset;
								mode_timing[i].usVSyncWidth = mode.v_sync_width;
								
								mode_timing[i].usPixClk = mode.pixel_clock;
							}
							i++;
						}
					}
					
				}
					break;
				case BT_NVDA:
				{
					s_aspect aspect_ratio;
					/*
					 * Get the aspect ratio for the requested mode
					 */
					if ((y * 16 / 9) == x) {
						aspect_ratio.width  = 16;
						aspect_ratio.height = 9;
					} else if ((y * 16 / 10) == x) {
						aspect_ratio.width  = 16;
						aspect_ratio.height = 10;
					} else if ((y * 5 / 4) == x) {
						aspect_ratio.width  = 5;
						aspect_ratio.height = 4;
					} else if ((y * 15 / 9) == x) {
						aspect_ratio.width  = 15;
						aspect_ratio.height = 9;
					} else {
						aspect_ratio.width  = 4;
						aspect_ratio.height = 3;
					}
					
					NV_MODELINE *mode_timing = (NV_MODELINE *) map->nv_mode_table;
					NV_MODELINE_2 *mode_timing_2 = (NV_MODELINE_2 *) map->nv_mode_table_2;
					
					i = 0;
					if (mode_timing_2[i].h_disp == 0x140) { //From 320x200 mode.
						while (mode_timing_2[i].h_disp <= 0x800) {
							
							vbios_modeline_type2 * modeliner = malloc(sizeof(vbios_modeline_type2));
							bzero(modeliner, sizeof(vbios_modeline_type2));
							
							x = mode_timing_2[i].h_disp;
							y = x * aspect_ratio.height / aspect_ratio.width;
							
							cvt_timings(x, y, 60, &modeliner->clock,
										&modeliner->hsyncstart, &modeliner->hsyncend,
										&modeliner->hblank, &modeliner->vsyncstart,
										&modeliner->vsyncend, &modeliner->vblank, TRUE);
							
							mode_timing_2[i].h_disp = x;
							mode_timing_2[i].v_disp = y;
							mode_timing_2[i].h_blank = modeliner->hblank - x;
							mode_timing_2[i].h_syncoffset = modeliner->hsyncstart - x;
							mode_timing_2[i].h_syncwidth = modeliner->hsyncend - modeliner->hsyncstart;
							mode_timing_2[i].v_blank = modeliner->vblank - y;
							i++;
							free(modeliner);	
						}
					} 
					i = 0;
					
					while ((mode_timing[i].reserved3 & 0xff) == 0xff) {
						x = mode_timing[i].usH_Active;
						y = x * aspect_ratio.height / aspect_ratio.width;
						
						vbios_modeline_type2 * modeliner = malloc(sizeof(vbios_modeline_type2));
						bzero(modeliner, sizeof(vbios_modeline_type2));
						
						cvt_timings(x, y, 60, &modeliner->clock,
									&modeliner->hsyncstart, &modeliner->hsyncend,
									&modeliner->hblank, &modeliner->vsyncstart,
									&modeliner->vsyncend, &modeliner->vblank, FALSE);
						
						mode_timing[i].usH_Total = x + modeliner->hblank;
						mode_timing[i].usH_Active = x;
						mode_timing[i].usH_Active_minus_One = x - 1;
						mode_timing[i].usH_Active_minus_One_ = x - 1;
						mode_timing[i].usH_Active = y;
						mode_timing[i].usH_SyncStart = modeliner->hsyncstart;
						mode_timing[i].usH_SyncEnd = modeliner->hsyncend;
						
						mode_timing[i].usV_Total = y + modeliner->vblank;
						mode_timing[i].usV_Active = y;
						mode_timing[i].usV_Active_minus_One = y - 1;
						mode_timing[i].usV_Active_minus_One_ = y - 1;
						mode_timing[i].usV_SyncStart = modeliner->vsyncend;
						mode_timing[i].usV_SyncEnd = modeliner->vsyncend;
						
						mode_timing[i].usPixel_Clock = modeliner->clock;
						
						i++;
					}
				}
					
					break;
				case BT_UNKWN:
					break;
			}
//		}
//	}
}   

