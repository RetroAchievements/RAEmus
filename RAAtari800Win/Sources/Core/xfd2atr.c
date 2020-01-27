/* XFD->ATR file conversion */
/* by Piotr Fusik <fox@scene.pl> */

#include <stdio.h>
#include <string.h>

#include "atari.h"				/* for AFILE_ATR_Header structure */
#include "xfd2atr.h"
#include "afile.h"


/* Files */
static FILE *xfdFile = NULL;
static FILE *atrFile = NULL;

/* Image properties */
static int sectorSize;			/* Size of a non-boot sector */
static int xfdBootSectorSize;	/* Size of a boot sector */
static int sectorCount;			/* Number of all sectors */

/* Open the XFD file and check its format */
int XFD2ATR_Open(char *xfdName)
{
	long xfdImageSize;
	/* Open the XFD file */
	xfdFile = fopen(xfdName, "rb");
	if (xfdFile == NULL)
		return XFD2ATR_ERR_XFD_FILE;
	/* Determine length of the file */
	fseek(xfdFile, 0L, SEEK_END);
	xfdImageSize = ftell(xfdFile);
	fseek(xfdFile, 0L, SEEK_SET);
	/* If the image size is smaller or equal to the size of a medium density disk,
	   it has 128-byte sectors */
	if (xfdImageSize <= 1040 * 128) {
		/* Check if there are full sectors */
		if (xfdImageSize == 0 || xfdImageSize % 128 != 0)
			return XFD2ATR_ERR_XFD_CORRUPT;
		sectorSize = 128;
		xfdBootSectorSize = 128;
		sectorCount = xfdImageSize / 128;
	}
	/* The image is greater than a medium density disk - assume 256-byte sectors */
	else {
		sectorSize = 256;
		/* Full 256-byte sectors */
		if (xfdImageSize % 256 == 0) {
			xfdBootSectorSize = 256;
			sectorCount = xfdImageSize / 256;
		}
		/* Possibly first 3 sectors (boot sectors) are 128-byte */
		else {
			if ((xfdImageSize - 384) % 256 != 0)
				return XFD2ATR_ERR_XFD_CORRUPT;
			xfdBootSectorSize = 128;
			sectorCount = 3 + (xfdImageSize - 384) / 256;
		}
		return XFD2ATR_QUERY_BOOT;
	}
	return XFD2ATR_OK;
}

/* Convert an open XFD file to an ATR file */
int XFD2ATR_Convert(char *atrName, int bootType)
{
	struct AFILE_ATR_Header atrHeader;
	char buffer[768];
	int bootSectorCount;
	int bootSize;
	int imagePara;
	int offset;
	int i;
	/* Compute number of boot sectors and total size */
	bootSectorCount = sectorCount < 3 ? sectorCount : 3;
	if (sectorSize == 128)
		bootSize = 128 * bootSectorCount;
	else
		switch (bootType) {
		case XFD2ATR_BOOT_LOGICAL:
			bootSize = 128 * bootSectorCount;
			break;
		case XFD2ATR_BOOT_PHYSICAL:
			bootSize = 256 * bootSectorCount;
			break;
		case XFD2ATR_BOOT_SIO2PC:
			bootSize = 768;
			break;
		default:
			return XFD2ATR_QUERY_BOOT;
		}
	imagePara = (bootSize + (sectorCount - bootSectorCount) * sectorSize) >> 4;
	/* Open the ATR file */
	atrFile = fopen(atrName, "wb");
	if (atrFile == NULL)
		return XFD2ATR_ERR_ATR_FILE;
	/* Write the header */
	memset(&atrHeader, 0, sizeof(atrHeader));
	atrHeader.magic1 = AFILE_ATR_MAGIC1;
	atrHeader.magic2 = AFILE_ATR_MAGIC2;
	atrHeader.seccountlo = imagePara & 0xff;
	atrHeader.seccounthi = (imagePara >> 8) & 0xff;
	atrHeader.secsizelo = sectorSize & 0xff;
	atrHeader.secsizehi = (sectorSize >> 8) & 0xff;
	atrHeader.hiseccountlo = (imagePara >> 16) & 0xff;
	atrHeader.hiseccounthi = (imagePara >> 24) & 0xff;
	if (fwrite(&atrHeader, sizeof(atrHeader), 1, atrFile) != 1)
		return XFD2ATR_ERR_ATR_FILE;
	/* First, convert the boot sectors */
	memset(buffer, 0, sizeof(buffer));
	for (i = 0; i < bootSectorCount; i++) {
		if (sectorSize == 128)
			offset = 128 * i;
		else
			switch (bootType) {
			case XFD2ATR_BOOT_LOGICAL:
			case XFD2ATR_BOOT_SIO2PC:
				offset = 128 * i;
				break;
			case XFD2ATR_BOOT_PHYSICAL:
				offset = 256 * i;
				break;
			}
		if (fread(buffer + offset, xfdBootSectorSize, 1, xfdFile) != 1)
			return XFD2ATR_ERR_XFD_FILE;
	}
	if (sectorSize != 128 && bootType == XFD2ATR_BOOT_SIO2PC)
		memset(buffer + 384, 0, 384);
	if (fwrite(buffer, bootSize, 1, atrFile) != 1)
		return XFD2ATR_ERR_ATR_FILE;
	/* Then convert rest of the disk */
	for (i = bootSectorCount; i < sectorCount; i++) {
		if (fread(buffer, sectorSize, 1, xfdFile) != 1)
			return XFD2ATR_ERR_XFD_FILE;
		if (fwrite(buffer, sectorSize, 1, atrFile) != 1)
			return XFD2ATR_ERR_ATR_FILE;
	}
	return XFD2ATR_OK;
}

/* Close files */
void XFD2ATR_Close(void)
{
	if (xfdFile != NULL) {
		fclose(xfdFile);
		xfdFile = NULL;
	}
	if (atrFile != NULL) {
		fclose(atrFile);
		atrFile = NULL;
	}
}
