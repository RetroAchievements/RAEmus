/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#if defined(_USE_QT) || defined(_USE_SDL)
	#include <stdarg.h>
	#include <fcntl.h>
	#include <stdio.h>
	#include <iostream>
	#include <fstream>
	#include <cstdio>
	#if defined(_USE_QT)
		#include <sys/types.h>
		#include <sys/stat.h>
		#if !defined(Q_OS_WIN)
			#include <unistd.h>
		#endif
	#endif
#elif defined(_WIN32)
	#include <windows.h>
#endif
#include "fileio.h"

#ifdef USE_ZLIB
	#if defined(USE_QT)
		#include <zlib.h>
		#include <zconf.h>
	#else
		#ifdef _WIN32
			#define ZLIB_WINAPI
		#endif
		#include "zlib-1.2.11/zlib.h"
		#include "zlib-1.2.11/zconf.h"
	#endif
	#if defined(ZLIB_VERNUM) && (ZLIB_VERNUM < 0x1290)
		inline size_t gzfread(void *buffer, size_t size, size_t count, gzFile file)
		{
			uint8_t *p = (uint8_t *)buffer;
			int i = 0;
			for(i = 0; i < count; i++) {
				for(int j = 0; j < size; j++) {
					int s = gzgetc(file);
					if(s < 0) return 0; // EOF
					*p++ = (uint8_t)s; 
				}
			}
			return i + 1;
		}
		inline size_t gzfwrite(void *buffer, size_t size, size_t count, gzFile file)
		{
			uint8_t *p = (uint8_t *)buffer;
			int i = 0;
			for(i = 0; i < count; i++) {
				for(int j = 0; j < size; j++) {
					uint8_t n = *p++; 
					int s = gzputc(file, n);
					if(s < 0) return 0; // EOF
				}
			}
			return i + 1;
		}
	#endif
#endif

FILEIO::FILEIO()
{
#ifdef USE_ZLIB
	gz = NULL;
#endif
	fp = NULL;
	path[0] = _T('\0');
}

FILEIO::~FILEIO(void)
{
	Fclose();
}

bool FILEIO::IsFileExisting(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	FILE *f = fopen(file_path, "r");
	if(f != NULL) {
		fclose(f);
		return true;
	}
	return false;
#elif defined(_WIN32)
	DWORD attr = GetFileAttributes(file_path);
	if(attr == -1) {
		return false;
	}
	return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	return (_taccess(file_path, 0) == 0);
#endif
}

bool FILEIO::IsFileProtected(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	struct stat st;
	if(stat(file_path, &st) == 0) {
#if defined(_WIN32)
		if((st.st_mode & S_IWUSR) == 0) {
#else
		if((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
#endif
			return true;
		}
	}
	return false;
#elif defined(_WIN32)
	return ((GetFileAttributes(file_path) & FILE_ATTRIBUTE_READONLY) != 0);
#else
	return (_taccess(file_path, 2) != 0);
#endif
}

bool FILEIO::RemoveFile(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	return (remove(file_path) == 0);
#elif defined(_WIN32)
	return (DeleteFile(file_path) != 0);
#else
	return (_tremove(file_path) == 0);
#endif
}

bool FILEIO::RenameFile(const _TCHAR *existing_file_path, const _TCHAR *new_file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	return (rename(existing_file_path, new_file_path) == 0);
#elif defined(_WIN32)
	return (MoveFile(existing_file_path, new_file_path) != 0);
#else
	return (_trename(existing_file_path, new_file_path) == 0);
#endif
}

bool FILEIO::Fopen(const _TCHAR *file_path, int mode)
{
	Fclose();
	
	// store file path
	my_tcscpy_s(path, _MAX_PATH, file_path);
	open_mode = mode;
	
#ifdef USE_ZLIB
	if(check_file_extension(file_path, _T(".gz"))) {
		return Gzopen(file_path, mode);
	}
#endif
	switch(mode) {
	case FILEIO_READ_BINARY:
		return ((fp = _tfopen(file_path, _T("rb"))) != NULL);
	case FILEIO_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("wb"))) != NULL);
	case FILEIO_READ_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("r+b"))) != NULL);
	case FILEIO_READ_WRITE_NEW_BINARY:
		return ((fp = _tfopen(file_path, _T("w+b"))) != NULL);
	case FILEIO_READ_ASCII:
		return ((fp = _tfopen(file_path, _T("r"))) != NULL);
	case FILEIO_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("w"))) != NULL);
	case FILEIO_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a"))) != NULL);
	case FILEIO_READ_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("r+"))) != NULL);
	case FILEIO_READ_WRITE_NEW_ASCII:
		return ((fp = _tfopen(file_path, _T("w+"))) != NULL);
	case FILEIO_READ_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a+"))) != NULL);
	}
	return false;
}

