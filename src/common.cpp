/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.01.17-

	[ common ]
*/

#if defined(_USE_QT)
	#include <string.h>
	#include <fcntl.h>
	#if !defined(__WIN32) && !defined(__WIN64)
		#include <unistd.h>
	#else
		#include <io.h>
		#include <direct.h>
	#endif
	#include <sys/types.h>
	#include <sys/stat.h>
	#include "csp_logger.h"
	#include <string>
	#include <algorithm>
	#include <cctype>
	#include <QDir>
	#include <QFileInfo>
#elif defined(_WIN32)
	#include <shlwapi.h>
	#pragma comment(lib, "shlwapi.lib")
#else
	#include <time.h>
#endif
#include <math.h>
#include "common.h"
#include "fileio.h"

#if defined(__MINGW32__) || defined(__MINGW64__)
	extern DWORD GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);
#endif
#if defined(_USE_QT)
	std::string DLL_PREFIX cpp_homedir;
	std::string DLL_PREFIX my_procname;
	std::string DLL_PREFIX sRssDir;
#endif

void DLL_PREFIX common_initialize()
{
	// get the initial current path when the software starts
	get_initial_current_path();
}

uint32_t DLL_PREFIX EndianToLittle_DWORD(uint32_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianToLittle_WORD(uint16_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint32_t DLL_PREFIX EndianFromLittle_DWORD(uint32_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianFromLittle_WORD(uint16_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint32_t DLL_PREFIX EndianToBig_DWORD(uint32_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianToBig_WORD(uint16_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint32_t DLL_PREFIX EndianFromBig_DWORD(uint32_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianFromBig_WORD(uint16_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint64_t DLL_PREFIX ExchangeEndianU64(uint64_t x)
{
	pair64_t __i, __o;
	__i.q = x;
	__o.b.h7  = __i.b.l;
	__o.b.h6  = __i.b.h;
	__o.b.h5  = __i.b.h2;
	__o.b.h4  = __i.b.h3;
	__o.b.h3  = __i.b.h4;
	__o.b.h2  = __i.b.h5;
	__o.b.h   = __i.b.h6;
	__o.b.l   = __i.b.h7;
	return __o.q;
}

int64_t DLL_PREFIX ExchangeEndianS64(uint64_t x)
{
	pair64_t __i, __o;
	__i.q = x;
	__o.b.h7  = __i.b.l;
	__o.b.h6  = __i.b.h;
	__o.b.h5  = __i.b.h2;
	__o.b.h4  = __i.b.h3;
	__o.b.h3  = __i.b.h4;
	__o.b.h2  = __i.b.h5;
	__o.b.h   = __i.b.h6;
	__o.b.l   = __i.b.h7;
	return __o.sq;
}

uint32_t DLL_PREFIX ExchangeEndianU32(uint32_t x)
{
	pair32_t __i, __o;
	__i.d = x;
	__o.b.h3 = __i.b.l;
	__o.b.h2 = __i.b.h;
	__o.b.h  = __i.b.h2;
	__o.b.l  = __i.b.h3;
	return __o.d;
}

int32_t DLL_PREFIX ExchangeEndianS32(uint32_t x)
{
	pair32_t __i, __o;
	__i.d = x;
	__o.b.h3 = __i.b.l;
	__o.b.h2 = __i.b.h;
	__o.b.h  = __i.b.h2;
	__o.b.l  = __i.b.h3;
	return __o.sd;
}

uint16_t DLL_PREFIX ExchangeEndianU16(uint16_t x)
{
	pair16_t __i, __o;
	__i.w = x;
	__o.b.h = __i.b.l;
	__o.b.l  = __i.b.h;
	return __o.w;
}

int16_t DLL_PREFIX ExchangeEndianS16(uint16_t x)
{
	pair16_t __i, __o;
	__i.w = x;
	__o.b.h = __i.b.l;
	__o.b.l = __i.b.h;
	return __o.sw;
}

#ifndef _MSC_VER
int DLL_PREFIX max(int a, int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(unsigned int a, unsigned int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(unsigned int a, int b)
{
	if(b < 0) return a;
	if(a > (unsigned int)b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(int a, unsigned int b)
{
	if(a < 0) return b;
	if((unsigned int)a > b) {
		return a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(int a, int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(unsigned int a, int b)
{
	if(b < 0) return b;
	if(a > INT_MAX) return b;
	
	if((int)a < b) {
		return (int)a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(int a, unsigned int b)
{
	if(a < 0) return a;
	if(b > INT_MAX) return a;
	
	if(a < (int)b) {
		return a;
	} else {
		return (int)b;
	}
}

unsigned int DLL_PREFIX min(unsigned int a, unsigned int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}
#endif

#ifndef SUPPORT_SECURE_FUNCTIONS
//errno_t my_tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode)
//{
//	if((*pFile = _tfopen(filename, mode)) != NULL) {
//		return 0;
//	} else {
//		return errno;
//	}
//}

errno_t DLL_PREFIX my_tcscat_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	_tcscat(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	strcpy(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	_tcscpy(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count)
{
	strncpy(strDestination, strSource, count);
	return 0;
}

errno_t DLL_PREFIX my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count)
{
	_tcsncpy(strDestination, strSource, count);
	return 0;
}

char *DLL_PREFIX my_strtok_s(char *strToken, const char *strDelimit, char **context)
{
	return strtok(strToken, strDelimit);
}

_TCHAR *DLL_PREFIX my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context)
{
	return _tcstok(strToken, strDelimit);
}

int DLL_PREFIX my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_swprintf_s(wchar_t *buffer, size_t sizeOfBuffer, const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vswprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = _vstprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr)
{
	return vsprintf(buffer, format, argptr);
}

int DLL_PREFIX my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr)
{
	return _vstprintf(buffer, format, argptr);
}
#endif

#ifndef _MSC_VER
void DLL_PREFIX *my_memcpy(void *dst, void *src, size_t len)
{
	size_t len1;
	register size_t len2;
	register uint32_t s_align = (uint32_t)(((size_t)src) & 0x1f);
	register uint32_t d_align = (uint32_t)(((size_t)dst) & 0x1f);
	int i;
	
	if(len == 0) return dst;
	if(len < 8) {
		return memcpy(dst, src, len);
	}
	len1 = len;
	
#if defined(WITHOUT_UNALIGNED_SIMD)
	// Using SIMD without un-aligned instructions.
	switch(s_align) {
	case 0: // Align 256
		{
			uint64_t b64[4];
			register uint64_t *s64 = (uint64_t *)src;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0: // 256 vs 256
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b64[i] = s64[i];
						for(i = 0; i < 4; i++) d64[i] = b64[i];
						s64 += 4;
						d64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x10: // 256 vs 128
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b64[i] = s64[i];
						for(i = 0; i < 2; i++) d64[i] = b64[i];
						d64 += 2;
						for(i = 2; i < 4; i++) d64[i - 2] = b64[i];
						d64 += 2;
						s64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 256 vs 64
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b64[i] = s64[i];
						for(i = 0; i < 4; ++i) {
							*d64 = b64[i];
							++d64;
						}
						s64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: 
			case 0x14:
			case 0x1c: // 256 vs 32
				{
					uint32_t b32[8];
					register uint32_t *s32 = (uint32_t *)src;
					register uint32_t *d32 = (uint32_t *)dst;
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 8; ++i) b32[i] = s32[i];
						*d32 = b32[0];
						++d32;
						*d32 = b32[1];
						++d32;
						*d32 = b32[2];
						++d32;
						*d32 = b32[3];
						++d32;
						*d32 = b32[4];
						++d32;
						*d32 = b32[5];
						++d32;
						*d32 = b32[6];
						++d32;
						*d32 = b32[7];
						++d32;
						s32 += 8;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	case 0x10: // Src alignn to 16.
		{
			uint32_t b32[4];
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			switch(d_align) {
			case 0: // 128 vs 256/128
			case 0x10:
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b32[i] = s32[i];
						for(i = 0; i < 4; i++) d32[i] = b32[i];
						s32 += 4;
						d32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 128 vs 64
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b32[i] = s32[i];
						for(i = 0; i < 2; ++i) {
							d32[i] = b32[i];
						}
						d32 += 2;
						for(i = 2; i < 4; ++i) {
							d32[i - 2] = b32[i];
						}
						d32 += 2;
						s32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c:
			case 0x14:
			case 0x1c: // 128 vs 32
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b32[i] = s32[i];
						*d32 = b32[0];
						++d32;
						*d32 = b32[1];
						++d32;
						*d32 = b32[2];
						++d32;
						*d32 = b32[3];
						++d32;
						s32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	case 0x08:
	case 0x18: // Src alignn to 64.
		{
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			register uint64_t *s64 = (uint64_t *)src;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0:
			case 0x10: // 64 vs 128
				{
					uint64_t b128[2];
					len2 = len1 >> 4;
					while(len2 > 0) {
						b128[0] = *s64;
						++s64;
						b128[1] = *s64;
						++s64;
						for(i = 0; i < 2; i++) d64[i] = b128[i];
						d64 += 2;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 64 vs 64
				{
					len2 = len1 >> 3;
					while(len2 > 0) {
						*d64 = *s64;
						++s64;
						++d64;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: // 64 vs 32
			case 0x14:
			case 0x1c: // 64 vs 32
				{
					uint32_t b32[2];
					len2 = len1 >> 3;
					while(len2 > 0) {
						for(i = 0; i < 2; ++i) b32[i] = s32[i];
						d32[0] = b32[0];
						d32[1] = b32[1];
						s32 += 2;
						d32 += 2;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
	case 0x04:
	case 0x0c:
	case 0x14:
	case 0x1c:  // Src align 32
		{
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0:
			case 0x10: // 32 vs 128
				{
					uint32_t b128[4];
					len2 = len1 >> 4;
					while(len2 > 0) {
						b128[0] = *s32;
						++s32;
						b128[1] = *s32;
						++s32;
						b128[3] = *s32;
						++s32;
						b128[4] = *s32;
						++s32;
						for(i = 0; i < 4; i++) d32[i] = b128[i];
						d32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 32 vs 64
				{
					uint32_t b64[2];
					len2 = len1 >> 3;
					while(len2 > 0) {
						b64[0] = *s32;
						++s32;
						b64[1] = *s32;
						++s32;

						for(i = 0; i < 2; i++) d32[i] = b64[i];
						d32 += 2;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: 
			case 0x14:
			case 0x1c: // 32 vs 32
				{
					len2 = len1 >> 2;
					while(len2 > 0) {
						*d32 = *s32;
						++s32;
						++d32;
						--len2;
					}
					len1 = len1 & 0x03;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	default:
		if(len1 != 0) return memcpy(dst, src, len1);
		break;
	}
#else
	// Using SIMD *with* un-aligned instructions.
	register uint32_t *s32 = (uint32_t *)src;
	register uint32_t *d32 = (uint32_t *)dst;
	if(((s_align & 0x07) != 0x0) && ((d_align & 0x07) != 0x0)) { // None align.
		return memcpy(dst, src, len);
	}
	if((s_align == 0x0) || (d_align == 0x0)) { // Align to 256bit
		uint32_t b256[8];
		len2 = len1 >> 5;
		while(len2 > 0) {
			for(i = 0; i < 8; i++) b256[i] = s32[i];
			for(i = 0; i < 8; i++) d32[i] = b256[i];
			s32 += 8;
			d32 += 8;
			--len2;
		}
		len1 = len1 & 0x1f;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}
	if(((s_align & 0x0f) == 0x0) || ((d_align & 0x0f) == 0x0)) { // Align to 128bit
		uint32_t b128[4];
		len2 = len1 >> 4;
		while(len2 > 0) {
			for(i = 0; i < 4; i++) b128[i] = s32[i];
			for(i = 0; i < 4; i++) d32[i] = b128[i];
			s32 += 4;
			d32 += 4;
			--len2;
		}
		len1 = len1 & 0x0f;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}		
	if(((s_align & 0x07) == 0x0) || ((d_align & 0x07) == 0x0)) { // Align to 64bit
		uint32_t b64[2];
		len2 = len1 >> 3;
		while(len2 > 0) {
			for(i = 0; i < 2; i++) b64[i] = s32[i];
			for(i = 0; i < 2; i++) d32[i] = b64[i];
			s32 += 2;
			d32 += 2;
			--len2;
		}
		len1 = len1 & 0x07;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}		
	//if(len1 != 0) return memcpy(dst, src, len1);
#endif
	// Trap
	return dst;
}
#endif

#ifndef _WIN32
BOOL DLL_PREFIX MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	BOOL result = FALSE;
	FILEIO* fio_i = new FILEIO();
	if(fio_i->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		char tmp_path[_MAX_PATH];
		my_sprintf_s(tmp_path, _MAX_PATH, "%s.$$$", lpFileName);
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(tmp_path, FILEIO_WRITE_ASCII)) {
			bool in_section = false;
			char section[1024], line[1024], *equal;
			my_sprintf_s(section, 1024, "[%s]", lpAppName);
			while(fio_i->Fgets(line, 1024) != NULL && strlen(line) > 0) {
				if(line[strlen(line) - 1] == '\n') {
					line[strlen(line) - 1] = '\0';
				}
				if(!result) {
					if(line[0] == '[') {
						if(in_section) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
						} else if(strcmp(line, section) == 0) {
							in_section = true;
						}
					} else if(in_section && (equal = strstr(line, "=")) != NULL) {
						*equal = '\0';
						if(strcmp(line, lpKeyName) == 0) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
							continue;
						}
						*equal = '=';
					}
				}
				fio_o->Fprintf("%s\n", line);
			}
			if(!result) {
				if(!in_section) {
					fio_o->Fprintf("[%s]\n", lpAppName);
				}
				fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
				result = TRUE;
			}
			fio_o->Fclose();
		}
		delete fio_o;
		fio_i->Fclose();
		if(result) {
			if(!(FILEIO::RemoveFile(lpFileName) && FILEIO::RenameFile(tmp_path, lpFileName))) {
				result = FALSE;
			}
		}
	} else {
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(lpFileName, FILEIO_WRITE_ASCII)) {
			fio_o->Fprintf("[%s]\n", lpAppName);
			fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
			fio_o->Fclose();
		}
		delete fio_o;
	}
	delete fio_i;
	return result;
}

DWORD DLL_PREFIX MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	_TCHAR *lpp = (_TCHAR *)lpReturnedString;
	if(lpDefault != NULL) {
		my_strcpy_s(lpp, nSize, lpDefault);
	} else {
		lpp[0] = '\0';
	}
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		bool in_section = false;
		char section[1024], line[1024], *equal;
		my_sprintf_s(section, 1024, "[%s]", lpAppName);
		while(fio->Fgets(line, 1024) != NULL && strlen(line) > 0) {
			if(line[strlen(line) - 1] == '\n') {
				line[strlen(line) - 1] = '\0';
			}
			if(line[0] == '[') {
				if(in_section) {
					break;
				} else if(strcmp(line, section) == 0) {
					in_section = true;
				}
			} else if(in_section && (equal = strstr(line, "=")) != NULL) {
				*equal = '\0';
				if(strcmp(line, lpKeyName) == 0) {
					my_strcpy_s(lpp, nSize, equal + 1);
					break;
				}
			}
		}
		fio->Fclose();
	}
	delete fio;
	return strlen(lpp);
}

UINT DLL_PREFIX MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	int i;
	char sstr[128];
	char sval[128];
	std::string s;
	memset(sstr, 0x00, sizeof(sstr));
	memset(sval, 0x00, sizeof(sval));
	snprintf(sval, 128, "%d", nDefault); 
	MyGetPrivateProfileString(lpAppName,lpKeyName, sval, sstr, 128, lpFileName);
	s = sstr;
	
	if(s.empty()) {
		i = nDefault;
	} else {
		i = strtol(s.c_str(), NULL, 10);
	}
	return i;
}
#endif

#if defined(_RGB555)
scrntype_t DLL_PREFIX RGB_COLOR(uint32_t r, uint32_t g, uint32_t b)
{
	scrntype_t rr = ((scrntype_t)r * 0x1f) / 0xff;
	scrntype_t gg = ((scrntype_t)g * 0x1f) / 0xff;
	scrntype_t bb = ((scrntype_t)b * 0x1f) / 0xff;
	return (rr << 10) | (gg << 5) | bb;
}

scrntype_t DLL_PREFIX RGBA_COLOR(uint32_t r, uint32_t g, uint b, uint32_t a)
{
	return RGB_COLOR(r, g, b);
}

uint8_t DLL_PREFIX R_OF_COLOR(scrntype_t c)
{
	c = (c >> 10) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX G_OF_COLOR(scrntype_t c)
{
	c = (c >>  5) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX B_OF_COLOR(scrntype_t c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX A_OF_COLOR(scrntype_t c)
{
	return 0;
}
#elif defined(_RGB565)
scrntype_t DLL_PREFIX RGB_COLOR(uint32_t r, uint32_t g, uint32_t b)
{
	scrntype_t rr = ((scrntype_t)r * 0x1f) / 0xff;
	scrntype_t gg = ((scrntype_t)g * 0x3f) / 0xff;
	scrntype_t bb = ((scrntype_t)b * 0x1f) / 0xff;
	return (rr << 11) | (gg << 5) | bb;
}

scrntype_t DLL_PREFIX RGBA_COLOR(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	return RGB_COLOR(r, g, b);
}

uint8_t DLL_PREFIX R_OF_COLOR(scrntype_t c)
{
	c = (c >> 11) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX G_OF_COLOR(scrntype_t c)
{
	c = (c >>  5) & 0x3f;
	c = (c * 0xff) / 0x3f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX B_OF_COLOR(scrntype_t c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX A_OF_COLOR(scrntype_t c)
{
	return 0;
}
#endif

#ifndef _MSC_VER
struct to_upper {  // Refer from documentation of libstdc++, GCC5.
	char operator() (char c) const { return std::toupper(c); }
};
#endif

#if defined(_USE_QT)
static void _my_mkdir(std::string t_dir)
{
	struct stat st;
//#if !defined(__WIN32) && !defined(__WIN64)
//	if(fstatat(AT_FDCWD, csppath.c_str(), &st, 0) != 0) {
//		mkdirat(AT_FDCWD, t_dir.c_str(), 0700); // Not found
//	}
#if defined(_USE_QT)
	if(stat(t_dir.c_str(), &st) != 0) {
		QDir dir = QDir::current();
		dir.mkdir(QString::fromStdString(t_dir));
		//dir.mkpath(QString::fromUtf8(app_path));
	}
#else
	if(stat(csppath.c_str(), &st) != 0) {
		_mkdir(t_dir.c_str()); // Not found
	}
#endif
}
#endif

const _TCHAR *DLL_PREFIX get_application_path()
{
	static _TCHAR app_path[_MAX_PATH];
	static bool initialized = false;
	
	if(!initialized) {
#if defined(_WIN32) && !defined(_USE_QT)
		_TCHAR tmp_path[_MAX_PATH], *ptr = NULL;
		if(GetModuleFileName(NULL, tmp_path, _MAX_PATH) != 0 && GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr) != 0 && ptr != NULL) {
			*ptr = _T('\0');
		} else {
			my_tcscpy_s(app_path, _MAX_PATH, _T(".\\"));
		}
#else
#if defined(Q_OS_WIN)
		std::string delim = "\\";
#else
		std::string delim = "/";
#endif
		std::string csppath = cpp_homedir + "CommonSourceCodeProject" + delim ;
		_my_mkdir(csppath);
		
		std::string cpath = csppath + my_procname + delim;
		_my_mkdir(cpath);
		strncpy(app_path, cpath.c_str(), _MAX_PATH - 1);
#endif
		initialized = true;
	}
	return (const _TCHAR *)app_path;
}

const _TCHAR *DLL_PREFIX get_initial_current_path()
{
	static _TCHAR current_path[_MAX_PATH];
	static bool initialized = false;
	
	if(!initialized) {
#if defined(_WIN32) && !defined(_USE_QT)
		GetCurrentDirectoryA(_MAX_PATH, current_path);
#else
		getcwd(current_path, _MAX_PATH);
#endif
		int len = strlen(current_path);
		if(current_path[len - 1] != '\\' && current_path[len - 1] != '/') {
#if defined(_WIN32) || defined(Q_OS_WIN)
			current_path[len] = '\\';
#else
			current_path[len] = '/';
#endif
			current_path[len + 1] = '\0';
		}

		initialized = true;
	}
	return (const _TCHAR *)current_path;
}

const _TCHAR *DLL_PREFIX create_local_path(const _TCHAR *format, ...)
{
	static _TCHAR file_path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	_TCHAR file_name[_MAX_PATH];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path[output_index], _MAX_PATH, _T("%s%s"), get_application_path(), file_name);
	return (const _TCHAR *)file_path[output_index];
}

void DLL_PREFIX create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...)
{
	_TCHAR file_name[_MAX_PATH];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path, length, _T("%s%s"), get_application_path(), file_name);
}

bool DLL_PREFIX is_absolute_path(const _TCHAR *file_path)
{
#ifdef _WIN32
	if(_tcslen(file_path) > 2 && ((file_path[0] >= _T('A') && file_path[0] <= _T('Z')) || (file_path[0] >= _T('a') && file_path[0] <= _T('z'))) && file_path[1] == _T(':')) {
		return true;
	}
#endif
	return (_tcslen(file_path) > 1 && (file_path[0] == _T('/') || file_path[0] == _T('\\')));
}

const _TCHAR *DLL_PREFIX create_date_file_path(const _TCHAR *extension)
{
	cur_time_t cur_time;
	
	get_host_time(&cur_time);
	return create_local_path(_T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
}

void DLL_PREFIX create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_path(extension));
}

const _TCHAR *DLL_PREFIX create_date_file_name(const _TCHAR *extension)
{
	static _TCHAR file_name[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	cur_time_t cur_time;
	
	get_host_time(&cur_time);
	my_stprintf_s(file_name[output_index], _MAX_PATH, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
	return (const _TCHAR *)file_name[output_index];
}

void DLL_PREFIX create_date_file_name(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_name(extension));
}

bool DLL_PREFIX check_file_extension(const _TCHAR *file_path, const _TCHAR *ext)
{
#if defined(_USE_QT)
	std::string s_fpath = file_path;
	std::string s_ext = ext;
	int pos;
	std::transform(s_fpath.begin(), s_fpath.end(), s_fpath.begin(), to_upper());
	std::transform(s_ext.begin(), s_ext.end(), s_ext.begin(), to_upper());
	if(s_fpath.length() < s_ext.length()) return false;
	pos = s_fpath.rfind(s_ext.c_str(), s_fpath.length());
	if((pos != (int)std::string::npos) && (pos >= ((int)s_fpath.length() - (int)s_ext.length()))) return true; 
	return false;
#else
	int nam_len = _tcslen(file_path);
	int ext_len = _tcslen(ext);
	
	return (nam_len >= ext_len && _tcsncicmp(&file_path[nam_len - ext_len], ext, ext_len) == 0);
#endif
}

const _TCHAR *DLL_PREFIX get_file_path_without_extensiton(const _TCHAR *file_path)
{
	static _TCHAR path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	my_tcscpy_s(path[output_index], _MAX_PATH, file_path);
#if defined(_WIN32) && defined(_MSC_VER)
	PathRemoveExtension(path[output_index]);
#elif defined(_USE_QT)
	QString delim;
	delim = QString::fromUtf8(".");
	QString tmp_path = QString::fromUtf8(file_path);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toUtf8().constData(), _MAX_PATH - 1);
#else
	_TCHAR *p = _tcsrchr(path[output_index], _T('.'));
	if(p != NULL) {
		*p = _T('\0');
	}
#endif
	return (const _TCHAR *)path[output_index];
}

void DLL_PREFIX get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len)
{
#ifdef _WIN32
	_TCHAR tmp[_MAX_PATH];
	if(GetFullPathName(src, _MAX_PATH, tmp, NULL) == 0) {
		my_tcscpy_s(dst, dst_len, src);
	} else if(GetLongPathName(tmp, dst, _MAX_PATH) == 0) {
		my_tcscpy_s(dst, dst_len, tmp);
	}
#elif defined(_USE_QT)
	QString tmp_path = QString::fromUtf8(src);
	QFileInfo info(tmp_path);
	my_tcscpy_s(dst, dst_len, info.absoluteFilePath().toLocal8Bit().constData());
#else
	// write code for your environment
#endif
}

const _TCHAR *DLL_PREFIX get_parent_dir(const _TCHAR* file)
{
	static _TCHAR path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
#ifdef _WIN32
	_TCHAR *ptr;
	GetFullPathName(file, _MAX_PATH, path[output_index], &ptr);
	if(ptr != NULL) {
		*ptr = _T('\0');
	}
#elif defined(_USE_QT)
	QString delim;
#if defined(Q_OS_WIN)
	delim = QString::fromUtf8("\\");
#else
	delim = QString::fromUtf8("/");
#endif
	QString tmp_path = QString::fromUtf8(file);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
		tmp_path.append(delim);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toUtf8().constData(), _MAX_PATH - 1);
#else
	// write code for your environment
#endif
	return path[output_index];
}

const wchar_t *DLL_PREFIX char_to_wchar(const char *cs)
{
	// char to wchar_t
	static wchar_t ws[4096];
	
#if defined(_WIN32) || defined(_USE_QT)
	mbstowcs(ws, cs, strlen(cs));
#else
	// write code for your environment
#endif
	return ws;
}

const char *DLL_PREFIX wchar_to_char(const wchar_t *ws)
{
	// wchar_t to char
	static char cs[4096];
	
#if defined(_WIN32) || defined(_USE_QT)
	wcstombs(cs, ws, wcslen(ws));
#else
	// write code for your environment
#endif
	return cs;
}

const _TCHAR *DLL_PREFIX char_to_tchar(const char *cs)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// char to wchar_t
	return char_to_wchar(cs);
#else
	// char to char
	return cs;
#endif
}

const char *DLL_PREFIX tchar_to_char(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to char
	return wchar_to_char(ts);
#else
	// char to char
	return ts;
#endif
}

const _TCHAR *DLL_PREFIX wchar_to_tchar(const wchar_t *ws)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ws;
#else
	// wchar_t to char
	return wchar_to_char(ws);
#endif
}

const wchar_t *DLL_PREFIX tchar_to_wchar(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ts;
#else
	// char to wchar_t
	return char_to_wchar(ts);
#endif
}

const _TCHAR *DLL_PREFIX create_string(const _TCHAR* format, ...)
{
	static _TCHAR buffer[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(buffer[output_index], 1024, format, ap);
	va_end(ap);
	return (const _TCHAR *)buffer[output_index];
}

int32_t DLL_PREFIX muldiv_s32(int32_t nNumber, int32_t nNumerator, int32_t nDenominator)
{
	try {
		int64_t tmp;
		tmp  = (int64_t)nNumber;
		tmp *= (int64_t)nNumerator;
		tmp /= (int64_t)nDenominator;
		return (int32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		if(tmp < 0) {
			return (int32_t)(tmp - 0.5);
		} else {
			return (int32_t)(tmp + 0.5);
		}
	}
}

uint32_t DLL_PREFIX muldiv_u32(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator)
{
	try {
		uint64_t tmp;
		tmp  = (uint64_t)nNumber;
		tmp *= (uint64_t)nNumerator;
		tmp /= (uint64_t)nDenominator;
		return (uint32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		return (uint32_t)(tmp + 0.5);
	}
}

uint32_t DLL_PREFIX get_crc32(uint8_t data[], int size)
{
	static bool initialized = false;
	static uint32_t table[256];
	
	if(!initialized) {
		for(int i = 0; i < 256; i++) {
			uint32_t c = i;
			for(int j = 0; j < 8; j++) {
				if(c & 1) {
					c = (c >> 1) ^ 0xedb88320;
				} else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
		initialized = true;
	}
	
	uint32_t c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

uint16_t DLL_PREFIX jis_to_sjis(uint16_t jis)
{
	pair32_t tmp;
	
	tmp.w.l = jis - 0x2121;
	if(tmp.w.l & 0x100) {
		tmp.w.l += 0x9e;
	} else {
		tmp.w.l += 0x40;
	}
	if(tmp.b.l > 0x7f) {
		tmp.w.l += 0x01;
	}
	tmp.b.h = (tmp.b.h >> 1) + 0x81;
	if(tmp.w.l >= 0xa000) {
		tmp.w.l += 0x4000;
	}
	return tmp.w.l;
}

int DLL_PREFIX decibel_to_volume(int decibel)
{
	// +1 equals +0.5dB (same as fmgen)
	return (int)(1024.0 * pow(10.0, decibel / 40.0) + 0.5);
}

int32_t DLL_PREFIX apply_volume(int32_t sample, int volume)
{
//	int64_t output;
	int32_t output;
	if(sample < 0) {
		output = -sample;
		output *= volume;
		output >>= 10;
		output = -output;
	} else {
		output = sample;
		output *= volume;
		output >>= 10;
	}
//	if(output > 2147483647) {
//		return 2147483647;
//	} else if(output < (-2147483647 - 1)) {
//		return (-2147483647 - 1);
//	} else {
//		return (int32_t)output;
//	}
	return output;
}

void DLL_PREFIX get_host_time(cur_time_t* cur_time)
{
#ifdef _WIN32
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	cur_time->year = sTime.wYear;
	cur_time->month = sTime.wMonth;
	cur_time->day = sTime.wDay;
	cur_time->day_of_week = sTime.wDayOfWeek;
	cur_time->hour = sTime.wHour;
	cur_time->minute = sTime.wMinute;
	cur_time->second = sTime.wSecond;
#else
	time_t timer = time(NULL);
	struct tm *local = localtime(&timer);
	cur_time->year = local->tm_year + 1900;
	cur_time->month = local->tm_mon + 1;
	cur_time->day = local->tm_mday;
	cur_time->day_of_week = local->tm_wday;
	cur_time->hour = local->tm_hour;
	cur_time->minute = local->tm_min;
	cur_time->second = local->tm_sec;
#endif
}

void DLL_PREFIX cur_time_t::increment()
{
	if(++second >= 60) {
		second = 0;
		if(++minute >= 60) {
			minute = 0;
			if(++hour >= 24) {
				hour = 0;
				// days in this month
				int days = 31;
				if(month == 2) {
					days = LEAP_YEAR(year) ? 29 : 28;
				} else if(month == 4 || month == 6 || month == 9 || month == 11) {
					days = 30;
				}
				if(++day > days) {
					day = 1;
					if(++month > 12) {
						month = 1;
						year++;
					}
				}
				if(++day_of_week >= 7) {
					day_of_week = 0;
				}
			}
		}
	}
}

void DLL_PREFIX cur_time_t::update_year()
{
	// 1970-2069
	if(year < 70) {
		year += 2000;
	} else if(year < 100) {
		year += 1900;
	}
}

void DLL_PREFIX cur_time_t::update_day_of_week()
{
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int y = year - (month < 3);
	day_of_week = (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
}

#define STATE_VERSION	1

bool DLL_PREFIX cur_time_t::process_state(void *f, bool loading)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(year);
	state_fio->StateValue(month);
	state_fio->StateValue(day);
	state_fio->StateValue(day_of_week);
	state_fio->StateValue(hour);
	state_fio->StateValue(minute);
	state_fio->StateValue(second);
	state_fio->StateValue(initialized);
	return true;
}

const _TCHAR *DLL_PREFIX get_symbol(symbol_t *first_symbol, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
		}
	}
	return NULL;
}

const _TCHAR *DLL_PREFIX get_value_or_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
		}
	}
	my_stprintf_s(name[output_index], 1024, format, addr);
	return name[output_index];
}

const _TCHAR *DLL_PREFIX get_value_and_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	my_stprintf_s(name[output_index], 1024, format, addr);
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				_TCHAR temp[1024];
//				my_stprintf_s(temp, 1024, _T(" (%s)"), symbol->name);
				my_stprintf_s(temp, 1024, _T(";%s"), symbol->name);
				my_tcscat_s(name[output_index], 1024, temp);
				return name[output_index];
			}
		}
	}
	return name[output_index];
}
