/*
 *  gui.c
 *  
 *
 *  Created by Jasmin Fazlic on 18.12.08.
 *  Copyright 2008/09 Jasmin Fazlic All rights reserved.
 *  Copyright 2008/09 iNDi All rights reserved.
 *
 */

#include "gui.h"
#include "term.h"
#include "appleboot.h"
#include "vers.h"

#define IMG_REQUIRED -1
#define THEME_NAME_DEFAULT	"Default"
static const char *theme_name = THEME_NAME_DEFAULT;

#ifdef CONFIG_EMBED_THEME
#include "art.h"
#endif

#define LOADPNG(img, alt_img) if (loadThemeImage(#img, alt_img) != 0) { return 1; }

#define VIDEO(x) (bootArgs->Video.v_ ## x)

#define vram VIDEO(baseAddr)

#define TAB_PIXELS_WIDTH (font->chars[0]->width * 4) // tab = 4 spaces

int lasttime = 0; // we need this for animating maybe


/*
 * ATTENTION: the enum and the following array images[] MUST match !!!
 */
enum {
	iBackground = 0,
	iLogo,

	iDeviceGeneric,
	iDeviceGeneric_o,
	iDeviceHFS,
	iDeviceHFS_o,
	iDeviceHFS_mav,
	iDeviceHFS_mav_o,
	iDeviceHFS_ML,
	iDeviceHFS_ML_o,
	iDeviceHFS_Lion,
	iDeviceHFS_Lion_o,
	iDeviceHFS_SL,
	iDeviceHFS_SL_o,
	iDeviceHFS_Leo,
	iDeviceHFS_Leo_o,
	iDeviceHFS_Tiger,
	iDeviceHFS_Tiger_o,

	iDeviceHFSRAID,
	iDeviceHFSRAID_o,
	iDeviceHFSRAID_mav,
	iDeviceHFSRAID_mav_o,
	iDeviceHFSRAID_ML,
	iDeviceHFSRAID_ML_o,
	iDeviceHFSRAID_Lion,
	iDeviceHFSRAID_Lion_o,
	iDeviceHFSRAID_SL,
	iDeviceHFSRAID_SL_o,
	iDeviceHFSRAID_Leo,
	iDeviceHFSRAID_Leo_o,
	iDeviceHFSRAID_Tiger,
	iDeviceHFSRAID_Tiger_o,
	iDeviceEXT3,
	iDeviceEXT3_o,
	iDeviceFreeBSD,     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	iDeviceFreeBSD_o,   /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	iDeviceOpenBSD,     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	iDeviceOpenBSD_o,   /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	iDeviceBEFS,        /* Haiku detection and Icon credits to scorpius  */
	iDeviceBEFS_o,      /* Haiku detection and Icon credits to scorpius  */
	iDeviceFAT,
	iDeviceFAT_o,
	iDeviceFAT16,
	iDeviceFAT16_o,
	iDeviceFAT32,
	iDeviceFAT32_o,
	iDeviceNTFS,
	iDeviceNTFS_o,
	iDeviceCDROM,
	iDeviceCDROM_o,

	iSelection,
	iDeviceScrollPrev,
	iDeviceScrollNext,

	iMenuBoot,
	iMenuVerbose,
	iMenuIgnoreCaches,
	iMenuSingleUser,
	iMenuMemoryInfo,
	iMenuVideoInfo,
	iMenuHelp,
	iMenuVerboseDisabled,
	iMenuIgnoreCachesDisabled,
	iMenuSingleUserDisabled,
	iMenuSelection,

	iProgressBar,
	iProgressBarBackground,

	iTextScrollPrev,
	iTextScrollNext,

	iFontConsole,
	iFontSmall,
};

image_t images[] = {
	{.name = "background",                  .image = NULL},
	{.name = "logo",                        .image = NULL},

	{.name = "device_generic",              .image = NULL},
	{.name = "device_generic_o",            .image = NULL},
	{.name = "device_hfsplus",              .image = NULL},
	{.name = "device_hfsplus_o",            .image = NULL},
	{.name = "device_hfsplus_mav",          .image = NULL},
	{.name = "device_hfsplus_mav_o",        .image = NULL},
	{.name = "device_hfsplus_ml",           .image = NULL},
	{.name = "device_hfsplus_ml_o",         .image = NULL},
	{.name = "device_hfsplus_lion",         .image = NULL},
	{.name = "device_hfsplus_lion_o",       .image = NULL},
	{.name = "device_hfsplus_sl",           .image = NULL},
	{.name = "device_hfsplus_sl_o",         .image = NULL},
	{.name = "device_hfsplus_leo",          .image = NULL},
	{.name = "device_hfsplus_leo_o",        .image = NULL},
	{.name = "device_hfsplus_tiger",        .image = NULL},
	{.name = "device_hfsplus_tiger_o",      .image = NULL},

	{.name = "device_hfsraid",              .image = NULL},
	{.name = "device_hfsraid_o",            .image = NULL},
	{.name = "device_hfsraid_mav",          .image = NULL},
	{.name = "device_hfsraid_mav_o",        .image = NULL},
	{.name = "device_hfsraid_ml",           .image = NULL},
	{.name = "device_hfsraid_ml_o",         .image = NULL},
	{.name = "device_hfsraid_lion",         .image = NULL},
	{.name = "device_hfsraid_lion_o",       .image = NULL},
	{.name = "device_hfsraid_sl",           .image = NULL},
	{.name = "device_hfsraid_sl_o",         .image = NULL},
	{.name = "device_hfsraid_leo",          .image = NULL},
	{.name = "device_hfsraid_leo_o",        .image = NULL},
	{.name = "device_hfsraid_tiger",        .image = NULL},
	{.name = "device_hfsraid_tiger_o",      .image = NULL},
	{.name = "device_ext3",                 .image = NULL},
	{.name = "device_ext3_o",               .image = NULL},
	{.name = "device_freebsd",              .image = NULL},     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	{.name = "device_freebsd_o",            .image = NULL},     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	{.name = "device_openbsd",              .image = NULL},     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	{.name = "device_openbsd_o",            .image = NULL},     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	{.name = "device_befs",                 .image = NULL},     /* Haiku detection and Icon credits to scorpius  */
	{.name = "device_befs_o",               .image = NULL},     /* Haiku detection and Icon credits to scorpius  */
	{.name = "device_fat",                  .image = NULL},
	{.name = "device_fat_o",                .image = NULL},
	{.name = "device_fat16",                .image = NULL},
	{.name = "device_fat16_o",              .image = NULL},
	{.name = "device_fat32",                .image = NULL},
	{.name = "device_fat32_o",              .image = NULL},
	{.name = "device_ntfs",                 .image = NULL},
	{.name = "device_ntfs_o",               .image = NULL},
	{.name = "device_cdrom",                .image = NULL},
	{.name = "device_cdrom_o",              .image = NULL},

	{.name = "device_selection",            .image = NULL},
	{.name = "device_scroll_prev",          .image = NULL},
	{.name = "device_scroll_next",          .image = NULL},

	{.name = "menu_boot",                   .image = NULL},
	{.name = "menu_verbose",                .image = NULL},
	{.name = "menu_ignore_caches",          .image = NULL},
	{.name = "menu_single_user",            .image = NULL},
	{.name = "menu_memory_info",            .image = NULL},
	{.name = "menu_video_info",             .image = NULL},
	{.name = "menu_help",                   .image = NULL},
	{.name = "menu_verbose_disabled",       .image = NULL},
	{.name = "menu_ignore_caches_disabled", .image = NULL},
	{.name = "menu_single_user_disabled",   .image = NULL},
	{.name = "menu_selection",              .image = NULL},

	{.name = "progress_bar",                .image = NULL},
	{.name = "progress_bar_background",     .image = NULL},

	{.name = "text_scroll_prev",            .image = NULL},
	{.name = "text_scroll_next",            .image = NULL},

	{.name = "font_console",                .image = NULL},
	{.name = "font_small",                  .image = NULL},
};

int imageCnt = 0;

extern int	gDeviceCount;
extern int	selectIndex;

extern MenuItem *menuItems;

//char prompt[BOOT_STRING_LEN];
extern char   gBootArgs[BOOT_STRING_LEN];

char prompt_text[] = "boot: ";

menuitem_t infoMenuItems[] =
{
	{ .text = "Boot" },
	{ .text = "Boot Verbose" },
	{ .text = "Boot Ignore Caches" },
	{ .text = "Boot Single User" },
	{ .text = "Memory Info" },
	{ .text = "Video Info" },
	{ .text = "Help" }
};

int  initFont(font_t *font, image_t *image);
int  destroyFont(font_t *font);
void colorFont(font_t *font, uint32_t color);
void makeRoundedCorners(pixmap_t *p);

static int infoMenuSelection = 0;
static int infoMenuItemsCount = sizeof(infoMenuItems)/sizeof(infoMenuItems[0]);

static bool infoMenuNativeBoot = false;