#ifdef USE_ZLIB
bool FILEIO::Gzopen(const _TCHAR *file_path, int mode)
{
	gz_size = 0;
	
	// store file path
	my_tcscpy_s(path, _MAX_PATH, file_path);
	open_mode = mode;
	
	switch(mode) {
	case FILEIO_READ_BINARY:
//	case FILEIO_READ_WRITE_BINARY:
	case FILEIO_READ_ASCII:
//	case FILEIO_READ_WRITE_ASCII:
//	case FILEIO_READ_WRITE_APPEND_ASCII:
		if((fp = _tfopen(file_path, _T("rb"))) != NULL) {
			// check gzip header
			uint8_t data[10], name[_MAX_PATH] = {0};
			fread(data, 10, 1, fp);
			if(data[0] == 0x1f && data[1] == 0x8b && data[2] == 0x08) {
				if(data[3] & 2) {
					// skip part number
					fseek(fp, 2, SEEK_CUR);
				}
				if(data[3] & 4) {
					// skip extra field
					fread(data + 4, 2, 1, fp);
					fseek(fp, data[4] | (data[5] << 8), SEEK_CUR);
				}
				if(data[3] & 8) {
					// read original file name
					fread(name, sizeof(name), 1, fp);
					my_stprintf_s(path, _MAX_PATH, _T("%s%s"), get_parent_dir(path), (char *)name);
				}
				// get uncompressed input size
				fseek(fp, -4, SEEK_END);
				fread(data, 4, 1, fp);
				gz_size = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
			}
			fclose(fp);
			fp = NULL;
		}
		if(gz_size == 0) {
			return false;
		}
		break;
	}
	switch(mode) {
	case FILEIO_READ_BINARY:
		return ((gz = gzopen(tchar_to_char(file_path), "rb")) != NULL);
	case FILEIO_WRITE_BINARY:
		return ((gz = gzopen(tchar_to_char(file_path), "wb")) != NULL);
//	case FILEIO_READ_WRITE_BINARY:
//		return ((gz = gzopen(tchar_to_char(file_path), "r+b")) != NULL);
//	case FILEIO_READ_WRITE_NEW_BINARY:
//		return ((gz = gzopen(tchar_to_char(file_path), "w+b")) != NULL);
	case FILEIO_READ_ASCII:
		return ((gz = gzopen(tchar_to_char(file_path), "r")) != NULL);
	case FILEIO_WRITE_ASCII:
		return ((gz = gzopen(tchar_to_char(file_path), "w")) != NULL);
//	case FILEIO_WRITE_APPEND_ASCII:
//		return ((gz = gzopen(tchar_to_char(file_path), "a")) != NULL);
//	case FILEIO_READ_WRITE_ASCII:
//		return ((gz = gzopen(tchar_to_char(file_path), "r+")) != NULL);
//	case FILEIO_READ_WRITE_NEW_ASCII:
//		return ((gz = gzopen(tchar_to_char(file_path), "w+")) != NULL);
//	case FILEIO_READ_WRITE_APPEND_ASCII:
//		return ((gz = gzopen(tchar_to_char(file_path), "a+")) != NULL);
	}
	return false;
}
#endif

void FILEIO::Fclose()
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		gzclose(gz);
		gz = NULL;
	}
#endif
	if(fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	path[0] = _T('\0');
}

long FILEIO::FileLength()
{
	long pos = Ftell();
	Fseek(0, FILEIO_SEEK_END);
	long len = Ftell();
	Fseek(pos, FILEIO_SEEK_SET);
	return len;
}

#define GET_VALUE(type) \
	uint8_t buffer[sizeof(type)] = {0}; \
	Fread(buffer, sizeof(buffer), 1); \
	return *(type *)buffer

#define PUT_VALUE(type, v) \
	Fwrite(&v, sizeof(type), 1)

