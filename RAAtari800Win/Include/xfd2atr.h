#ifndef __XFD2ATR__
#define __XFD2ATR__

#ifdef __cplusplus
extern "C" {
#endif

/* Codes returned by XFD2ATR_Open and XFD2ATR_Convert */
#define XFD2ATR_OK				0		/* No error */
#define XFD2ATR_QUERY_BOOT		1		/* No error, prompt user to choose boot sectors type */
#define XFD2ATR_ERR_XFD_FILE	2		/* Error reading the XFD file */
#define XFD2ATR_ERR_XFD_CORRUPT	3		/* The XFD file is corrupt */
/* Codes returned by XFD2ATR_Convert only */
#define XFD2ATR_ERR_ATR_FILE	4		/* Error writing the ATR file */

/* Boot sectors types, passed to XFD2ATR_Convert */
#define XFD2ATR_BOOT_LOGICAL	0
#define XFD2ATR_BOOT_PHYSICAL	1
#define XFD2ATR_BOOT_SIO2PC		2

/* Open the XFD file and check its format */
int XFD2ATR_Open(char *xfdName);

/* Convert an open XFD file to an ATR file */
int XFD2ATR_Convert(char *atrName, int bootType);

/* Close files - always call after successful conversion, any error,
   or if user cancels while choosing boot sectors type */
void XFD2ATR_Close(void);

#ifdef __cplusplus
}
#endif

#endif
