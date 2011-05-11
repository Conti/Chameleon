/*
 *  edid.c
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */


#include "edid.h"
#include "vbe.h"
#include "graphics.h"
 


void getResolution(UInt32* x, UInt32* y, UInt32* bp)
{
	static UInt32 xResolution, yResolution, bpResolution;

	bpResolution = 32;	// assume 32bits
	
	if(!xResolution || !yResolution || !bpResolution)
	{
		
		char* edidInfo = readEDID();
		
		if(!edidInfo) return;
		
		// TODO: check *all* resolutions reported and either use the highest, or the native resolution (if there is a flag for that)
		xResolution =  edidInfo[56] | ((edidInfo[58] & 0xF0) << 4);
		yResolution = edidInfo[59] | ((edidInfo[61] & 0xF0) << 4);
		
		//printf("H Active = %d", edidInfo[56] | ((edidInfo[58] & 0xF0) << 4) );
		//printf("V Active = %d", edidInfo[59] | ((edidInfo[61] & 0xF0) << 4) );
		
		free( edidInfo );
		
		if(!xResolution) xResolution = DEFAULT_SCREEN_WIDTH;
		if(!yResolution) yResolution = DEFAULT_SCREEN_HEIGHT;

	}

	*x  = xResolution;
	*y  = yResolution;
	*bp = bpResolution;

}

char* readEDID()
{
	SInt16 last_reported = -1;
	UInt8 edidInfo[EDID_BLOCK_SIZE];

	UInt8 header1[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	UInt8 header2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
	SInt16 status;
	UInt16 blocks_left = 1;
	
	do
	{
		// TODO: This currently only retrieves the *last* block, make the block buffer expand as needed / calculated from the first block

		bzero( edidInfo, EDID_BLOCK_SIZE);

		status = getEDID(edidInfo, blocks_left);
		
		
		//printf("Buffer location: 0x%X\n", SEG(buffer) << 16 | OFF(buffer));
		/*
		int j, i;
		for (j = 0; j < 8; j++) {
			for(i = 0; i < 16; i++) printf("0x%X ", ebiosInfo[((i+1) * (j + 1)) - 1]);

		}
		printf("\n");
		*/
		
		if(status == 0)
		{
			//if( edidInfo[0] == 0x00 || edidInfo[0] == 0xFF)
			if((memcmp(edidInfo, header1, sizeof(header1)) != 0) ||
			   (memcmp(edidInfo, header2, sizeof(header2)) != 0) )
			{
				blocks_left--;
				int reported = edidInfo[ EDID_V1_BLOCKS_TO_GO_OFFSET ];
				
				if ( reported > blocks_left )
				{
					
					printf("EDID claims %d more blocks left\n", reported);
				}
				
				if ( (last_reported <= reported && last_reported != -1)
					|| reported == 0xff
					/* 0xff frequently comes up in corrupt edids */
					//|| reported == MAGIC
					)
				{
					printf("Last reported %d\n", last_reported);
					printf( "EDID blocks left is wrong.\n"
						   "Your EDID is probably invalid.\n");
					return 0;
				}
				else
				{
					//printf("Reading EDID block\n");
					//printf("H Active = %d", ebiosInfo[56] | ((ebiosInfo[58] & 0xF0) << 4) );
					//printf("V Active = %d", ebiosInfo[59] | ((ebiosInfo[61] & 0xF0) << 4) );

					last_reported = reported;
					blocks_left = reported;
				}
			} 
			else
			{
				printf("Invalid block %d\n", blocks_left);
				printf("Header1 = %d", memcmp(edidInfo, header1, sizeof(header1)) );
				printf("Header2 = %d", memcmp(edidInfo, header2, sizeof(header2)) );
				return 0;
			}
		}
		blocks_left = 0;	
	} while(blocks_left);

	char* ret = malloc(sizeof(edidInfo));
	memcpy(ret, edidInfo, sizeof(edidInfo));
	return ret;
}


int getEDID( void * edidBlock, UInt8 block)
{
	biosBuf_t bb;
	
	bzero(&bb, sizeof(bb));
    bb.intno  = 0x10;
    bb.eax.rr = 0x4F15;
	bb.ebx.r.l= 0x01;
	bb.edx.rr = block;
	
    bb.es     = SEG( edidBlock );
    bb.edi.rr = OFF( edidBlock );
	
    bios( &bb );
    return(bb.eax.r.h);
}