bool FILEIO::FgetBool()
{
	GET_VALUE(bool);
}

void FILEIO::FputBool(bool val)
{
	PUT_VALUE(bool, val);
}

uint8_t FILEIO::FgetUint8()
{
	GET_VALUE(uint8_t);
}

void FILEIO::FputUint8(uint8_t val)
{
	PUT_VALUE(uint8_t, val);
}

uint16_t FILEIO::FgetUint16()
{
	GET_VALUE(uint16_t);
}

void FILEIO::FputUint16(uint16_t val)
{
	PUT_VALUE(uint16_t, val);
}

uint32_t FILEIO::FgetUint32()
{
	GET_VALUE(uint32_t);
}

void FILEIO::FputUint32(uint32_t val)
{
	PUT_VALUE(uint32_t, val);
}

uint64_t FILEIO::FgetUint64()
{
	GET_VALUE(uint64_t);
}

void FILEIO::FputUint64(uint64_t val)
{
	PUT_VALUE(uint64_t, val);
}

int8_t FILEIO::FgetInt8()
{
	GET_VALUE(int8_t);
}

void FILEIO::FputInt8(int8_t val)
{
	PUT_VALUE(int8_t, val);
}

int16_t FILEIO::FgetInt16()
{
	GET_VALUE(int16_t);
}

void FILEIO::FputInt16(int16_t val)
{
	PUT_VALUE(int16_t, val);
}

int32_t FILEIO::FgetInt32()
{
	GET_VALUE(int32_t);
}

void FILEIO::FputInt32(int32_t val)
{
	PUT_VALUE(int32_t, val);
}

int64_t FILEIO::FgetInt64()
{
	GET_VALUE(int64_t);
}

void FILEIO::FputInt64(int64_t val)
{
	PUT_VALUE(int64_t, val);
}

float FILEIO::FgetFloat()
{
	GET_VALUE(float);
}

void FILEIO::FputFloat(float val)
{
	PUT_VALUE(float, val);
}

double FILEIO::FgetDouble()
{
	GET_VALUE(double);
}

void FILEIO::FputDouble(double val)
{
	PUT_VALUE(double, val);
}

uint16_t FILEIO::FgetUint16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.w;
}

void FILEIO::FputUint16_LE(uint16_t val)
{
	pair16_t tmp;
	tmp.w = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

uint32_t FILEIO::FgetUint32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.d;
}

void FILEIO::FputUint32_LE(uint32_t val)
{
	pair32_t tmp;
	tmp.d = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

uint64_t FILEIO::FgetUint64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.q;
}

void FILEIO::FputUint64_LE(uint64_t val)
{
	pair64_t tmp;
	tmp.q = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

int16_t FILEIO::FgetInt16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.sw;
}

void FILEIO::FputInt16_LE(int16_t val)
{
	pair16_t tmp;
	tmp.sw = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

int32_t FILEIO::FgetInt32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.sd;
}

void FILEIO::FputInt32_LE(int32_t val)
{
	pair32_t tmp;
	tmp.sd = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

int64_t FILEIO::FgetInt64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.sq;
}

void FILEIO::FputInt64_LE(int64_t val)
{
	pair64_t tmp;
	tmp.sq = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

float FILEIO::FgetFloat_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.f;
}

void FILEIO::FputFloat_LE(float val)
{
	pair32_t tmp;
	tmp.f = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

double FILEIO::FgetDouble_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.df;
}

void FILEIO::FputDouble_LE(double val)
{
	pair64_t tmp;
	tmp.df = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

_TCHAR FILEIO::FgetTCHAR_LE()
{
	switch(sizeof(_TCHAR)) {
	case 2: return (_TCHAR)FgetUint16_LE();
	case 4: return (_TCHAR)FgetUint32_LE();
	case 8: return (_TCHAR)FgetUint64_LE();
	}
	return (_TCHAR)FgetUint8();
}

void FILEIO::FputTCHAR_LE(_TCHAR val)
{
	switch(sizeof(_TCHAR)) {
	case 2: FputUint16_LE((uint16_t)val); return;
	case 4: FputUint32_LE((uint32_t)val); return;
	case 8: FputUint32_LE((uint64_t)val); return;
	}
	FputUint8((uint8_t )val);
}

uint16_t FILEIO::FgetUint16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.w;
}

