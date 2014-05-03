/* Header file for decode functions */
#ifndef _GENIE_DECODE_H__
#define _GENIE_DECODE_H__

struct patch { unsigned int addr, data; };

#ifdef __cplusplus
extern "C" {
#endif

struct GG_Code
{
	char code[16];
	char name[240];
	unsigned int active;
	unsigned int restore;
	unsigned int addr;
	unsigned short data;
};

extern struct GG_Code Liste_GG[256];
extern char Patch_Dir[1024];

int Load_Patch_File(void);
int Save_Patch_File(void);
void decode(char* code, struct patch *result);
void Patch_Codes(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _GENIE_DECODE_H__