// here we store the used screen resolution
static unsigned long screen_params[4] = {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 32, 0};

static int getImageIndexByName(const char *name)
{
	int i;
	for (i = 0; i < sizeof(images) / sizeof(images[0]); i++) {
		if (strcmp(name, images[i].name) == 0) {
			return i; // found the name
		}
	}
	return -1;
}

#ifdef CONFIG_EMBED_THEME
static int getEmbeddedImageIndexByName(const char *name)
{
	int upperLimit = sizeof(embeddedImages) / sizeof(embeddedImages[0]) - 1;
	int lowerLimit = 0;
	int compareIndex = (upperLimit - lowerLimit) >> 1; // Midpoint
	int result;
	
	// NOTE: This algorithm assumes that the embedded images are sorted.
	// This is currently done using the make file. If the array is
	// generated manualy, this *will* fail to work properly.
	while((result = strcmp(name, embeddedImages[compareIndex].name)) != 0)
	{
		if (result > 0)	{ // We need to search a HIGHER index
			if (compareIndex != lowerLimit) {
				lowerLimit = compareIndex;
			} else {
				return -1;
			}
			compareIndex = (upperLimit + lowerLimit + 1) >> 1;	// Midpoint, round up
		}
		else { // We Need to search a LOWER index
			if (compareIndex != upperLimit) {
				upperLimit = compareIndex;
			} else {
				return -1;
			}
			compareIndex = (upperLimit + lowerLimit) >> 1;	// Midpoint, round down
		}
	}
	return compareIndex;
}
#endif

static int loadThemeImage(const char *image, int alt_image)
{
	char		dirspec[256];
	int 		i;
#ifdef CONFIG_EMBED_THEME
	int 		e;
#endif
	uint16_t	width;
	uint16_t	height;
	uint8_t		*imagedata;

	if ((strlen(image) + strlen(theme_name) + 20) > sizeof(dirspec)) {
		return 1;
	}
	if ((i = getImageIndexByName(image)) < 0) {
		return 1;
	}
	if (!images[i].image && !(images[i].image = malloc(sizeof(pixmap_t)))) {
		return 1;
	}
	sprintf(dirspec, "/Extra/Themes/%s/%s.png", theme_name, image);
	width = 0;
	height = 0;
	imagedata = NULL;
	if ((loadPngImage(dirspec, &width, &height, &imagedata)) == 0) {
		images[i].image->width = width;
		images[i].image->height = height;
		images[i].image->pixels = (pixel_t *)imagedata;
		flipRB(images[i].image);
		return 0;
	}
#ifdef CONFIG_EMBED_THEME
	else if ((e = getEmbeddedImageIndexByName(image)) >= 0) {
		unsigned char *embed_data;
		unsigned int embed_size;
		embed_data = embeddedImages[e].pngdata;
		embed_size = *embeddedImages[e].length;

		if (loadEmbeddedPngImage(embed_data, embed_size, &width, &height, &imagedata) == 0) {
			images[i].image->width = width;
			images[i].image->height = height;
			images[i].image->pixels = (pixel_t *)imagedata;
			flipRB(images[i].image);
			return 0;
            }
        }
#endif
        else if (alt_image != IMG_REQUIRED && is_image_loaded(alt_image)) {
            // Using the passed alternate image for non-mandatory images.
            // We don't clone the already existing pixmap, but using its properties instead!
            images[i].image->width = images[alt_image].image->width;
            images[i].image->height = images[alt_image].image->height;
            images[i].image->pixels = images[alt_image].image->pixels;
            return 0;
        }

	// If we got here it's an error
#ifndef CONFIG_EMBED_THEME
	printf("ERROR: GUI: could not open '%s/%s.png'!\n", theme_name, image);
	sleep(2);
#endif
	free(images[i].image);
	images[i].image = NULL;
	return 1;
}

static int loadGraphics(void)
{
	LOADPNG(background,                     IMG_REQUIRED);
	LOADPNG(logo,                           IMG_REQUIRED);

	LOADPNG(device_generic,                 IMG_REQUIRED);
	LOADPNG(device_generic_o,               iDeviceGeneric);
	LOADPNG(device_hfsplus,                 iDeviceGeneric);
	LOADPNG(device_hfsplus_o,               iDeviceHFS);
	LOADPNG(device_hfsplus_mav,             iDeviceHFS);
	LOADPNG(device_hfsplus_mav_o,           iDeviceHFS_mav);
	LOADPNG(device_hfsplus_ml,              iDeviceHFS);
	LOADPNG(device_hfsplus_ml_o,            iDeviceHFS_ML);
	LOADPNG(device_hfsplus_lion,            iDeviceHFS);
	LOADPNG(device_hfsplus_lion_o,          iDeviceHFS_Lion);
	LOADPNG(device_hfsplus_sl,              iDeviceHFS);
	LOADPNG(device_hfsplus_sl_o,            iDeviceHFS_SL);
	LOADPNG(device_hfsplus_leo,             iDeviceHFS);
	LOADPNG(device_hfsplus_leo_o,           iDeviceHFS_Leo);
	LOADPNG(device_hfsplus_tiger,           iDeviceHFS);
	LOADPNG(device_hfsplus_tiger_o,         iDeviceHFS_Tiger);

	LOADPNG(device_hfsraid,                 iDeviceHFS);
	LOADPNG(device_hfsraid_o,               iDeviceHFSRAID);
	LOADPNG(device_hfsraid_mav,             iDeviceHFSRAID);
	LOADPNG(device_hfsraid_mav_o,           iDeviceHFSRAID_mav);
	LOADPNG(device_hfsraid_ml,              iDeviceHFSRAID);
	LOADPNG(device_hfsraid_ml_o,            iDeviceHFSRAID_ML);
	LOADPNG(device_hfsraid_lion,            iDeviceHFSRAID);
	LOADPNG(device_hfsraid_lion_o,          iDeviceHFSRAID_Lion);
	LOADPNG(device_hfsraid_sl,              iDeviceHFSRAID);
	LOADPNG(device_hfsraid_sl_o,            iDeviceHFSRAID_SL);
	LOADPNG(device_hfsraid_leo,             iDeviceHFSRAID);
	LOADPNG(device_hfsraid_leo_o,           iDeviceHFSRAID_Leo);
	LOADPNG(device_hfsraid_tiger,           iDeviceHFSRAID);
	LOADPNG(device_hfsraid_tiger_o,         iDeviceHFSRAID_Tiger);
	LOADPNG(device_ext3,                    iDeviceGeneric);
	LOADPNG(device_ext3_o,                  iDeviceEXT3);
	LOADPNG(device_freebsd,                 iDeviceGeneric);        /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	LOADPNG(device_freebsd_o,               iDeviceFreeBSD);        /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	LOADPNG(device_openbsd,                 iDeviceGeneric);        /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	LOADPNG(device_openbsd_o,               iDeviceOpenBSD);        /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
	LOADPNG(device_befs,                    iDeviceGeneric);        /* Haiku detection and Icon credits to scorpius  */
	LOADPNG(device_befs_o,                  iDeviceBEFS);           /* Haiku detection and Icon credits to scorpius  */
	LOADPNG(device_fat,                     iDeviceGeneric);
	LOADPNG(device_fat_o,                   iDeviceFAT);
	LOADPNG(device_fat16,                   iDeviceFAT);
	LOADPNG(device_fat16_o,                 iDeviceFAT16);
	LOADPNG(device_fat32,                   iDeviceFAT);
	LOADPNG(device_fat32_o,                 iDeviceFAT32);
	LOADPNG(device_ntfs,                    iDeviceGeneric);
	LOADPNG(device_ntfs_o,                  iDeviceNTFS);
	LOADPNG(device_cdrom,                   iDeviceGeneric);
	LOADPNG(device_cdrom_o,                 iDeviceCDROM);

	LOADPNG(device_selection,               IMG_REQUIRED);
	LOADPNG(device_scroll_prev,             IMG_REQUIRED);
	LOADPNG(device_scroll_next,             IMG_REQUIRED);

	LOADPNG(menu_boot,                      IMG_REQUIRED);
	LOADPNG(menu_verbose,                   IMG_REQUIRED);
	LOADPNG(menu_ignore_caches,             IMG_REQUIRED);
	LOADPNG(menu_single_user,               IMG_REQUIRED);
	LOADPNG(menu_memory_info,               IMG_REQUIRED);
	LOADPNG(menu_video_info,                IMG_REQUIRED);
	LOADPNG(menu_help,                      IMG_REQUIRED);
	LOADPNG(menu_verbose_disabled,          IMG_REQUIRED);
	LOADPNG(menu_ignore_caches_disabled,    IMG_REQUIRED);
	LOADPNG(menu_single_user_disabled,      IMG_REQUIRED);
	LOADPNG(menu_selection,                 IMG_REQUIRED);

	LOADPNG(progress_bar,                   IMG_REQUIRED);
	LOADPNG(progress_bar_background,        IMG_REQUIRED);

	LOADPNG(text_scroll_prev,               IMG_REQUIRED);
	LOADPNG(text_scroll_next,               IMG_REQUIRED);

	LOADPNG(font_console,                   IMG_REQUIRED);
	LOADPNG(font_small,                     IMG_REQUIRED);

	initFont( &font_console, &images[iFontConsole]);
	initFont( &font_small, &images[iFontSmall]);

	return 0;
}