void FILEIO::FputUint16_BE(uint16_t val)
{
	pair16_t tmp;
	tmp.w = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint32_t FILEIO::FgetUint32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.d;
}

void FILEIO::FputUint32_BE(uint32_t val)
{
	pair32_t tmp;
	tmp.d = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint64_t FILEIO::FgetUint64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.q;
}

void FILEIO::FputUint64_BE(uint64_t val)
{
	pair64_t tmp;
	tmp.q = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int16_t FILEIO::FgetInt16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.sw;
}

void FILEIO::FputInt16_BE(int16_t val)
{
	pair16_t tmp;
	tmp.sw = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int32_t FILEIO::FgetInt32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.sd;
}

void FILEIO::FputInt32_BE(int32_t val)
{
	pair32_t tmp;
	tmp.sd = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int64_t FILEIO::FgetInt64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.sq;
}

void FILEIO::FputInt64_BE(int64_t val)
{
	pair64_t tmp;
	tmp.sq = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

float FILEIO::FgetFloat_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.f;
}

void FILEIO::FputFloat_BE(float val)
{
	pair32_t tmp;
	tmp.f = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

double FILEIO::FgetDouble_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.df;
}

void FILEIO::FputDouble_BE(double val)
{
	pair64_t tmp;
	tmp.df = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

_TCHAR FILEIO::FgetTCHAR_BE()
{
	switch(sizeof(_TCHAR)) {
	case 2: return (_TCHAR)FgetUint16_BE();
	case 4: return (_TCHAR)FgetUint32_BE();
	case 8: return (_TCHAR)FgetUint64_BE();
	}
	return (_TCHAR)FgetUint8();
}

void FILEIO::FputTCHAR_BE(_TCHAR val)
{
	switch(sizeof(_TCHAR)) {
	case 2: FputUint16_BE((uint16_t)val); return;
	case 4: FputUint32_BE((uint32_t)val); return;
	case 8: FputUint32_BE((uint64_t)val); return;
	}
	FputUint8((uint8_t )val);
}

int FILEIO::Fgetc()
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzgetc(gz);
	} else
#endif
	if(fp != NULL) {
		return fgetc(fp);
	}
	return 0;
}

int FILEIO::Fputc(int c)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzputc(gz, c);
	} else
#endif
	if(fp != NULL) {
		return fputc(c, fp);
	}
	return 0;
}

char *FILEIO::Fgets(char *str, int n)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzgets(gz, str, n);
	} else
#endif
	if(fp != NULL) {
		return fgets(str, n, fp);
	}
	return 0;
}

_TCHAR *FILEIO::Fgetts(_TCHAR *str, int n)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
		char *str_mb = (char *)calloc(sizeof(char), n + 1);
		gzgets(gz, str_mb, n);
		my_swprintf_s(str, n, L"%s", char_to_wchar(str_mb));
		free(str_mb);
		return str;
#else
		return gzgets(gz, str, n);
#endif
	} else
#endif
	if(fp != NULL) {
		return _fgetts(str, n, fp);
	}
	return 0;
}

int FILEIO::Fprintf(const char* format, ...)
{
	va_list ap;
	char buffer[1024];
	
	va_start(ap, format);
	my_vsprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzprintf(gz, "%s", buffer);
	} else
#endif
	if(fp != NULL) {
		return my_fprintf_s(fp, "%s", buffer);
	}
	return 0;
}

int FILEIO::Ftprintf(const _TCHAR* format, ...)
{
	va_list ap;
	_TCHAR buffer[1024];
	
	va_start(ap, format);
	my_vstprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzprintf(gz, "%s", tchar_to_char(buffer));
	} else
#endif
	if(fp != NULL) {
		return my_ftprintf_s(fp, _T("%s"), buffer);
	}
	return 0;
}

size_t FILEIO::Fread(void* buffer, size_t size, size_t count)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzfread(buffer, size, count, gz);
	} else
#endif
	if(fp != NULL) {
		return fread(buffer, size, count, fp);
	}
	return 0;
}

size_t FILEIO::Fwrite(const void* buffer, size_t size, size_t count)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gzfwrite(buffer, size, count, gz);
	} else
#endif
	if(fp != NULL) {
		return fwrite(buffer, size, count, fp);
	}
	return 0;
}

