/****************************************************************************
File    : rdevice.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of R: device stuff
@(#) #BY# Daniel Noguerol, Piotr Fusik
@(#) #LM# 13.07.2003
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#include "atari800.h"
#include "globals.h"
#include "rdevice.h"


/* Added by Tomek Szymankowski

/* Private objects */

static HANDLE  s_hWinSock2        = NULL;

/* WinSock starters */

static FARPROC s_pfnCloseSocket   = NULL;
static FARPROC s_pfnIOCtlSocket   = NULL;
static FARPROC s_pfnConnect       = NULL;
static FARPROC s_pfnHtons         = NULL;
static FARPROC s_pfnSocket        = NULL;
static FARPROC s_pfnGetHostByName = NULL;
static FARPROC s_pfnWSAStartup    = NULL;
static FARPROC s_pfnSend          = NULL;
static FARPROC s_pfnRecv          = NULL;
static FARPROC s_pfnInetNtoa      = NULL;
static FARPROC s_pfnAccept        = NULL;
static FARPROC s_pfnListen        = NULL;
static FARPROC s_pfnBind          = NULL;

/* Wrappers for WinSock2 exports */

static int       WSCLOSESOCKET  ( SOCKET nSocket );
static int       WSIOCTLSOCKET  ( SOCKET nSocket, long nCmd, u_long FAR *pArgp );
static int       WSCONNECT      ( SOCKET nSocket, const LPSOCKADDR pszName, int nNameLen );
static u_short   WSHTONS        ( u_short nHostShort );
static SOCKET    WSSOCKET       ( int nAf, int nType, int nProtocol );
static LPHOSTENT WSGETHOSTBYNAME( const char FAR *pszName );
static int       WSWSASTARTUP   ( WORD wVersionRequested, LPWSADATA pWSAData );
static int       WSSEND         ( SOCKET nSocket, const char FAR *pszBuf, int nLen, int nFlags );
static int       WSRECV         ( SOCKET nSocket, char FAR *pszBuf, int nLen, int nFlags );
static char FAR* WSINETNTOA     ( IN_ADDR inAddr );
static SOCKET    WSACCEPT       ( SOCKET nSocket, LPSOCKADDR pAddr, int FAR *pAddrLen );
static int       WSLISTEN       ( SOCKET nSocket, int nBackLog );
static int       WSBIND         ( SOCKET nSocket, const LPSOCKADDR pszName, int nNameLen );


/* following is copied from devices.c */

static char filename[64];

static int Device_isvalid(UBYTE ch)
{
	int valid;

	if (ch < 0x80 && isalnum(ch))
		valid = TRUE;
	else
		switch (ch) {
		case ':':
		case '.':
		case '_':
		case '*':
		case '?':
			valid = TRUE;
			break;
		default:
			valid = FALSE;
			break;
		}

	return valid;
}

static void Device_GetFilename(void)
{
	int bufadr;
	int offset = 0;
	int devnam = TRUE;

	bufadr = (MEMORY_dGetByte(Devices_ICBAHZ) << 8) | MEMORY_dGetByte(Devices_ICBALZ);

	while (Device_isvalid(MEMORY_dGetByte(bufadr))) {
		int byte = MEMORY_dGetByte(bufadr);

		if (!devnam) {
			if (isupper(byte))
				byte = tolower(byte);

			filename[offset++] = byte;
		}
		else if (byte == ':')
			devnam = FALSE;

		bufadr++;
	}

	filename[offset++] = '\0';
}

/* Original code by Daniel Noguerol <dnoguero@earthlink.net>.

   Changes by Piotr Fusik <fox@scene.pl>:
   - removed fid in xio_34 (was unused)
   - Peek -> MEMORY_dGetByte, Poke -> MEMORY_dPutByte
   - Device_* functions renamed to match devices.c naming convention
   - connection info written to Atari800 log, not c:\\Atari800Log.txt
*/

/**********************************************/
/*             R: stuff                       */
/**********************************************/
static int do_once;
static int winsock_started;

UBYTE r_dtr, r_rts, r_sd;
UBYTE r_dsr, r_cts, r_cd;
UBYTE r_parity, r_stop;
UBYTE r_error, r_in;
UBYTE r_tr = 32, r_tr_to = 32;
unsigned int r_stat;
unsigned int svainit;