static int unloadGraphics(void)
{
	int i;

	destroyFont(&font_console);
	destroyFont(&font_small);
	for (i = 0; i < sizeof(images) / sizeof(images[0]); i++) {
		if (images[i].image) {
			if (images[i].image->pixels) {
				free(images[i].image->pixels);
			}
			free (images[i].image);
			images[i].image = 0;
	    }
	}
	return 0;
}

int freeBackBuffer( window_t *window )
{
	if (gui.backbuffer && gui.backbuffer->pixels) {
		free(gui.backbuffer->pixels);
		free(gui.backbuffer);
		gui.backbuffer = 0;
		return 0;
	}

	return 1;
}

pixmap_t *getCroppedPixmapAtPosition( pixmap_t *from, position_t pos, uint16_t width, uint16_t height )
{
	pixmap_t *cropped = malloc( sizeof( pixmap_t ) );
	if( !cropped ) {
		return 0;
	}
	cropped->pixels = malloc( width * height * 4 );
	if ( !cropped->pixels ) {
		return 0;
	}
	cropped->width = width;
	cropped->height = height;
	
	int destx = 0, desty = 0;
	int srcx = pos.x, srcy = pos.y;
	
	for( ; desty < height; desty++, srcy++) {
		for( destx = 0, srcx = pos.x; destx < width; destx++, srcx++ ) {
			pixel( cropped, destx, desty ).value = pixel( from, srcx, srcy ).value;
		}
	}
	return cropped;
}

int createBackBuffer( window_t *window )
{
	gui.backbuffer = malloc(sizeof(pixmap_t));
	if(!gui.backbuffer) {
		return 1;
	}
	gui.backbuffer->pixels = malloc( window->width * window->height * 4 );
	if(!gui.backbuffer->pixels) {
		free(gui.backbuffer);
		gui.backbuffer = 0;
		return 1;
	}
	
	gui.backbuffer->width = gui.screen.width;
	gui.backbuffer->height = gui.screen.height;

	return 0;
}

int createWindowBuffer( window_t *window )
{
	window->pixmap = malloc(sizeof(pixmap_t));
	if(!window->pixmap) {
		return 1;
	}

	window->pixmap->pixels = malloc( window->width * window->height * 4 );
	if(!window->pixmap->pixels) {
		free(window->pixmap);
		window->pixmap = 0;
		return 1;
	}
	
	window->pixmap->width = window->width;
	window->pixmap->height = window->height;

	return 0;
}

int freeWindowBuffer( window_t *window )
{
	if (window->pixmap && window->pixmap->pixels) {
		free(window->pixmap->pixels);
		free(window->pixmap);
		return 0;
	}
	return 1;
}

void fillPixmapWithColor(pixmap_t *pm, uint32_t color)
{
	int x,y;
	
	// fill with given color AARRGGBB
	for( x=0; x < pm->width; x++ ) {
		for( y=0; y< pm->height; y++) {
			pixel(pm,x,y).value = color;
		}
	}
}

void drawBackground()
{
	// reset text cursor
	gui.screen.cursor.x = gui.screen.hborder;
	gui.screen.cursor.y = gui.screen.vborder;
	
	fillPixmapWithColor( gui.screen.pixmap, gui.screen.bgcolor);
	
	// draw background.png into background buffer
	blend( images[iBackground].image, gui.screen.pixmap, gui.background.pos );
	
	// draw logo.png into background buffer
	if (gui.logo.draw)
	{
		blend( images[iLogo].image, gui.screen.pixmap, gui.logo.pos);
	}
	
	memcpy( gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4 );
}

void setupDeviceList(config_file_t *theme)
{
	unsigned int pixel;
	int	alpha;			// transparency level 0 (obligue) - 255 (transparent)
	uint32_t color;			// color value formatted RRGGBB
	int val, len;
	const char *string;	

	if(getIntForKey("devices_max_visible", &val, theme )) {
		gui.maxdevices = MIN( val, gDeviceCount );
	}

	if(getIntForKey("devices_iconspacing", &val, theme )) {
		gui.devicelist.iconspacing = val;
	}

	// check layout for horizontal or vertical
	gui.layout = HorizontalLayout;
	if(getValueForKey( "devices_layout", &string, &len, theme)) {
		if (!strcmp (string, "vertical")) {
			gui.layout = VerticalLayout;
		}
	}

	switch (gui.layout) {
	case VerticalLayout:
		gui.devicelist.height = ((images[iSelection].image->height + font_console.chars[0]->height + gui.devicelist.iconspacing) * MIN(gui.maxdevices, gDeviceCount) + (images[iDeviceScrollPrev].image->height + images[iDeviceScrollNext].image->height) + gui.devicelist.iconspacing);
		gui.devicelist.width  = (images[iSelection].image->width + gui.devicelist.iconspacing);

		if(getDimensionForKey("devices_pos_x", &pixel, theme, gui.screen.width , images[iSelection].image->width ) ) {
			gui.devicelist.pos.x = pixel;
		}

		if(getDimensionForKey("devices_pos_y", &pixel, theme, gui.screen.height , gui.devicelist.height ) ) {
			gui.devicelist.pos.y = pixel;
		}
		break;

	case HorizontalLayout:
	default:
		gui.devicelist.width = ((images[iSelection].image->width + gui.devicelist.iconspacing) * MIN(gui.maxdevices, gDeviceCount) + (images[iDeviceScrollPrev].image->width + images[iDeviceScrollNext].image->width) + gui.devicelist.iconspacing);
		gui.devicelist.height = (images[iSelection].image->height + font_console.chars[0]->height + gui.devicelist.iconspacing);

		if(getDimensionForKey("devices_pos_x", &pixel, theme, gui.screen.width , gui.devicelist.width ) ) {
			gui.devicelist.pos.x = pixel;
		} else {
			gui.devicelist.pos.x = ( gui.screen.width - gui.devicelist.width ) / 2;
		}

		if(getDimensionForKey("devices_pos_y", &pixel, theme, gui.screen.height , images[iSelection].image->height ) )
		{
			gui.devicelist.pos.y = pixel;
		} else {
			gui.devicelist.pos.y = ( gui.screen.height - gui.devicelist.height ) / 2;
		}
		break;
	}

	if(getColorForKey("devices_bgcolor", &color, theme)) {
		gui.devicelist.bgcolor = (color & 0x00FFFFFF);
	}

	if(getIntForKey("devices_transparency", &alpha, theme)) {
		gui.devicelist.bgcolor = gui.devicelist.bgcolor | (( 255 - ( alpha & 0xFF) ) << 24);
	}

	if (gui.devicelist.pixmap) {
		freeWindowBuffer(&gui.devicelist);
		createWindowBuffer(&gui.devicelist);
	}
}

