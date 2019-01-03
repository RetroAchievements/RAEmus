#ifndef BEEP_H
#define BEEP_H

void BEEP88UpdateRequest(void *param);

void * BEEP88Init(void *param, int index, int clock, int sample_rate);
void BEEP88Shutdown(void *chip);
void BEEP88ResetChip(void *chip);
void BEEP88UpdateOne(void *chip, stream_sample_t *buffer, int length);

void BEEP88Write(void *chip,int v);
void BEEP88Control(void *chip,int v);

#endif