int FILEIO::Fseek(long offset, int origin)
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		switch(origin) {
		case FILEIO_SEEK_CUR:
			return (gzseek(gz, offset, SEEK_CUR) == -1 ? -1 : 0);
		case FILEIO_SEEK_END:
			return (gzseek(gz, offset + gz_size, SEEK_SET) == -1 ? -1 : 0);
		case FILEIO_SEEK_SET:
			return (gzseek(gz, offset, SEEK_SET) == -1 ? -1 : 0);
		}
	} else
#endif
	if(fp != NULL) {
		switch(origin) {
		case FILEIO_SEEK_CUR:
			return fseek(fp, offset, SEEK_CUR);
		case FILEIO_SEEK_END:
			return fseek(fp, offset, SEEK_END);
		case FILEIO_SEEK_SET:
			return fseek(fp, offset, SEEK_SET);
		}
	}
	return -1;
}

long FILEIO::Ftell()
{
#ifdef USE_ZLIB
	if(gz != NULL) {
		return gztell(gz);
	} else
#endif
	if(fp != NULL) {
		return ftell(fp);
	}
	return 0;
}

bool FILEIO::StateCheckUint32(uint32_t val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		return (val == FgetUint32_LE());
	} else {
		FputUint32_LE(val);
		return true;
	}
}

bool FILEIO::StateCheckInt32(int32_t val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		return (val == FgetInt32_LE());
	} else {
		FputInt32_LE(val);
		return true;
	}
}

bool FILEIO::StateCheckBuffer(const _TCHAR *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		if(open_mode == FILEIO_READ_BINARY) {
			if(buffer[i] != FgetTCHAR_LE()) {
				return false;
			}
		} else {
			FputTCHAR_LE(buffer[i]);
		}
	}
	return true;
}

void FILEIO::StateValue(bool &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetBool();
	} else {
		FputBool(val);
	}
}

void FILEIO::StateValue(uint8_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetUint8();
	} else {
		FputUint8(val);
	}
}

void FILEIO::StateValue(uint16_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetUint16_LE();
	} else {
		FputUint16_LE(val);
	}
}

void FILEIO::StateValue(uint32_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetUint32_LE();
	} else {
		FputUint32_LE(val);
	}
}

void FILEIO::StateValue(uint64_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetUint64_LE();
	} else {
		FputUint64_LE(val);
	}
}

void FILEIO::StateValue(int8_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetInt8();
	} else {
		FputInt8(val);
	}
}

void FILEIO::StateValue(int16_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetInt16_LE();
	} else {
		FputInt16_LE(val);
	}
}

void FILEIO::StateValue(int32_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetInt32_LE();
	} else {
		FputInt32_LE(val);
	}
}

void FILEIO::StateValue(int64_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetInt64_LE();
	} else {
		FputInt64_LE(val);
	}
}

void FILEIO::StateValue(pair16_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val.w = FgetUint16_LE();
	} else {
		FputUint16_LE(val.w);
	}
}

void FILEIO::StateValue(pair32_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val.d = FgetUint32_LE();
	} else {
		FputUint32_LE(val.d);
	}
}

void FILEIO::StateValue(pair64_t &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val.q = FgetUint64_LE();
	} else {
		FputUint64_LE(val.q);
	}
}

void FILEIO::StateValue(float &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetFloat_LE();
	} else {
		FputFloat_LE(val);
	}
}

void FILEIO::StateValue(double &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetDouble_LE();
	} else {
		FputDouble_LE(val);
	}
}

void FILEIO::StateValue(_TCHAR &val)
{
	if(open_mode == FILEIO_READ_BINARY) {
		val = FgetTCHAR_LE();
	} else {
		FputTCHAR_LE(val);
	}
}

void FILEIO::StateArray(bool *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(uint8_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(uint16_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(uint32_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(uint64_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(int8_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(int16_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(int32_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(int64_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(pair16_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(pair32_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(pair64_t *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(float *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(double *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateArray(_TCHAR *buffer, size_t size, size_t count)
{
	for(unsigned int i = 0; i < size / sizeof(buffer[0]) * count; i++) {
		StateValue(buffer[i]);
	}
}

void FILEIO::StateBuffer(void *buffer, size_t size, size_t count)
{
	if(open_mode == FILEIO_READ_BINARY) {
		Fread(buffer, size, count);
	} else {
		Fwrite(buffer, size, count);
	}
}