void loadThemeValues(config_file_t *theme)
{
	unsigned int screen_width  = gui.screen.width;
	unsigned int screen_height = gui.screen.height;
	unsigned int pixel;
	int	alpha;			// transparency level 0 (obligue) - 255 (transparent)
	uint32_t color;			// color value formatted RRGGBB
	int val;

	/*
	 * Parse screen parameters
	 */
	if(getColorForKey("screen_bgcolor", &color, theme )) {
		gui.screen.bgcolor = (color & 0x00FFFFFF);
	}
	if(getIntForKey("screen_textmargin_h", &val, theme)) {
		gui.screen.hborder = MIN( gui.screen.width , val );
	}
	if(getIntForKey("screen_textmargin_v", &val, theme)) {
		gui.screen.vborder = MIN( gui.screen.height , val );
	}

	/*
	 * Parse background parameters
	 */
	if(getDimensionForKey("background_pos_x", &pixel, theme, screen_width , images[iBackground].image->width ) )
		gui.background.pos.x = pixel;

	if(getDimensionForKey("background_pos_y", &pixel, theme, screen_height , images[iBackground].image->height ) ) {
		gui.background.pos.y = pixel;
	}

	/*
	 * Parse logo parameters
	 */
	if(getDimensionForKey("logo_pos_x", &pixel, theme, screen_width , images[iLogo].image->width ) ) {
		gui.logo.pos.x = pixel;
	}

	if(getDimensionForKey("logo_pos_y", &pixel, theme, screen_height , images[iLogo].image->height ) )
	{
		gui.logo.pos.y = pixel;
	}

	/*
	 * Parse progress bar parameters
	 */
	if(getDimensionForKey("progressbar_pos_x", &pixel, theme, screen_width , 0 ) )
	{
		gui.progressbar.pos.x = pixel;
	}

	if(getDimensionForKey("progressbar_pos_y", &pixel, theme, screen_height , 0 ) )
	{
		gui.progressbar.pos.y = pixel;
	}

	/*
	 * Parse countdown text parameters
	 */
	if(getDimensionForKey("countdown_pos_x", &pixel, theme, screen_width , 0 ) )
	{
		gui.countdown.pos.x = pixel;
	}

	if(getDimensionForKey("countdown_pos_y", &pixel, theme, screen_height , 0 ) )
	{
		gui.countdown.pos.y = pixel;
	}

    /*
	 * Parse devicelist parameters
	 */
	setupDeviceList(theme);

	/*
	 * Parse infobox parameters
	 */
	if(getIntForKey("infobox_width", &val, theme))
	{
		gui.infobox.width = MIN( screen_width , val );
	}
	if(getIntForKey("infobox_height", &val, theme))
	{
		gui.infobox.height = MIN( screen_height , val );
	}
	if(getDimensionForKey("infobox_pos_x", &pixel, theme, screen_width , gui.infobox.width ) )
	{
		gui.infobox.pos.x = pixel;
	}
	if(getDimensionForKey("infobox_pos_y", &pixel, theme, screen_height , gui.infobox.height ) )
	{
		gui.infobox.pos.y = pixel;
	}
	if(getIntForKey("infobox_textmargin_h", &val, theme))
	{
		gui.infobox.hborder = MIN( gui.infobox.width , val );
	}
	if(getIntForKey("infobox_textmargin_v", &val, theme))
	{
		gui.infobox.vborder = MIN( gui.infobox.height , val );
	}
	if(getColorForKey("infobox_bgcolor", &color, theme))
	{
		gui.infobox.bgcolor = (color & 0x00FFFFFF);
	}
	if(getIntForKey("infobox_transparency", &alpha, theme))
	{
		gui.infobox.bgcolor = gui.infobox.bgcolor | (( 255 - ( alpha & 0xFF) ) << 24);
	}
	/*
	 * Parse menu parameters
	 */
	if(getDimensionForKey("menu_width", &pixel, theme, gui.screen.width , 0 ) )
	{
		gui.menu.width = pixel;
	}
	else
	{
		gui.menu.width = images[iMenuSelection].image->width;
	}
	if(getDimensionForKey("menu_height", &pixel, theme, gui.screen.height , 0 ) )
	{
		gui.menu.height = pixel;
	}
	else
	{
		gui.menu.height = (infoMenuItemsCount) * images[iMenuSelection].image->height;
	}
	if(getDimensionForKey("menu_pos_x", &pixel, theme, screen_width , gui.menu.width ) )
	{
		gui.menu.pos.x = pixel;
	}
	if(getDimensionForKey("menu_pos_y", &pixel, theme, screen_height , gui.menu.height ) )
	{
		gui.menu.pos.y = pixel;
	}
	if(getIntForKey("menu_textmargin_h", &val, theme))
	{
		gui.menu.hborder = MIN( gui.menu.width , val );
	}
	if(getIntForKey("menu_textmargin_v", &val, theme))
	{
		gui.menu.vborder = MIN( gui.menu.height , val );
	}
	if(getColorForKey("menu_bgcolor", &color, theme))
	{
		gui.menu.bgcolor = (color & 0x00FFFFFF);
	}
	if(getIntForKey("menu_transparency", &alpha, theme))
	{
		gui.menu.bgcolor = gui.menu.bgcolor | (( 255 - ( alpha & 0xFF) ) << 24);		
	}

	/*
	 * Parse bootprompt parameters
	 */
	if(getDimensionForKey("bootprompt_width", &pixel, theme, screen_width , 0 ) )
	{
		gui.bootprompt.width = pixel;
	}
	if(getIntForKey("bootprompt_height", &val, theme))
	{
		gui.bootprompt.height = MIN( screen_height , val );
	}
	if(getDimensionForKey("bootprompt_pos_x", &pixel, theme, screen_width , gui.bootprompt.width ) )
	{
		gui.bootprompt.pos.x = pixel;
	}
	if(getDimensionForKey("bootprompt_pos_y", &pixel, theme, screen_height , gui.bootprompt.height ) )
	{
		gui.bootprompt.pos.y = pixel;
	}
	if(getIntForKey("bootprompt_textmargin_h", &val, theme))
	{
		gui.bootprompt.hborder = MIN( gui.bootprompt.width , val );
	}
	if(getIntForKey("bootprompt_textmargin_v", &val, theme))
	{
		gui.bootprompt.vborder = MIN( gui.bootprompt.height , val );
	}
	if(getColorForKey("bootprompt_bgcolor", &color, theme))
	{
		gui.bootprompt.bgcolor = (color & 0x00FFFFFF);
	}
	if(getIntForKey("bootprompt_transparency", &alpha, theme))
	{
		gui.bootprompt.bgcolor = gui.bootprompt.bgcolor | (( 255 - ( alpha & 0xFF) ) << 24);
	}
	if(getColorForKey("font_small_color", &color, theme))
	{
		gui.screen.font_small_color = (color & 0x00FFFFFF);
	}
	if(getColorForKey("font_console_color", &color, theme))
	{
		gui.screen.font_console_color = (color & 0x00FFFFFF);
	}
}

int initGUI(void)
{
	int		val;
	int	len;
	char	dirspec[256];

	getValueForKey( "Theme", &theme_name, &len, &bootInfo->chameleonConfig );
	if ((strlen(theme_name) + 27) > sizeof(dirspec)) {
		return 1;
	}
	sprintf(dirspec, "/Extra/Themes/%s/theme.plist", theme_name);
	if (loadConfigFile(dirspec, &bootInfo->themeConfig) != 0) {
#ifdef CONFIG_EMBED_THEME
	config_file_t	*config;
    
	config = &bootInfo->themeConfig;
	if (ParseXMLFile((char *)__theme_plist, &config->dictionary) != 0) {
	return 1;
	}
#else
		return 1;
#endif
	}
	// parse display size parameters
	if (getIntForKey("screen_width", &val, &bootInfo->themeConfig) && val > 0) {
		screen_params[0] = val;
	}
	if (getIntForKey("screen_height", &val, &bootInfo->themeConfig) && val > 0) {
		screen_params[1] = val;
	}

	// Initalizing GUI strucutre.
	bzero(&gui, sizeof(gui_t));
	
	// find best matching vesa mode for our requested width & height
	getGraphicModeParams(screen_params);

	// set our screen structure with the mode width & height
	gui.screen.width = screen_params[0];	
	gui.screen.height = screen_params[1];

	// load graphics otherwise fail and return
	if (loadGraphics() == 0) {
		loadThemeValues(&bootInfo->themeConfig);
		colorFont(&font_small, gui.screen.font_small_color);
		colorFont(&font_console, gui.screen.font_console_color);

		// create the screen & window buffers
		if (createBackBuffer(&gui.screen) == 0) {
			if (createWindowBuffer(&gui.screen) == 0) {
				if (createWindowBuffer(&gui.devicelist) == 0) {
					if (createWindowBuffer(&gui.bootprompt) == 0) {
						if (createWindowBuffer(&gui.infobox) == 0) {
							if (createWindowBuffer(&gui.menu) == 0) {
								gui.logo.draw = true;
								drawBackground();
								// lets copy the screen into the back buffer
								memcpy( gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4 );
								setVideoMode( GRAPHICS_MODE, 0 );
								gui.initialised = true;
								return 0;
							}
						}
					}
				}
			}
		}
	}

    // not available memory, freeing resources
    freeWindowBuffer(&gui.menu);
    freeWindowBuffer(&gui.infobox);
    freeWindowBuffer(&gui.bootprompt);
    freeWindowBuffer(&gui.devicelist);
    freeWindowBuffer(&gui.screen);
    freeBackBuffer(&gui.screen);
    unloadGraphics();

	return 1;
}

bool is_image_loaded(int i)
{	
	return (images[i].image != NULL) ? true : false;
}

