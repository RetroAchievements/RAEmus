#ifndef BASIC_H_INCLUDED
#define BASIC_H_INCLUDED

extern int basic_mode;

int basic_encode_list(FILE *fp);
int basic_load_intermediate_code(FILE *fp);
int basic_decode_list(FILE *fp);
int basic_save_intermediate_code(FILE *fp);

#endif	/* BASIC_H_INCLUDED */
