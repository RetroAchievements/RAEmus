#ifndef _Z80DIS_H_
#define _Z80DIS_H_

#ifdef __cplusplus
extern "C" {
#endif
	

int z80dis(unsigned char *buf, int *Counter, char str[128]);


#ifdef __cplusplus
};
#endif

#endif