void drawDeviceIcon(BVRef device, pixmap_t *buffer, position_t p, bool isSelected)
{
	int devicetype;

	if( diskIsCDROM(device) ) {
		devicetype = iDeviceCDROM;				// Use CDROM icon
	} else {
		switch (device->part_type)
		{
			case kPartitionTypeHFS:
			{
				// Use HFS or HFSRAID icon depending on bvr flags. Fallbacks are handled by alt_image above.
				switch (device->OSVersion[3]) {
					case '9':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_mav : iDeviceHFS_mav);
						break;
					case '8':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_ML : iDeviceHFS_ML);
						break;
					case '7':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_Lion : iDeviceHFS_Lion);
						break;
					case '6':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_SL : iDeviceHFS_SL);
						break;
					case '5':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_Leo : iDeviceHFS_Leo);
						break;
					case '4':
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID_Tiger : iDeviceHFS_Tiger);
						break;
					default:
						devicetype = (device->flags & kBVFlagBooter ? iDeviceHFSRAID : iDeviceHFS);
						break;
				}
				
				break;
				
			}
			case kPartitionTypeHPFS:
				devicetype = iDeviceNTFS;		// Use HPFS / NTFS icon
				break;

			case kPartitionTypeFAT16:
				devicetype = iDeviceFAT16;		// Use FAT16 icon
				break;

			case kPartitionTypeFAT32:
				devicetype = iDeviceFAT32;		// Use FAT32 icon
				break;

			case kPartitionTypeEXT3:
				devicetype = iDeviceEXT3;		// Use EXT2/3 icon
				break;

			case kPartitionTypeFreeBSD:                     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
				devicetype = iDeviceFreeBSD;		// Use FreeBSD icon
				break;

			case kPartitionTypeOpenBSD:                     /* FreeBSD/OpenBSD detection,nawcom's code by valv, Icon credits to blackosx  */
				devicetype = iDeviceOpenBSD;		// Use OpenBSD icon
				break;

			case kPartitionTypeBEFS:                        /* Haiku detection and Icon credits to scorpius  */
				devicetype = iDeviceBEFS;		// Use BEFS / Haiku icon
				break;

			default:
				devicetype = iDeviceGeneric;		// Use Generic icon
				break;
		}
	}
	
	// Draw the selection image and use the next (device_*_o) image for the selected item.
	if (isSelected)
	{
		blend(images[iSelection].image, buffer, centeredAt(images[iSelection].image, p));
		devicetype++; // select override image 
	}

	// draw icon
	blend(images[devicetype].image, buffer, centeredAt(images[devicetype].image, p));
	
	p.y += (images[iSelection].image->height / 2) + font_console.chars[0]->height;
	
	// draw volume label 
	drawStrCenteredAt( device->label, &font_small, buffer, p);	

}

void drawDeviceList (int start, int end, int selection)
{
	int			i;
	bool		shoWinfo = false;
	extern bool showBootBanner;
	position_t	p, p_prev, p_next;

	//uint8_t	maxDevices = MIN( gui.maxdevices, menucount );

	fillPixmapWithColor( gui.devicelist.pixmap, gui.devicelist.bgcolor);

	makeRoundedCorners( gui.devicelist.pixmap);

	switch (gui.layout) {

		case VerticalLayout:
			p.x = (gui.devicelist.width /2);
			p.y =  ( ( images[iSelection].image->height / 2 ) + images[iDeviceScrollPrev].image->height + gui.devicelist.iconspacing );

			// place scroll indicators at top & bottom edges
			p_prev = pos ( gui.devicelist.width / 2 , gui.devicelist.iconspacing );
			p_next = pos ( p_prev.x, gui.devicelist.height - gui.devicelist.iconspacing );
			
			break;
			
		default:	// use Horizontal layout as the default

		case HorizontalLayout:
			p.x = (gui.devicelist.width - ( gui.devicelist.width / gui.maxdevices ) * gui.maxdevices ) / 2 + ( images[iSelection].image->width / 2) + images[iDeviceScrollPrev].image->width + gui.devicelist.iconspacing;
			p.y = ((gui.devicelist.height - font_console.chars[0]->height ) - images[iSelection].image->height) / 2 + ( images[iSelection].image->height / 2 );

			// place scroll indicators at left & right edges
			p_prev = pos ( images[iDeviceScrollPrev].image->width / 2  + gui.devicelist.iconspacing / 2, gui.devicelist.height / 2 );
			p_next = pos ( gui.devicelist.width - ( images[iDeviceScrollNext].image->width / 2 + gui.devicelist.iconspacing / 2), gui.devicelist.height / 2 );
			
			break;
			
	}
	
	// draw visible device icons
	for (i = 0; i < gui.maxdevices; i++) {
		BVRef param = menuItems[start + i].param;

		bool isSelected = ((start + i) == selection) ? true : false;
		if (isSelected) {
			if (param->flags & kBVFlagNativeBoot) {
				infoMenuNativeBoot = true;
			} else {
				infoMenuNativeBoot = false;
				if(infoMenuSelection >= INFOMENU_NATIVEBOOT_START && infoMenuSelection <= INFOMENU_NATIVEBOOT_END) {
					infoMenuSelection = 0;
				}
			}

			if (gui.menu.draw) {
				drawInfoMenuItems();
			}
			getBoolForKey(kShowInfoKey, &shoWinfo, &bootInfo->chameleonConfig);
			
			if (shoWinfo && showBootBanner) {
				gui.debug.cursor = pos( 10, 100);
				dprintf( &gui.screen, "label:     %s\n",   param->label );
				dprintf( &gui.screen, "biosdev:   0x%x\n", param->biosdev );
				dprintf( &gui.screen, "type:      0x%x\n", param->type );
				dprintf( &gui.screen, "flags:     0x%x\n", param->flags );
				dprintf( &gui.screen, "part_no:   %d\n",   param->part_no );
				dprintf( &gui.screen, "part_boff: 0x%x\n", param->part_boff );
				dprintf( &gui.screen, "part_type: 0x%x\n", param->part_type );
				dprintf( &gui.screen, "bps:       0x%x\n", param->bps );
				dprintf( &gui.screen, "name:      %s\n",   param->name );
				dprintf( &gui.screen, "type_name: %s\n",   param->type_name );
				dprintf( &gui.screen, "modtime:   %d\n",   param->modTime );
//				// res
				dprintf( &gui.screen, "width:     %d\n",   gui.screen.width );
				dprintf( &gui.screen, "height:    %d\n",   gui.screen.height );
//				dprintf( &gui.screen, "attr:      0x%x\n", gui.screen.attr ); //Azi: reminder
//				dprintf( &gui.screen, "mm:        %d\n",   gui.screen.mm );
			}
		}
		
		drawDeviceIcon( param, gui.devicelist.pixmap, p, isSelected);
		
		if (gui.layout == HorizontalLayout) {
			p.x += images[iSelection].image->width + gui.devicelist.iconspacing; 
		}
		if (gui.layout == VerticalLayout) {
			p.y += ( images[iSelection].image->height + font_console.chars[0]->height + gui.devicelist.iconspacing ); 
		}
	}

	// draw prev indicator
	if (start) {
		blend( images[iDeviceScrollPrev].image, gui.devicelist.pixmap, centeredAt( images[iDeviceScrollPrev].image, p_prev ) );
	}
	// draw next indicator
	if ( end < gDeviceCount - 1 ) {
		blend( images[iDeviceScrollNext].image, gui.devicelist.pixmap, centeredAt( images[iDeviceScrollNext].image, p_next ) );
	}
	gui.redraw = true;
	
	updateVRAM();
	
}

void clearGraphicBootPrompt()
{
	// clear text buffer
	//prompt[0] = '\0';
	//prompt_pos=0;

	
	if( gui.bootprompt.draw == true ) {
		gui.bootprompt.draw = false;
		gui.redraw = true;
		// this causes extra frames to be drawn
		//updateVRAM();
	}

	return;
}

void updateGraphicBootPrompt()
{
	fillPixmapWithColor( gui.bootprompt.pixmap, gui.bootprompt.bgcolor);

	makeRoundedCorners( gui.bootprompt.pixmap);

	position_t p_text = pos( gui.bootprompt.hborder , ( ( gui.bootprompt.height -  font_console.chars[0]->height) ) / 2 );

	// print the boot prompt text
	drawStr(prompt_text, &font_console, gui.bootprompt.pixmap, p_text);
	
	// get the position of the end of the boot prompt text to display user input
	position_t p_prompt = pos( p_text.x + ( ( strlen(prompt_text) ) * font_console.chars[0]->width ), p_text.y );
	
	drawStr( gBootArgs, &font_console, gui.bootprompt.pixmap, p_prompt);

	gui.menu.draw = false;
	gui.bootprompt.draw = true;
	gui.redraw = true;
	
	updateVRAM();
	
	return;
}

static inline
void vramwrite (void *data, int width, int height)
{
	if (VIDEO (depth) == 32 && VIDEO (rowBytes) == gui.backbuffer->width * 4) {
		memcpy((uint8_t *)vram, gui.backbuffer->pixels, VIDEO (rowBytes)*VIDEO (height));
	} else {
		uint32_t r, g, b;
		int i, j;
		for (i = 0; i < VIDEO (height); i++) {
			for (j = 0; j < VIDEO (width); j++) {
				b = ((uint8_t *) data)[4*i*width + 4*j];
				g = ((uint8_t *) data)[4*i*width + 4*j + 1];
				r = ((uint8_t *) data)[4*i*width + 4*j + 2];
				switch (VIDEO (depth)) {
					case 32:
						*(uint32_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*4) = (b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16);
						break;
					case 24:
						*(uint32_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*3) = ((*(uint32_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*3))&0xff000000)
						| (b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16);
						break;
					case 16:
						// Somehow 16-bit is always 15-bits really
						//						*(uint16_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*2) = ((b&0xf8)>>3) | ((g&0xfc)<<3) | ((r&0xf8)<<8);
						//						break;							
					case 15:
						*(uint16_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*2) = ((b&0xf8)>>3) | ((g&0xf8)<<2) | ((r&0xf8)<<7);
						break;	
				}
			}
		}
	}
}

