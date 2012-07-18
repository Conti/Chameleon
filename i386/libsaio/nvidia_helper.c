/*
 * Copyright (c) 2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "xml.h"
#include "nvidia_helper.h"
#include "pci.h"
#include "nvidia.h"

/*
 
 NVIDIA card injection usage e.g (to be placed in the boot.plist): 
 
 <key>NVIDIA</key>
 <array>
 <dict>
 <key>Chipset Name</key>
 <string>Quadro FX 380</string>
 <key>IOPCIPrimaryMatch</key>
 <string>0x10DE0658</string>
 <key>VRam Size</key>
 <string>256</string>
 </dict>
 <dict>
 <key>Chipset Name</key>
 <string>YOUR_CARD_NAME</string>
 <key>IOPCIPrimaryMatch</key>
 <string>YOUR_CARD_ID</string>
 <key>IOPCISubDevId</key>
 <string>YOUR_CARD_SUB_ID(if necessary)</string>
 <key>VRam Size</key>
 <string>YOUR_CARD_VRAM_SIZE</string>
 </dict>
 <dict>
 <key>Chipset Name</key>
 <string>YOUR_SECOND_CARD_NAME</string>
 <key>IOPCIPrimaryMatch</key>
 <string>YOUR_SECOND_CARD_ID</string>
 <key>IOPCISubDevId</key>
 <string>YOUR_SECOND_CARD_SUB_ID(if necessary)</string>
 <key>VRam Size</key>
 <string>YOUR_SECOND_CARD_VRAM_SIZE</string>
 </dict>
 .
 .
 .
 .
 </array>
 
 */

cardList_t* cardList = NULL;

void add_card(char* model, uint32_t id, uint32_t subid, uint64_t videoRam)
{
	
	cardList_t* new_card = malloc(sizeof(cardList_t));
	if (new_card)
	{	
		new_card->next = cardList;
		
		cardList = new_card;
		
		new_card->id = id;
		new_card->subid = subid;
		new_card->videoRam = videoRam;
		new_card->model = model;
	}	
}

cardList_t* FindCardWithIds(uint32_t id, uint32_t subid)
{
	cardList_t* entry = cardList;
	while(entry)
	{		
		if((entry->id == id) && (entry->subid == subid))
		{
			return entry;
		}
		else
		{
			entry = entry->next;
		}             
		
	}
	
	// LET A SECOND CHANCE by seaching only for the device-id
	entry = cardList;
	while(entry)
	{		
		if((entry->id == id))
		{
			return entry;
		}
		else
		{
			entry = entry->next;
		}             
		
	}
	
	return NULL;
}

void fill_card_list(void) 
{
	unsigned int	i, count;
	TagPtr NVDIATag;                           
	char *model_name = NULL, *match_id = NULL, *sub_id = NULL, *vram_size = NULL;	
	uint32_t dev_id = 0, subdev_id = NV_SUB_IDS;	
	uint64_t  VramSize = 0;	
	
	if ((NVDIATag = XMLCastArray(XMLGetProperty(bootInfo->chameleonConfig.dictionary, (const char*)"NVIDIA"))))
	{
		count = XMLTagCount(NVDIATag);
		
		for (i=0; i<count; i++) 
		{
			TagPtr element = XMLGetElement( NVDIATag, i );
			if (element) 
			{
				match_id   = XMLCastString(XMLGetProperty(element, (const char*)"IOPCIPrimaryMatch")); //device-id
				sub_id   = XMLCastString(XMLGetProperty(element, (const char*)"IOPCISubDevId")); //sub device-id
				model_name  = XMLCastString(XMLGetProperty(element, (const char*)"Chipset Name"));
				vram_size  = XMLCastString(XMLGetProperty(element, (const char*)"VRam Size"));
				
				if (match_id) {
					dev_id = strtoul(match_id, NULL, 16);
				}
				
				if (sub_id) {
					subdev_id = strtoul(sub_id, NULL, 16);
				}
				
				if (vram_size) {
					VramSize = strtoul(vram_size, NULL, 10);
				}
				
				add_card(model_name, dev_id, subdev_id, VramSize);								
			}
		}	
	}	
}