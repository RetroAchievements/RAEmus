#ifndef CFG_H_
#define CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "util.h"

/* Paths to ROM images. */
extern char CFG_osa_filename[FILENAME_MAX];
extern char CFG_osb_filename[FILENAME_MAX];
extern char CFG_xlxe_filename[FILENAME_MAX];
extern char CFG_5200_filename[FILENAME_MAX];
extern char CFG_basic_filename[FILENAME_MAX];

/* Compares the string PARAM with each entry in the CFG_STRINGS array
   (of size CFG_STRINGS_SIZE), and returns index under which PARAM is found.
   If PARAM does not exist in CFG_STRINGS, returns value lower than 0.
   String comparison is case-insensitive. */
int CFG_MatchTextParameter(char const *param, char const * const cfg_strings[], int cfg_strings_size);

int Palette_Read(char *file);
void Palette_Generate(int black, int white, int saturation, int contrast, int brightness, int gamma, int external);

#ifdef __cplusplus
}
#endif

#endif /* CFG_H_ */