void updateVRAM()
{
	if (gui.redraw) {
		if (gui.devicelist.draw) {
			blend( gui.devicelist.pixmap, gui.backbuffer, gui.devicelist.pos );
		}
		if (gui.bootprompt.draw) {
			blend( gui.bootprompt.pixmap, gui.backbuffer, gui.bootprompt.pos );
		}
		if (gui.menu.draw) {
			blend( gui.menu.pixmap, gui.backbuffer, gui.menu.pos );
		}
		if (gui.infobox.draw) {
			blend( gui.infobox.pixmap, gui.backbuffer, gui.infobox.pos );
		}
	}

	vramwrite ( gui.backbuffer->pixels, gui.backbuffer->width, gui.backbuffer->height );

	if (gui.redraw) {
		memcpy( gui.backbuffer->pixels, gui.screen.pixmap->pixels, gui.backbuffer->width * gui.backbuffer->height * 4 );
		gui.redraw = false;
	}
}

struct putc_info //Azi: exists on console.c & printf.c
{
	char * str;
	char * last_str;
};

static int
sputc(int c, struct putc_info * pi) //Azi: same as above
{
	if (pi->last_str) {
		if (pi->str == pi->last_str) {
			*(pi->str) = '\0';
			return 0;
		}
	}
	*(pi->str)++ = c;
	return c;
}

int gprintf( window_t * window, const char * fmt, ...)
{
	char *formattedtext;

	va_list ap;
	
	struct putc_info pi;

	if ((formattedtext = malloc(1024)) != NULL) {
		// format the text
		va_start(ap, fmt);
		pi.str = formattedtext;
		pi.last_str = 0;
		prf(fmt, ap, sputc, &pi);
		*pi.str = '\0';
		va_end(ap);
		
		position_t	origin, cursor, bounds;

		int i;
		int character;

		origin.x = MAX( window->cursor.x, window->hborder );
		origin.y = MAX( window->cursor.y, window->vborder );
		
		bounds.x = ( window->width - window->hborder );
		bounds.y = ( window->height - window->vborder );
		
		cursor = origin;

		font_t *font = &font_console;

		for( i=0; i< strlen(formattedtext); i++ ) {
			character = formattedtext[i];
			
			character -= 32;

			// newline ?
			if( formattedtext[i] == '\n' ) {
				cursor.x = window->hborder;
				cursor.y += font->height;

				if ( cursor.y > bounds.y ) {
					cursor.y = origin.y;
				}
				continue;
			}

			// tab ?
			if( formattedtext[i] == '\t' ) {
				cursor.x += ( font->chars[0]->width * 5 );
			}

			// draw the character
			if( font->chars[character]) {
				blend(font->chars[character], window->pixmap, cursor);
			}

			cursor.x += font->chars[character]->width;

			// check x pos and do newline
			if ( cursor.x > bounds.x ) {
				cursor.x = origin.x;
				cursor.y += font->height;
			}
			
			// check y pos and reset to origin.y
			if ( cursor.y > bounds.y ) {
				cursor.y = origin.y;
			}
		}

		// update cursor postition
		window->cursor = cursor;
		
		free(formattedtext);
		
		return 0;
	}
	return 1;
}

int dprintf( window_t * window, const char * fmt, ...)
{
	char *formattedtext;
	
	va_list ap;
	
	//window = &gui.debug;
	
	struct putc_info pi;
	
	if ((formattedtext = malloc(1024)) != NULL) {
		// format the text
		va_start(ap, fmt);
		pi.str = formattedtext;
		pi.last_str = 0;
		prf(fmt, ap, sputc, &pi);
		*pi.str = '\0';
		va_end(ap);
		
		position_t	origin, cursor, bounds;
		
		int i;
		int character;
		
		origin.x = MAX( gui.debug.cursor.x, window->hborder );
		origin.y = MAX( gui.debug.cursor.y, window->vborder );
		
		bounds.x = ( window->width - window->hborder );
		bounds.y = ( window->height - window->vborder );
		
		cursor = origin;
		
		font_t *font = &font_console;
		
		for( i=0; i< strlen(formattedtext); i++ ) {
			character = formattedtext[i];
			
			character -= 32;
			
			// newline ?
			if( formattedtext[i] == '\n' ) {
				cursor.x = window->hborder;
				cursor.y += font->height;
				
				if ( cursor.y > bounds.y ) {
					cursor.y = origin.y;
				}
				continue;
			}
			
			// tab ?
			if( formattedtext[i] == '\t' ) {
				cursor.x += ( font->chars[0]->width * 5 );
			}
			// draw the character
			if( font->chars[character]) {
				blend(font->chars[character], gui.backbuffer, cursor);
			}
			cursor.x += font->chars[character]->width;
			
			// check x pos and do newline
			if ( cursor.x > bounds.x ) {
				cursor.x = origin.x;
				cursor.y += font->height;
			}
			
			// check y pos and reset to origin.y
			if ( cursor.y > bounds.y ) {
				cursor.y = origin.y;
			}
		}
		
		// update cursor postition
		gui.debug.cursor = cursor;
		
		free(formattedtext);
		
		return 0;
		
	}
	return 1;
}

int vprf(const char * fmt, va_list ap)
{
	int i;
	int character;

	char *formattedtext;
	window_t *window = &gui.screen;
	struct putc_info pi;
	
	position_t	origin, cursor, bounds;
	font_t *font = &font_console;
	
	if ((formattedtext = malloc(1024)) != NULL) {
		// format the text
		pi.str = formattedtext;
		pi.last_str = 0;
		prf(fmt, ap, sputc, &pi);
		*pi.str = '\0';
		
		origin.x = MAX( window->cursor.x, window->hborder );
		origin.y = MAX( window->cursor.y, window->vborder );
		bounds.x = ( window->width - ( window->hborder * 2 ) );
		bounds.y = ( window->height - ( window->vborder * 2 ) );
		cursor = origin;

		for( i=0; i< strlen(formattedtext); i++ ) {
			character = formattedtext[i];
			character -= 32;
			
			// newline ?
			if( formattedtext[i] == '\n' ) {
				cursor.x = window->hborder;
				cursor.y += font->height;
				if ( cursor.y > bounds.y ) {
					gui.redraw = true;
					updateVRAM();
					cursor.y = window->vborder;
				}
				window->cursor.y = cursor.y;
				continue;
			}

			// tab ?
			if( formattedtext[i] == '\t' ) {
				cursor.x = ( cursor.x / ( font->chars[0]->width * 8 ) + 1 ) * ( font->chars[0]->width * 8 );
				continue;
			}
			cursor.x += font->chars[character]->width;
			
			// check x pos and do newline
			if ( cursor.x > bounds.x ) {
				cursor.x = origin.x;
				cursor.y += font->height;
			}

			// check y pos and reset to origin.y
			if ( cursor.y > ( bounds.y + font->chars[0]->height) ) {
				gui.redraw = true;
				updateVRAM();
				cursor.y = window->vborder;
			}
			// draw the character
			if( font->chars[character]) {
				blend(font->chars[character], gui.backbuffer, cursor);
			}
		}
		// save cursor postition
		window->cursor.x = cursor.x;
		updateVRAM();
		free(formattedtext);
		return 0;
	}
	return 1;
}

pixmap_t* charToPixmap(unsigned char ch, font_t *font) {
	unsigned int cha = (unsigned int)ch - 32;
	if (cha >= font->count) {
		// return ? if the font for the char doesn't exists
		cha = '?' - 32;
	}
	return font->chars[cha] ? font->chars[cha] : NULL;
}

position_t drawChar(unsigned char ch, font_t *font, pixmap_t *blendInto, position_t p) {
	pixmap_t* pm = charToPixmap(ch, font);
	if (pm && ((p.x + pm->width) < blendInto->width)) {
		blend(pm, blendInto, p);
		return pos(p.x + pm->width, p.y);
	} else {
		return p;
	}
}

void drawStr(char *ch, font_t *font, pixmap_t *blendInto, position_t p)
{
	int i=0;
	position_t current_pos = pos(p.x, p.y);
	
	for (i=0; i < strlen(ch); i++) {
		// newline ?
		if ( ch[i] == '\n' ) {
			current_pos.x = p.x;
			current_pos.y += font->height;
			continue;
		}
		
		// tab ?
		if ( ch[i] == '\t' ) {
			current_pos.x += TAB_PIXELS_WIDTH;
			continue;
		}
		
		current_pos = drawChar(ch[i], font, blendInto, current_pos);
	}
}

