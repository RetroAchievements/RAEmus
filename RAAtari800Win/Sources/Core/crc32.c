
///////////////////////////////////////////////////////////////////////////////
// crc32.c: Core CRC calculation code and data
//	Holds routines to check a whole file or an arbitrary segment of memory
//////////////////////////////////////////////////////////////////////////////

#pragma warning (disable: 4201 4310)

#include "crc.h"
#include <fcntl.h>
#include <stdlib.h>        
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <malloc.h>
#include <windows.h>

#define CRCDEBUG_STR(text) {};

///////////////////////////////////////////////////////////////////////////////
// The data table used by the UpdateCRC() function to calculate a CRC

static unsigned long CRCTable[256];
unsigned long crc;

unsigned long CheckFile( char *fn, int *result )
{
	int	file=0;
	unsigned long limit, count;
    unsigned char *buffer;

	file = _open( fn, _O_RDONLY | _O_BINARY );

	if( file == -1 )
	{
		*result = CRC_COULD_NOT_CHECK;
		return 0;
	}
    limit = _filelength( file );

	buffer = (unsigned char *)malloc( CRCBUFFLEN );
	if( !buffer) 
	{
		*result = CRC_COULD_NOT_CHECK;
		return 0;
	}
	
    crc = 0xFFFFFFFFL;

    for( ; ; )
	{
	    count = _read( file, (void *)buffer, CRCBUFFLEN );
		if( count == 0 )
			break;
		crc = CalculateBufferCRC( crc, 0L, count, buffer );
	}

	_close( file );
    free(buffer);

	*result = 0;
	return( crc );
}

/*
 * This routine calculates the CRC for a block of data using the
 * table lookup method. It accepts an original value for the crc,
 * and returns the updated value.
 */

unsigned long CalculateBufferCRC(unsigned long crc_tmp, unsigned long start, unsigned long count, unsigned char *buffer )
{
    unsigned char *p;
    unsigned long temp1;
    unsigned long temp2;

    p = (unsigned char*) buffer;
	p += start;

    while ( count-- != 0 ) {
        temp1 = ( crc_tmp >> 8 ) & 0x00FFFFFFL;
        temp2 = CRCTable[ ( (int) crc_tmp ^ *p++ ) & 0xff ];
        crc_tmp = temp1 ^ temp2;
    }
    return( crc_tmp );
}

void BuildCRCTable(void)
{
    register int i;
    register int j;
    unsigned long tmpcrc;

    for ( i = 0; i <= 255 ; i++ ) {
        tmpcrc = i;
        for ( j = 8 ; j > 0; j-- ) {
            if ( tmpcrc & 1 )
                tmpcrc = ( tmpcrc >> 1 ) ^ CRC32_POLYNOMIAL;
            else
                tmpcrc >>= 1;
        }
        CRCTable[ i ] = tmpcrc;
    }
}