char ahost[256];
struct hostent *hent;
unsigned long int haddress;

struct sockaddr_in in;
struct sockaddr_in peer_in;
int myport;
char PORT[256];
char MESSAGE[256];
int newsock;
int len;
char buf[256];
fd_set fd;
struct timeval tv;
char str[256];
int retval;
char buffer[200];
char reading=0;
unsigned char one;
unsigned char bufout[256];
unsigned char bufend = 0;
char temp1;
char temp2;
int bytesread;
int concurrent;
int connected;
int sock;


void xio_34(void)
{
	int temp;

/* Controls handshake lines DTR, RTS, SD */

	temp = MEMORY_dGetByte(Devices_ICAX1Z);

	if (temp & 128) {
		if (temp & 64) {
/* turn DTR on */
		} else {
			if (connected != 0) {
				WSCLOSESOCKET(newsock);
				connected = 0;
				do_once = 0;
				concurrent = 0;
				bufend = 0;
			}
		}
	}
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;

	MEMORY_dPutByte(747,0);
}

void xio_36(void)
{
/* Sets baud, stop bits, and ready monitoring. */

	r_cd = MEMORY_dGetByte(Devices_ICAX2Z);

	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
	MEMORY_dPutByte(747,0);
}



void xio_38(void)
{
/* Translation and parity */

	r_tr = MEMORY_dGetByte(Devices_ICAX1Z);
	r_tr_to = MEMORY_dGetByte(Devices_ICAX2Z);

	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
	MEMORY_dPutByte(747,0);
}

void xio_40(void)
{

/* Sets concurrent mode.  Also checks for dropped carrier. */

/*
		if(connected == 0)
			MEMORY_dPutByte(747,0);
*/

	MEMORY_dPutByte(747,0);
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
	concurrent = 1;
}


