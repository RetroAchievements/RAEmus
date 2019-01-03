#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"
#include "beep.h"


#define	SCALE	(1)

/* ---------- work of BEEP emulator ----------- */
typedef struct beep88_f {
	void			*param;
	int				clock;			/* 2400Hz */
	int				sample_rate;	/* 44100/22050 Hz */
	int				sample_bit_8;	/* TRUE / FALSE */
	int				cnt;			/* 0...	*/
	int				cnt_of_clock;	/* sample_rate/clock */
	int				cmd_sing;		/* TRUE / FALSE */
	unsigned char	port40;			/* Output value of port 40 */
} BEEP88;


/* ---------- update one of chip ----------- */
void BEEP88UpdateOne(void *chip, stream_sample_t *buffer, int length)
{
	BEEP88 *BEEP = chip;

	int	i, data, on_level;

	if (BEEP->sample_bit_8) {
		on_level = 0x20;
	} else {
		on_level = 0x2000;
	}

#if 1
	for (i=0; i < length ; i++) {
		if (BEEP->port40 & 0x20) {
			BEEP->cnt += SCALE;
			if      (BEEP->cnt < BEEP->cnt_of_clock/2) data = on_level;
			else if (BEEP->cnt < BEEP->cnt_of_clock  ) data = 0x0000;
			else {   BEEP->cnt -= BEEP->cnt_of_clock;  data = on_level; }
		} else {
			data = 0x0000;
		}
		if (BEEP->cmd_sing) {
			if (BEEP->port40 & 0x80) data = on_level;
		}
		*buffer ++ = data;
	}
#else
	for (i=0; i < length ; i++) {
		BEEP->cnt ++;
		if (BEEP->cnt >= BEEP->cnt_of_clock) {
			BEEP->cnt = 0;
		}
		if (BEEP->port40 & 0x20) {
			if      (BEEP->cnt < BEEP->cnt_of_clock/2) data = on_level;
			else                                       data = 0x0000;
		} else {
			data = 0x0000;
		}
		if (BEEP->cmd_sing) {
			if (BEEP->port40 & 0x80) data = on_level;
		}
		*buffer ++ = data;
	}
#endif
}


/* ---------- reset one of chip ---------- */
void BEEP88ResetChip(void *chip)
{
    BEEP88 *BEEP = chip;

    BEEP->cnt = 0;
    BEEP->port40 = 0;
}


/* ----------  Initialize BEEP emulator(s) ---------- */
void * BEEP88Init(void *param, int index, int clock, int sample_rate)
{
	BEEP88 *BEEP;

	/* allocate beep88 state space */
	if( (BEEP = (BEEP88 *)malloc(sizeof(BEEP88)))==NULL)
	return NULL;
	/* clear */
	memset(BEEP,0,sizeof(BEEP88));

	BEEP->param  = param;
	BEEP->clock	 = clock;
	BEEP->sample_rate  = sample_rate;
	BEEP->sample_bit_8 = FALSE;   /* ...always false... */
	BEEP->cnt_of_clock = sample_rate * SCALE / clock;
	BEEP->cmd_sing = TRUE;
	BEEP88ResetChip(BEEP);

	return BEEP;
}


/* ---------- shut down BEEP emulator ----------- */
void BEEP88Shutdown(void *chip)
{
	BEEP88 *BEEP = chip;

	free(BEEP);
}


/* ---------- BEEP I/O interface ---------- */
void BEEP88Write(void *chip,int v)
{
	BEEP88 *BEEP = chip;

	BEEP88UpdateRequest(BEEP->param);

	if( !(BEEP->port40 & 0x20) && (v & 0x20) ){
		BEEP->cnt = 0;
	}
	BEEP->port40 = v;
}


void BEEP88Control(void *chip,int v)
{
	BEEP88 *BEEP = chip;

	BEEP->cmd_sing = v;
}
