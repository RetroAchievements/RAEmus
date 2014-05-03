#ifndef _CC_NETWORK_
#define _CC_NETWORK_

#ifdef __cplusplus
extern "C" {
#endif

#define CC_SUPPORT

typedef struct
{
	char RName[256];
	int RSize;
} _CC_Rom;

extern _CC_Rom CCRom;


int CC_Connect(char *Command, char *Rom, void (*Callback)(char mess[256]));
int CC_Close(void);
void CC_Idle(void (*Callback)(char mess[256]));


#ifdef __cplusplus
};
#endif

#endif _CC_NETWORK_
