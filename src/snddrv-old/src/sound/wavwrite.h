/* This file is ported from MAME 0.112 */

#ifndef WAVWRITE_H
#define WAVWRITE_H

typedef struct _wav_file wav_file;

wav_file *wav_open(const char *filename, int sample_rate, int channels);
void wav_close(wav_file*wavptr);

void wav_add_data_16(wav_file *wavptr, INT16 *data, int samples);

#endif /* WAVWRITE_H */