void Device_RHOPEN(void)
{
int temp, mode, nWSAerror;
u_long argp = 1L;
static unsigned long int HOST;
static int PORT;
char port[] = "        \n";
struct hostent *hostlookup;
static unsigned long int hent;
WSADATA stWSAData;

/* open #1,8,23,"R:cth.tzo.com" =
   open port 23 of cth.tzo.com
*/
	// set up winsock
	if (winsock_started == 0) {
		winsock_started = 1;
		nWSAerror = WSWSASTARTUP(0x0101, &stWSAData);
		if (nWSAerror != 0) {
			Log_print("Unable to start Winsock!");
		}
	}

	temp = MEMORY_dGetByte(Devices_ICAX1Z);
	mode = MEMORY_dGetByte(0x30a);
	PORT = (int) MEMORY_dGetByte(CPU_regX + 0x340 + 11);
	if (temp == 8 && PORT > 0)
	{
		Log_print("R: request to dial-out intercepted.");
		Device_GetFilename();
		strcpy(ahost, filename);
		Log_print(ahost);
		_itoa(PORT, port, 5);
		hostlookup = WSGETHOSTBYNAME(ahost);
		if (hostlookup == NULL)
		{
			Log_print("ERROR! Cannot resolve %s.", ahost);
			MEMORY_dPutByte(747,0);
			CPU_regA = 170;
			CPU_regY = 170;
			CPU_SetN;
			return;
		}
		Log_print("Resolve successful.");

		HOST = ((struct in_addr *)hostlookup->h_addr)->s_addr;
		Log_print("Got HOST.\n");

		memset(&in, 0, sizeof(struct sockaddr_in));

		if ((newsock = WSSOCKET(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			Log_print("Socket connect error.");
			MEMORY_dPutByte(747, 0);
			CPU_regA = 170;
			CPU_regY = 170;
			CPU_SetN;
			return;
		}
		in.sin_family = AF_INET;
		in.sin_port = WSHTONS((u_short)PORT);
		in.sin_addr.s_addr = HOST;

		Log_print("Ready to connect.");

		retval = WSCONNECT(newsock, (struct sockaddr *)&in, sizeof(struct sockaddr_in));

		Log_print("Got retval.");

		if (retval == -1)
		{
			Log_print("Connect error.");
			MEMORY_dPutByte(747, 0);
			CPU_regA = 170;
			CPU_regY = 170;
			CPU_SetN;
			return;
		}

		Log_print("Successful connect.");
		WSIOCTLSOCKET(sock, FIONBIO, (u_long FAR*)&argp);
		connected = 1;
		concurrent = 1;
		MEMORY_dPutByte(747, 0);
		CPU_regA = 1;
		CPU_regY = 1;
		CPU_ClrN;
		return;

	}

	MEMORY_dPutByte(747,0);
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;

}

void Device_RHCLOS(void)
{
	MEMORY_dPutByte(747,0);
	bufend = 0;
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
	concurrent = 0;
}

void Device_RHREAD(void)
{
	char j;

	if (bufend > 0) {
		CPU_regA = bufout[1];
		CPU_regY = 1;
		CPU_ClrN;

		bufend--;
		MEMORY_dPutByte(747,bufend);

		j = 1;

		for (j = 1; j <= bufend; j++) {
			bufout[j] = bufout[j+1];
		}

		/* translation */
		/* heavy */

		if (r_tr == 32) { /* no translation */
		} else {
			/* light translation */
			if (CPU_regA == 13) {
				CPU_regA = 155;
			} else if (CPU_regA == 10) {
				CPU_regA = 32;
			}
		}
		return;
	}
}



void Device_RHWRIT(void)
{
	if (connected != 0) {
			if (r_tr == 32) { /* no translation */
			} else {
				if (CPU_regA == 155) {
					CPU_regA = 13;
					retval = WSSEND(newsock, &CPU_regA, 1, 0);
					CPU_regA = 1;
					CPU_regY = 1;
					CPU_ClrN;
					MEMORY_dPutByte(749, 0);
					return;
				}
			}
			if (CPU_regA == 255)
				retval = WSSEND(newsock, &CPU_regA, 1, 0); /* IAC escape sequence */


		retval = WSSEND(newsock, &CPU_regA, 1, 0); /* returns -1 if disconnected */
		if (retval == -1) {
			Log_print("Error on R: write.");
			MEMORY_dPutByte(749,0);
			CPU_regA = 1;
			CPU_regY = 1;
			CPU_ClrN;


		} else {
			MEMORY_dPutByte(749,0);
			CPU_regA = 1;
			CPU_regY = 1;
			CPU_ClrN;
		}
		MEMORY_dPutByte(749,0); /* bytes waiting to be sent */
		return;
	}
	MEMORY_dPutByte(749,0);
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
}

void Device_RHSTAT(void)
{
	static char IACctr = 0;
//	FILE *fd;
//	unsigned int st;

	u_long argp = 1L;
	int nWSAerror;
	WSADATA stWSAData;
	struct tm *datetime;
	time_t lTime;

/* are we connected? */
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;

	if (connected == 0) {

		if (do_once == 0) {

			// set up winsock
			if (winsock_started == 0) {
				winsock_started = 1;
				nWSAerror = WSWSASTARTUP(0x0101, &stWSAData);
				if (nWSAerror != 0) {
					Log_print("Unable to start Winsock!");
				}
			}

			// Set up the listening port.
			do_once = 1;
			memset ( &in, 0, sizeof ( struct sockaddr_in ) );
			sock = WSSOCKET(AF_INET, SOCK_STREAM, 0);
			if (sock == INVALID_SOCKET) {
				Log_print("Unable to create socket!");
			}
			in.sin_family = AF_INET;
			in.sin_addr.s_addr = INADDR_ANY;
			in.sin_port = WSHTONS(23);
			nWSAerror = WSBIND(sock, (LPSOCKADDR)&in, sizeof(struct sockaddr_in));
			if (nWSAerror == SOCKET_ERROR) {
				Log_print("Unable to bind to socket!");
			}
			nWSAerror = WSLISTEN(sock, 5);
			if (nWSAerror == SOCKET_ERROR) {
				Log_print("Unable to listen to socket!");
			}
			retval = WSIOCTLSOCKET(sock, FIONBIO, (u_long FAR*)&argp);
			len = sizeof ( struct sockaddr_in );
			connected = 0;
			bufend = 0;
			Log_print("Socket is listening.");
		}

		newsock = WSACCEPT( sock, (struct sockaddr *)&peer_in, &len );
		if (newsock != -1) {
//			Log_print("Connected.");

			retval = WSIOCTLSOCKET(newsock, FIONBIO, (u_long FAR*)&argp);
			connected = 1;

			// add to log
#if 0	/* original code by Daniel */
			fd = fopen("c:\\Atari800Log.txt", "a");
			time(&lTime);
			datetime = localtime(&lTime);
			fprintf(fd, "Connected ");
			fprintf(fd, "%02u", datetime->tm_hour);
			fprintf(fd, ":");
			fprintf(fd, "%02u", datetime->tm_min);
			fprintf(fd, ":");
			fprintf(fd, "%02u", datetime->tm_sec);
			fprintf(fd, "  -  ");
			fprintf(fd, "%02u/", datetime->tm_mon + 1);
			fprintf(fd, "%02u/", datetime->tm_mday);
			fprintf(fd, "%02u  -  ", datetime->tm_year % 100);
			fprintf(fd, "From %s\r\n", _INET_NTOA(peer_in.sin_addr));
			fclose(fd);
#else	/* Piotr's code */
			time(&lTime);
			datetime = localtime(&lTime);
			Log_print("Connected %02u:%02u:%02u - %02u/%02u/%02u - From %s",
				datetime->tm_hour,
				datetime->tm_min,
				datetime->tm_sec,
				datetime->tm_mon + 1,
				datetime->tm_mday,
				datetime->tm_year % 100,
				WSINETNTOA(peer_in.sin_addr)
			);
#endif

			strcat(bufout,"\r\n_CONNECT 2400\r\n");
			bufend = 17;
			MEMORY_dPutByte(747,17);
			WSCLOSESOCKET(sock);
			return;
		}

	}
	if (concurrent == 1) {
		bytesread = WSRECV(newsock, &one, 1, 0);
		if (bytesread > 0) {
			if (one == 255) {
				while ((bytesread = WSRECV(newsock, &one, 1, 0))==0) {
				}
				if (one == 255) {
					bufend++;
					bufout[bufend] = one;
					MEMORY_dPutByte(747,bufend);
					CPU_regA = 1;
					CPU_regY = 1;
					CPU_ClrN;
					return;
				} else {
					while ((bytesread = WSRECV(newsock, &one, 1, 0))==0) {
					}

					CPU_regA = 1;
					CPU_regY = 1;
					CPU_ClrN;
					return;
				}
			} else {
				bufend++;
				bufout[bufend] = one;
				MEMORY_dPutByte(747,bufend);
				CPU_regA = 1;
				CPU_regY = 1;
				CPU_ClrN;
				return;
			}

		}
	} else {
		if (concurrent == 0 && connected == 1) {
			MEMORY_dPutByte(747,12);
			CPU_regA = 1;
			CPU_regY = 1;
			CPU_ClrN;
			return;
		}
	}
}

void Device_RHSPEC(void)
{
	r_in = MEMORY_dGetByte(Devices_ICCOMZ);
/*
	Log_print( "R: device special" );

	Log_print("Devices_ICCOMZ =");
	Log_print("%d",r_in);
	Log_print("^^ in Devices_ICCOMZ");
*/

	switch (r_in) {
	case 34:
		xio_34();
		break;
	case 36:
		xio_36();
		break;
	case 38:
		xio_38();
		break;
	case 40:
		xio_40();
		break;
	default:
		Log_print("Unsupported XIO #.");
		break;
	}
/*
	CPU_regA = 1;
	CPU_regY = 1;
	CPU_ClrN;
*/

}

/* Following code by Piotr Fusik.
   It adds R: device using new routines in Atari800 core.
   Advantages:
   - R: doesn't overwrite P:
   - no byte of Atari OS is changed
   - address of every R: escape is checked to make sure that it is a patch
     set by the emulator, not a CIM in an Atari program
   - no #ifdef WIN32 in Atari800 core routines
*/

static UWORD r_entry_address = 0;

/* call before Atari800_Frame() */
void RDevice_Frame(void)
{
	if (Devices_enable_r_patch)
		r_entry_address = Devices_UpdateHATABSEntry('R', r_entry_address, R_TABLE_ADDRESS);
}

/* call before or after ESC_UpdatePatches */
void RDevice_UpdatePatches(void)
{
	if (Devices_enable_r_patch) {		/* enable R: device */
		/* change memory attributex for the area, where we put
		   R: handler table and patches */
#ifndef PAGED_ATTRIB
		MEMORY_SetROM(R_DEVICE_BEGIN, R_DEVICE_END);
#else
#pragma message ("R: device not working yet")
#endif
		/* set handler table */
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_OPEN, R_PATCH_OPEN - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_CLOS, R_PATCH_CLOS - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_READ, R_PATCH_READ - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_WRIT, R_PATCH_WRIT - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_STAT, R_PATCH_STAT - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_SPEC, R_PATCH_SPEC - 1);
		/* set patches */
		ESC_AddEscRts(R_PATCH_OPEN, ESC_RHOPEN, Device_RHOPEN);
		ESC_AddEscRts(R_PATCH_CLOS, ESC_RHCLOS, Device_RHCLOS);
		ESC_AddEscRts(R_PATCH_READ, ESC_RHREAD, Device_RHREAD);
		ESC_AddEscRts(R_PATCH_WRIT, ESC_RHWRIT, Device_RHWRIT);
		ESC_AddEscRts(R_PATCH_STAT, ESC_RHSTAT, Device_RHSTAT);
		ESC_AddEscRts(R_PATCH_SPEC, ESC_RHSPEC, Device_RHSPEC);
		/* R: in HATABS will be added next frame by Devices_Frame */
	}
	else {	/* disable R: device */
		/* remove R: entry from HATABS */
		Devices_RemoveHATABSEntry('R', r_entry_address, R_TABLE_ADDRESS);
		/* remove patches */
		ESC_Remove(ESC_RHOPEN);
		ESC_Remove(ESC_RHCLOS);
		ESC_Remove(ESC_RHREAD);
		ESC_Remove(ESC_RHWRIT);
		ESC_Remove(ESC_RHSTAT);
		ESC_Remove(ESC_RHSPEC);
		/* fill memory area used for table and patches with 0xff */
		MEMORY_dFillMem(R_DEVICE_BEGIN, 0xff, R_DEVICE_END - R_DEVICE_BEGIN + 1);
	}
}

/* call in Atari_Exit() */
void RDevice_Exit()
{
    if (connected) {
         WSCLOSESOCKET(newsock);
         WSCLOSESOCKET(sock);
         connected = 0;
    }
}

/* Added by Tomek Szymankowski
   The best way to obtain the winsock library is to load it dynamically
*/

/*========================================================
Function : RDevice_LoadLibrary
=========================================================*/
/* #FN#
   Loads dynamically WS2_32.dll library */
int
/* #AS#
   0 if suceeded, otherwise a value greater than 0 */
RDevice_LoadLibrary(
	PHANDLE pHandle
)
{
	int nResult = 0;

	/* Load WS2_32 library */
	if( !s_hWinSock2 )
	{
		s_hWinSock2 = LoadLibrary( "WS2_32.DLL" );
		if( !s_hWinSock2 )
			nResult = WS_ERROR_LOAD;
		else
		{
			s_pfnCloseSocket   = GetProcAddress( s_hWinSock2, "closesocket"   );
			s_pfnIOCtlSocket   = GetProcAddress( s_hWinSock2, "ioctlsocket"   );
			s_pfnConnect       = GetProcAddress( s_hWinSock2, "connect"       );
			s_pfnHtons         = GetProcAddress( s_hWinSock2, "htons"         );
			s_pfnSocket        = GetProcAddress( s_hWinSock2, "socket"        );
			s_pfnGetHostByName = GetProcAddress( s_hWinSock2, "gethostbyname" );
			s_pfnWSAStartup    = GetProcAddress( s_hWinSock2, "WSAStartup"    );
			s_pfnSend          = GetProcAddress( s_hWinSock2, "send"          );
			s_pfnRecv          = GetProcAddress( s_hWinSock2, "recv"          );
			s_pfnInetNtoa      = GetProcAddress( s_hWinSock2, "inet_ntoa"     );
			s_pfnAccept        = GetProcAddress( s_hWinSock2, "accept"        );
			s_pfnListen        = GetProcAddress( s_hWinSock2, "listen"        );
			s_pfnBind          = GetProcAddress( s_hWinSock2, "bind"          );

			if( !s_pfnCloseSocket   ||
				!s_pfnIOCtlSocket   ||
				!s_pfnConnect       ||
				!s_pfnHtons         ||
				!s_pfnSocket        ||
				!s_pfnGetHostByName ||
				!s_pfnWSAStartup    ||
				!s_pfnSend          ||
				!s_pfnRecv          ||
				!s_pfnInetNtoa      ||
				!s_pfnAccept        ||
				!s_pfnListen        ||
				!s_pfnBind )
			{
				FreeLibrary( s_hWinSock2 );
				s_hWinSock2 = NULL;
				nResult = WS_ERROR_FUNC;
			}
		}
	}
	if( NULL != pHandle )
		pHandle = s_hWinSock2;

	return nResult;

} /* #OF# RDevice_LoadLibrary */

/*========================================================
Function : RDevice_FreeLibrary
=========================================================*/
/* #FN#
   Releases WS2_32.dll library */
void
/* #AS#
   Nothing */
RDevice_FreeLibrary( void )
{
	if( s_hWinSock2 )
	{
		FreeLibrary( s_hWinSock2 );

		s_hWinSock2        = NULL;
		s_pfnCloseSocket   = NULL;
		s_pfnIOCtlSocket   = NULL;
		s_pfnConnect       = NULL;
		s_pfnHtons         = NULL;
		s_pfnSocket        = NULL;
		s_pfnGetHostByName = NULL;
		s_pfnWSAStartup    = NULL;
		s_pfnSend          = NULL;
		s_pfnRecv          = NULL;
		s_pfnInetNtoa      = NULL;
		s_pfnAccept        = NULL;
		s_pfnListen        = NULL;
		s_pfnBind          = NULL;
	}
} /* #OF# RDevice_FreeLibrary */

/*========================================================
Function : RDevice_IsCapable
=========================================================*/
/* #FN#
   Checks if the WS2_32 library is available */
int
/* #AS#
   1 if WS2_32 is available, otherwise -1 */
RDevice_IsCapable( void )
{
	if( !s_hWinSock2 )
	{
//		Log_print( "Cannot emulate R: device without ws2_32.dll loaded properly." );
		return -1;
	}
	return 1;

} /* #OF# RDevice_IsCapable */

/*========================================================
Function : WSCLOSESOCKET
=========================================================*/
/* #FN#
   Closes an existing socket */
static
int
/* #AS#
   0 if successful, otherwise SOCKET_ERROR */
WSCLOSESOCKET(
	SOCKET nSocket
)
{
	return (int)s_pfnCloseSocket( nSocket );

} /* #OF# WSCLOSESOCKET */

/*========================================================
Function : WSIOCTLSOCKET
=========================================================*/
/* #FN#
   Controls the I/O mode of a socket */
static
int
/* #AS#
   0 if successful, otherwise SOCKET_ERROR */
WSIOCTLSOCKET(
	SOCKET      nSocket,
	long        nCmd,
	u_long FAR *pArgp
)
{
	return (int)s_pfnIOCtlSocket( nSocket, nCmd, pArgp );

} /* #OF# WSIOCTLSOCKET */

/*========================================================
Function : WSCONNECT
=========================================================*/
/* #FN#
   Establishes a connection to a specified socket */
static
int
/* #AS#
   0 if successful, otherwise SOCKET_ERROR */
WSCONNECT(
	SOCKET           nSocket,
	const LPSOCKADDR pszName,
	int              nNameLen
)
{
	return (int)s_pfnConnect( nSocket, pszName, nNameLen );

} /* #OF# WSCONNECT */

/*========================================================
Function : WSHTONS
=========================================================*/
/* #FN#
   Takes a 16-bit number in host byte order and returns a 16-bit number in
   network byte order used in TCP/IP networks */
static
u_short
/* #AS#
   Value in TCP/IP network byte order */
WSHTONS(
	u_short nHostShort
)
{
	return (u_short)s_pfnHtons( nHostShort );

} /* #OF# WSHTONS */

/*========================================================
Function : WSSOCKET
=========================================================*/
/* #FN#
   Creates a socket that is bound to a specific service provider */
static
SOCKET
/* #AS#
   Descriptor referencing the new socket if successful, otherwise INVALID_SOCKET */
WSSOCKET(
	int nAf,
	int nType,
	int nProtocol
)
{
	return (SOCKET)s_pfnSocket( nAf, nType, nProtocol );

} /* #OF# WSSOCKET */

/*========================================================
Function : WSGETHOSTBYNAME
=========================================================*/
/* #FN#
   Retrieves host information corresponding to a host name from a host database */
static
LPHOSTENT
/* #AS#
   Pointer to the HOSTENT structure if successful, otherwise NULL */
WSGETHOSTBYNAME(
	const char FAR *pszName
)
{
	return (LPHOSTENT)s_pfnGetHostByName( pszName );

} /* #OF# WSGETHOSTBYNAME */

/*========================================================
Function : WSWSASTARTUP
=========================================================*/
/* #FN#
   Initiates use of Ws2_32.dll by a process */
static
int
/* #AS#
   0 if successful, otherwise an error code */
WSWSASTARTUP(
	WORD      wVersionRequested,
	LPWSADATA pWSAData
)
{
	return (int)s_pfnWSAStartup( wVersionRequested, pWSAData );

} /* #OF# WSWSASTARTUP */

/*========================================================
Function : WSSEND
=========================================================*/
/* #FN#
   Sends data on a connected socket */
static
int
/* #AS#
   Total number of bytes sent if successful, otherwise SOCKET_ERROR */
WSSEND(
	SOCKET          nSocket,
	const char FAR *pszBuf,
	int             nLen,
	int             nFlags
)
{
	return (int)s_pfnSend( nSocket, pszBuf, nLen, nFlags );

} /* #OF# WSSEND */

/*========================================================
Function : WSRECV
=========================================================*/
/* #FN#
   Receives data from a connected socket */
static
int
/* #AS#
   Number of bytes received if successful, otherwise SOCKET_ERROR */
WSRECV(
	SOCKET    nSocket,
	char FAR *pszBuf,
	int       nLen,
	int       nFlags
)
{
	return (int)s_pfnRecv( nSocket, pszBuf, nLen, nFlags );

} /* #OF# WSRECV */

/*========================================================
Function : WSINETNTOA
=========================================================*/
/* #FN#
   Converts an (Ipv4) Internet network address into a string in Internet
   standard dotted format */
static
char FAR *
/* #AS#
   Pointer to a static buffer containing the text address if successful,
   otherwise NULL */
WSINETNTOA(
	IN_ADDR inAddr
)
{
	return (char FAR *)s_pfnInetNtoa( inAddr );

} /* #OF# WSINETNTOA */

/*========================================================
Function : WSACCEPT
=========================================================*/
/* #FN#
   Permits an incoming connection attempt on a socket */
static
SOCKET
/* #AS#
   Handle for the socket on which the actual connection is made if successful,
   otherwise INVALID_SOCKET */
WSACCEPT(
	SOCKET     nSocket,
	LPSOCKADDR pAddr,
	int FAR    *pAddrLen
)
{
	return (SOCKET)s_pfnAccept( nSocket, pAddr, pAddrLen );

} /* #OF# WSACCEPT */

/*========================================================
Function : WSLISTEN
=========================================================*/
/* #FN#
   Places a socket a state where it is listening for an incoming connection */
static
int
/* #AS#
   0 if successful, otherwise SOCKET_ERROR */
WSLISTEN(
	SOCKET nSocket,
	int    nBackLog
)
{
	return (int)s_pfnListen( nSocket, nBackLog );

} /* #OF# WSLISTEN */

/*========================================================
Function : WSBIND
=========================================================*/
/* #FN#
   Associates a local address with a socket */
static
int
/* #AS#
   0 if successful, otherwise SOCKET_ERROR */
WSBIND(
	SOCKET           nSocket,
	const LPSOCKADDR pszName,
	int              nNameLen
)
{
	return (int)s_pfnBind( nSocket, pszName, nNameLen );

} /* #OF# WSBIND */