void drawStrCenteredAt(char *text, font_t *font, pixmap_t *blendInto, position_t p)
{
	int i = 0;
	int width = 0;
	int max_width = 0;
	int height = font->height;

	// calculate the width in pixels
	for (i=0; i < strlen(text); i++) {
		if (text[i] == '\n') {
			width = 0;
			height += font->height;
		} else if (text[i] == '\t') {
			width += TAB_PIXELS_WIDTH;
		} else {
			pixmap_t* pm = charToPixmap(text[i], font);
			if (pm)	{
				width += pm->width;
			}
		}
		if (width > max_width) {
			max_width = width;
		}
	}

	p.x = ( p.x - ( max_width / 2 ) );
	p.y = ( p.y - ( height / 2 ) );
	
	drawStr(text, font, blendInto, p);
}

int destroyFont(font_t *font)
{
	int i;
	for (i = 0; i < CHARACTERS_COUNT; i++) {
		if (font->chars[i]) {
			if (font->chars[i]->pixels) {
				free (font->chars[i]->pixels);
			}
			free (font->chars[i]);
			font->chars[i] = 0;
		}
	}
	return 0;
}

int initFont(font_t *font, image_t *data)
{
	unsigned int x = 0, y = 0, x2 = 0, x3 = 0;
	
	int start = 0, end = 0, count = 0, space = 0;
	
	bool monospaced = false;
	
	font->height = data->image->height;

	for( x = 0; x < data->image->width && count < CHARACTERS_COUNT; x++) {
		start = end;

		// if the pixel is red we've reached the end of the char
		if( pixel( data->image, x, 0 ).value == 0xFFFF0000) {
			end = x + 1;

			if( (font->chars[count] = malloc(sizeof(pixmap_t)) ) ) {
				font->chars[count]->width = ( end - start) - 1;
				font->chars[count]->height = font->height;
			
				if ( ( font->chars[count]->pixels = malloc( font->chars[count]->width * data->image->height * 4) ) ) {
					space += ( font->chars[count]->width * data->image->height * 4 );
					// we skip the first line because there are just the red pixels for the char width
					for( y = 1; y< (font->height); y++) {
						for( x2 = start, x3 = 0; x2 < end; x2++, x3++) {
							pixel( font->chars[count], x3, y ) = pixel( data->image, x2, y );
						}	
					}
					
					// check if font is monospaced
					if( ( count > 0 ) && ( font->width != font->chars[count]->width ) ) {
						monospaced = true;
					}

					font->width = font->chars[count]->width;
					
					count++;
				}
			}
		}
	}

	for (x = count; x < CHARACTERS_COUNT; x++) {
		font->chars[x] = NULL;
	}

	if(monospaced) {
		font->width = 0;
	}

	font->count = count;

	return 0;
}

void colorFont(font_t *font, uint32_t color)
{
	if( !color ) {
		return;
	}

	int x, y, width, height;
	int count = 0;
	pixel_t *buff;
	
	while( font->chars[count++] ) {
		width = font->chars[count-1]->width;
		height = font->chars[count-1]->height;
		for( y = 0; y < height; y++ ) {
			for( x = 0; x < width; x++ ) {
				buff = &(pixel( font->chars[count-1], x, y ));
				if( buff->ch.a ) {
					buff->ch.r = (color & 0xFFFF0000) >> 16;
					buff->ch.g = (color & 0xFF00FF00) >> 8;
					buff->ch.b = (color & 0xFF0000FF);
				}
			}
		}
	}
}

void makeRoundedCorners(pixmap_t *p)
{
	int x,y;
	int width=p->width-1;
	int height=p->height-1;
	
	// 10px rounded corner alpha values
	uint8_t roundedCorner[10][10] =
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0xC0, 0xFF},
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x00, 0x00, 0x00, 0x40, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x00, 0x00, 0x40, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x00, 0x40, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x00, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x40, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		{ 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
	};
	
	uint8_t alpha=0;
	
	for( y=0; y<10; y++) {
		for( x=0; x<10; x++) {
			// skip if the pixel should be visible
			if(roundedCorner[y][x] != 0xFF)	{ 
				alpha = ( roundedCorner[y][x] ? (uint8_t) (roundedCorner[y][x] * pixel(p, x, y).ch.a) / 255 : 0 );
				// Upper left corner
				pixel(p, x, y).ch.a = alpha;

				// upper right corner
				pixel(p, width-x,y).ch.a = alpha;

				// lower left corner
				pixel(p, x, height-y).ch.a = alpha;

				// lower right corner
				pixel(p, width-x, height-y).ch.a = alpha;
			}
		}
	}
}

void showInfoBox(char *title, char *text_orig)
{
	char* text;
	int i, key, lines, visiblelines;

	int currentline=0;
	int cnt=0;
	int offset=0;
	
	if( !title || !text_orig ) {
		return;
	}

	// Create a copy so that we don't mangle the original
	text = malloc(strlen(text_orig) + 1);
	strcpy(text, text_orig);

	position_t pos_title = pos ( gui.infobox.vborder, gui.infobox.vborder );

	// calculate number of lines in the title
	for ( i = 0, lines = 1; i<strlen(title); i++ ) {
		if( title[i] == '\n') {
			lines++;
		}
	}
	// y position of text is lines in title * height of font
	position_t pos_text =  pos( pos_title.x , pos_title.y + ( font_console.height * lines ));
	
	// calculate number of lines in the text
	for ( i=0, lines = 1; i<strlen(text); i++ ) {
		if( text[i] == '\n') {
			lines++;
		}
	}
	// if text ends with \n strip off
	if( text[i] == '\n' || text[i] == '\0') {
		lines--;
	}
	visiblelines = ( ( gui.infobox.height - ( gui.infobox.vborder * 2 ) ) / font_console.height ) - 1;
	
	// lets display the text and allow scroll thru using up down / arrows
	while(1) {
		// move to current line in text
		for( offset = 0, i = 0; offset < strlen(text); offset++ ) {
			if( currentline == i) {
				break;
			}
			if( text[offset] =='\n') {
				i++;
			}
		}

		// find last visible line in text and place \0
		for( i = offset, cnt = 0; i < strlen(text); i++) {
			if(text[i]=='\n') {
				cnt++;
			}
			if ( cnt == visiblelines ) {
				text[i]='\0';
				break;
			}
		}

		fillPixmapWithColor( gui.infobox.pixmap, gui.infobox.bgcolor);

		makeRoundedCorners( gui.infobox.pixmap);

		// print the title if present
		if( title ) {
			drawStr(title, &font_console, gui.infobox.pixmap, pos_title);
		}
		// print the text
		drawStr( text + offset, &font_console, gui.infobox.pixmap, pos_text);

		// restore \n in text
		if ( cnt == visiblelines ) {
			text[i] = '\n';
		}
		position_t pos_indicator =  pos( gui.infobox.width - ( images[iTextScrollPrev].image->width - ( gui.infobox.vborder / 2) ), pos_text.y );
		
		// draw prev indicator
		if(offset) {
			blend( images[iTextScrollPrev].image, gui.infobox.pixmap, centeredAt( images[iTextScrollPrev].image, pos_indicator ));
		}
		
		// draw next indicator
		if( lines > ( currentline + visiblelines ) ) {
			pos_indicator.y = ( gui.infobox.height - ( ( images[iTextScrollNext].image->width + gui.infobox.vborder ) / 2 ) );
			blend( images[iTextScrollNext].image, gui.infobox.pixmap, centeredAt( images[iTextScrollNext].image, pos_indicator ) );
		}

		gui.bootprompt.draw = false;
		gui.infobox.draw = true;
		gui.redraw = true;
		
		updateVRAM();
		
		key = getchar();
			
		if( key == KEY_UP ) {
			if( currentline > 0 ) {
				currentline--;
			}
		}

		if( key == KEY_DOWN ) {
			if( lines > ( currentline + visiblelines ) ) {
				currentline++;
			}
		}

		if( key == KEY_ESC || key == 'q' || key == 'Q') {
			gui.infobox.draw = false;
			gui.redraw = true;
			updateVRAM();
			break;
		}

		if(key == ' ') { // spacebar = next page
			if( lines > ( currentline + visiblelines ) ) {
				currentline += visiblelines;
			}
			if(lines < (currentline + visiblelines)) {
				currentline = lines - visiblelines;
			}
		}
	}
	free(text);
}

void animateProgressBar()
{
	int y;
	
	if( time18() > lasttime) {
		lasttime = time18();

		pixmap_t *buffBar = images[iProgressBar].image;

		uint32_t buff = buffBar->pixels[0].value;
	
		memcpy( buffBar->pixels, buffBar->pixels + 1, ( (buffBar->width*buffBar->height) - 1 ) * 4 );

		for( y = buffBar->height - 1; y > 0; y--) {
			pixel(buffBar, buffBar->width - 1, y) = pixel(buffBar, buffBar->width - 1, y - 1);
		}
		pixel(buffBar, buffBar->width-1, 0).value = buff;
	}
}

