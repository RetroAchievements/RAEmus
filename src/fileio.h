/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <stdio.h>
#include "common.h"

#define FILEIO_READ_BINARY		1
#define FILEIO_WRITE_BINARY		2
#define FILEIO_READ_WRITE_BINARY	3
#define FILEIO_READ_WRITE_NEW_BINARY	4
#define FILEIO_READ_ASCII		5
#define FILEIO_WRITE_ASCII		6
#define FILEIO_READ_WRITE_ASCII		7
#define FILEIO_READ_WRITE_NEW_ASCII	8
#define FILEIO_WRITE_APPEND_ASCII	9
#define FILEIO_READ_WRITE_APPEND_ASCII	10
#define FILEIO_SEEK_SET			0
#define FILEIO_SEEK_CUR			1
#define FILEIO_SEEK_END			2

#ifdef USE_ZLIB
struct gzFile_s;
typedef struct gzFile_s *gzFile;
#endif

class DLL_PREFIX FILEIO
{
private:
#ifdef USE_ZLIB
	gzFile gz;
	long gz_size;
#endif
	FILE* fp;
	_TCHAR path[_MAX_PATH];
	int open_mode;
	
public:
	FILEIO();
	~FILEIO();
	
	static bool IsFileExisting(const _TCHAR *file_path);
	static bool IsFileProtected(const _TCHAR *file_path);
	static bool RemoveFile(const _TCHAR *file_path);
	static bool RenameFile(const _TCHAR *existing_file_path, const _TCHAR *new_file_path);
	
	bool Fopen(const _TCHAR *file_path, int mode);
#ifdef USE_ZLIB
	bool Gzopen(const _TCHAR *file_path, int mode);
#endif
	void Fclose();
	bool IsOpened()
	{
#ifdef USE_ZLIB
		if(gz != NULL) {
			return true;
		} else
#endif
		return (fp != NULL);
	}
	const _TCHAR *FilePath()
	{
		return path;
	}
	long FileLength();
	
	bool FgetBool();
	void FputBool(bool val);
	uint8_t FgetUint8();
	void FputUint8(uint8_t val);
	uint16_t FgetUint16();
	void FputUint16(uint16_t val);
	uint32_t FgetUint32();
	void FputUint32(uint32_t val);
	uint64_t FgetUint64();
	void FputUint64(uint64_t val);
	int8_t FgetInt8();
	void FputInt8(int8_t val);
	int16_t FgetInt16();
	void FputInt16(int16_t val);
	int32_t FgetInt32();
	void FputInt32(int32_t val);
	int64_t FgetInt64();
	void FputInt64(int64_t val);
	float FgetFloat();
	void FputFloat(float val);
	double FgetDouble();
	void FputDouble(double val);
	
	uint16_t FgetUint16_LE();
	void FputUint16_LE(uint16_t val);
	uint32_t FgetUint32_LE();
	void FputUint32_LE(uint32_t val);
	uint64_t FgetUint64_LE();
	void FputUint64_LE(uint64_t val);
	int16_t FgetInt16_LE();
	void FputInt16_LE(int16_t val);
	int32_t FgetInt32_LE();
	void FputInt32_LE(int32_t val);
	int64_t FgetInt64_LE();
	void FputInt64_LE(int64_t val);
	float FgetFloat_LE();
	void FputFloat_LE(float val);
	double FgetDouble_LE();
	void FputDouble_LE(double val);
	_TCHAR FgetTCHAR_LE();
	void FputTCHAR_LE(_TCHAR val);
	
	uint16_t FgetUint16_BE();
	void FputUint16_BE(uint16_t val);
	uint32_t FgetUint32_BE();
	void FputUint32_BE(uint32_t val);
	uint64_t FgetUint64_BE();
	void FputUint64_BE(uint64_t val);
	int16_t FgetInt16_BE();
	void FputInt16_BE(int16_t val);
	int32_t FgetInt32_BE();
	void FputInt32_BE(int32_t val);
	int64_t FgetInt64_BE();
	void FputInt64_BE(int64_t val);
	float FgetFloat_BE();
	void FputFloat_BE(float val);
	double FgetDouble_BE();
	void FputDouble_BE(double val);
	_TCHAR FgetTCHAR_BE();
	void FputTCHAR_BE(_TCHAR val);
	
	int Fgetc();
	int Fputc(int c);
	char *Fgets(char *str, int n);
	_TCHAR *Fgetts(_TCHAR *str, int n);
	int Fprintf(const char* format, ...);
	int Ftprintf(const _TCHAR* format, ...);
	
	size_t Fread(void* buffer, size_t size, size_t count);
	size_t Fwrite(const void* buffer, size_t size, size_t count);
	int Fseek(long offset, int origin);
	long Ftell();
	
	bool StateCheckUint32(uint32_t val);
	bool StateCheckInt32(int32_t val);
	bool StateCheckBuffer(const _TCHAR *buffer, size_t size, size_t count);
	
	void StateValue(bool &val);
	void StateValue(uint8_t &val);
	void StateValue(uint16_t &val);
	void StateValue(uint32_t &val);
	void StateValue(uint64_t &val);
	void StateValue(int8_t &val);
	void StateValue(int16_t &val);
	void StateValue(int32_t &val);
	void StateValue(int64_t &val);
	void StateValue(pair16_t &val);
	void StateValue(pair32_t &val);
	void StateValue(pair64_t &val);
	void StateValue(float &val);
	void StateValue(double &val);
	void StateValue(_TCHAR &val);
	
	void StateArray(bool *buffer, size_t size, size_t count);
	void StateArray(uint8_t *buffer, size_t size, size_t count);
	void StateArray(uint16_t *buffer, size_t size, size_t count);
	void StateArray(uint32_t *buffer, size_t size, size_t count);
	void StateArray(uint64_t *buffer, size_t size, size_t count);
	void StateArray(int8_t *buffer, size_t size, size_t count);
	void StateArray(int16_t *buffer, size_t size, size_t count);
	void StateArray(int32_t *buffer, size_t size, size_t count);
	void StateArray(int64_t *buffer, size_t size, size_t count);
	void StateArray(pair16_t *buffer, size_t size, size_t count);
	void StateArray(pair32_t *buffer, size_t size, size_t count);
	void StateArray(pair64_t *buffer, size_t size, size_t count);
	void StateArray(float *buffer, size_t size, size_t count);
	void StateArray(double *buffer, size_t size, size_t count);
	void StateArray(_TCHAR *buffer, size_t size, size_t count);
	
	// obsolete function
	void StateBuffer(void *buffer, size_t size, size_t count);
};

#endif