void drawProgressBar(pixmap_t *blendInto, uint16_t width, position_t p, uint8_t progress)
{
	if(progress>100) {
		return;
	}

	p.x = ( p.x - ( width / 2 ) );

	int todraw = (width * progress) / 100;

	pixmap_t *buff = images[iProgressBar].image;
	pixmap_t *buffBG = images[iProgressBarBackground].image;
	if(!buff || !buffBG) {
		return;
	}

	pixmap_t progressbar;
	progressbar.pixels=malloc(width * 4 * buff->height);
	if(!progressbar.pixels) {
		return; 
	}

	progressbar.width = width;
	progressbar.height = buff->height;

	int x=0,x2=0,y=0;
	
	for(y=0; y<buff->height; y++) {
		for(x=0; x<todraw; x++, x2++) {
			if(x2 == (buff->width-1)) {
				x2=0;
			}
			pixel(&progressbar, x,y).value = pixel(buff, x2,y).value;
		}
		x2=0;
	}

	for(y=0; y<buff->height; y++) {
		for(x=todraw, x2 = 0; x < width - 1; x++, x2++) {
			if(x2 == (buffBG->width -2 )) {
				x2 = 0;
			}
			pixel(&progressbar, x,y).value = pixel(buffBG, x2,y).value;
		}
		if(progress < 100) {
			pixel(&progressbar, width - 1, y).value = pixel(buffBG, buffBG->width - 1, y).value;
		}

		if(progress == 0) {
			pixel(&progressbar, 0, y).value = pixel(buffBG, buffBG->width - 1, y).value;
		}

		x2=0;
	}

	blend(&progressbar, blendInto, p);
	animateProgressBar();
	free(progressbar.pixels);
}

void drawInfoMenuItems()
{
	int i,n;
	
	position_t position;
	
	pixmap_t *selection = images[iMenuSelection].image;
	
	pixmap_t *pbuff;

	fillPixmapWithColor(gui.menu.pixmap, gui.menu.bgcolor);

	makeRoundedCorners(gui.menu.pixmap);
	
	uint8_t offset = infoMenuNativeBoot ? 0 : infoMenuItemsCount - 1;

	position = pos(0,0);
	
	for ( i = 0, n = iMenuBoot; i < infoMenuItemsCount; i++, n++) {
		if (i == infoMenuSelection) {
			blend(selection, gui.menu.pixmap, position);
		}

		pbuff = images[n].image;
		if (offset && i >= INFOMENU_NATIVEBOOT_START && i <= INFOMENU_NATIVEBOOT_END) {
			blend( images[n + (iMenuHelp - iMenuBoot)].image , gui.menu.pixmap, 
				pos((position.x + (gui.menu.hborder / 2)), position.y + ((selection->height - pbuff->height) / 2)));
		} else {
			blend( pbuff, gui.menu.pixmap, 
				pos((position.x + (gui.menu.hborder / 2)), position.y + ((selection->height - pbuff->height) / 2)));
		}

		drawStr(infoMenuItems[i].text, &font_console, gui.menu.pixmap, 
			pos(position.x + (pbuff->width + gui.menu.hborder), 
				position.y + ((selection->height - font_console.height) / 2)));
		position.y += images[iMenuSelection].image->height;
	
	}
	
	gui.redraw = true;
}

int drawInfoMenu()
{
	drawInfoMenuItems();

	gui.menu.draw = true;

	updateVRAM();
	
	return 1;
}

int updateInfoMenu(int key)
{
	switch (key)
	{

		case KEY_UP:	// up arrow
			if (infoMenuSelection > 0) {
				if(!infoMenuNativeBoot && infoMenuSelection == INFOMENU_NATIVEBOOT_END + 1) {
					infoMenuSelection -= 4;
				} else {
					infoMenuSelection--;
				}
				drawInfoMenuItems();
				updateVRAM();

			} else {

				gui.menu.draw = false;
				gui.redraw = true;

				updateVRAM();
				
				return CLOSE_INFO_MENU;
			}
			break;

		case KEY_DOWN:	// down arrow
			if (infoMenuSelection < infoMenuItemsCount - 1) {
				if(!infoMenuNativeBoot && infoMenuSelection == INFOMENU_NATIVEBOOT_START - 1)
				{
					infoMenuSelection += 4;
				} else {
					infoMenuSelection++;
				}
				drawInfoMenuItems();
				updateVRAM();
			}
			break;

		case KEY_ENTER:
			key = 0;
			if( infoMenuSelection == MENU_SHOW_MEMORY_INFO ) {
				showInfoBox( "Memory Info. Press q to quit.\n", getMemoryInfoString());
			} else if( infoMenuSelection == MENU_SHOW_VIDEO_INFO ) {
				showInfoBox( getVBEInfoString(), getVBEModeInfoString() );
			} else if( infoMenuSelection == MENU_SHOW_HELP ) {
				showHelp();
			} else {
				int buff = infoMenuSelection;
				infoMenuSelection = 0;
				return buff;
			}
			break;
	}
	return DO_NOT_BOOT;
}

uint16_t bootImageWidth = 0; 
uint16_t bootImageHeight = 0; 
uint8_t *bootImageData = NULL; 
static bool usePngImage = true;

//==========================================================================
// loadBootGraphics
static void loadBootGraphics(void)
{
	if (bootImageData != NULL) {
		return;
	}

	char dirspec[256];

	if ((strlen(theme_name) + 24) > sizeof(dirspec)) {
		usePngImage = false; 
		return;
	}
	sprintf(dirspec, "/Extra/Themes/%s/boot.png", theme_name);
	if (loadPngImage(dirspec, &bootImageWidth, &bootImageHeight, &bootImageData) != 0) {
#ifdef CONFIG_EMBED_THEME
		if ((loadEmbeddedPngImage(__boot_png, __boot_png_len, &bootImageWidth, &bootImageHeight, &bootImageData)) != 0)
#endif
		usePngImage = false; 
	}
}

//==========================================================================
// drawBootGraphics
void drawBootGraphics(void)
{
	int pos;
	int length;
	const char *dummyVal;
	int oldScreenWidth, oldScreenHeight;
	bool legacy_logo;
	uint16_t x, y; 
	
	if (getBoolForKey("Legacy Logo", &legacy_logo, &bootInfo->chameleonConfig) && legacy_logo) {
		usePngImage = false; 
	} else if (bootImageData == NULL) {
		loadBootGraphics();
	}

	// parse screen size parameters
	if (getIntForKey("boot_width", &pos, &bootInfo->themeConfig) && pos > 0) {
		screen_params[0] = pos;
	} else {
		screen_params[0] = DEFAULT_SCREEN_WIDTH;
	}
	if (getIntForKey("boot_height", &pos, &bootInfo->themeConfig) && pos > 0) {
		screen_params[1] = pos;
	} else {
		screen_params[1] = DEFAULT_SCREEN_HEIGHT;
	}

	// Save current screen resolution.
	oldScreenWidth = gui.screen.width;
	oldScreenHeight = gui.screen.height;

	gui.screen.width = screen_params[0];
	gui.screen.height = screen_params[1];

	// find best matching vesa mode for our requested width & height
	getGraphicModeParams(screen_params);

	// Set graphics mode if the booter was in text mode or the screen resolution has changed.
	if (bootArgs->Video.v_display == VGA_TEXT_MODE || (screen_params[0] != oldScreenWidth && screen_params[1] != oldScreenHeight) ) {
		setVideoMode(GRAPHICS_MODE, 0);
	}

	if (getValueForKey("-checkers", &dummyVal, &length, &bootInfo->chameleonConfig)) {
		drawCheckerBoard();
	} else {
		// Fill the background to 75% grey (same as BootX). 
		drawColorRectangle(0, 0, screen_params[0], screen_params[1], 0x01); 
	}
	if ((bootImageData) && (usePngImage)) { 
		x = (screen_params[0] - MIN(bootImageWidth, screen_params[0])) / 2; 
		y = (screen_params[1] - MIN(bootImageHeight, screen_params[1])) / 2; 

		// Draw the image in the center of the display. 
		blendImage(x, y, bootImageWidth, bootImageHeight, bootImageData); 
	} else { 
		uint8_t *appleBootPict; 
		bootImageData = NULL; 
		bootImageWidth = kAppleBootWidth; 
		bootImageHeight = kAppleBootHeight; 

		// Prepare the data for the default Apple boot image. 
		appleBootPict = (uint8_t *) decodeRLE(gAppleBootPictRLE, kAppleBootRLEBlocks, bootImageWidth * bootImageHeight); 
		if (appleBootPict) { 
			convertImage(bootImageWidth, bootImageHeight, appleBootPict, &bootImageData); 
			if (bootImageData) {	
				x = (screen_params[0] - MIN(kAppleBootWidth, screen_params[0])) / 2; 
				y = (screen_params[1] - MIN(kAppleBootHeight, screen_params[1])) / 2; 
				drawDataRectangle(x, y, kAppleBootWidth, kAppleBootHeight, bootImageData);
				free(bootImageData);
			}
			free(appleBootPict); 
		} 
	} 
}
