#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"
#include "beep.h"


#define	SCALE	(1)

/* ---------- work of BEEP emulator ----------- */
typedef struct beep_f {
  int		status;
  int		clock;			/* 2400Hz */
  int		sample_rate;		/* 44100/22050 Hz */
  int		sample_bit_8;		/* TRUE / FALSE */
  int		cnt;			/* 0...	*/
  int		cnt_of_clock;		/* sample_rate/clock */
  int		cmd_sing;		/* TRUE / FALSE */
  unsigned char	port40;			/* Output value of port 40 */
} Beep88;


static Beep88 BEEP[MAX_BEEP88];


/* ---------- update one of chip ----------- */
void BEEP88UpdateOne(int num, INT16 *buffer, int length)
{
  int	i, data;
#if 0
  unsigned char	 *buf8  = (unsigned char *)buffer;
#endif
  unsigned short *buf16 = (unsigned short *)buffer;

  /*printf("Beep Update %d %d %d < %d\n",num, length, BEEP[num].cnt, BEEP[num].cnt_of_clock);*/
  /*printf("Beep Update %d=%d %d %dHz %d %d<%d %02x\n",num,BEEP[num].status,BEEP[num].clock,BEEP[num].sample_rate,BEEP[num].sample_bit_8,BEEP[num].cnt,BEEP[num].cnt_of_clock,BEEP[num].port40);*/


#if 0
  if( BEEP[num].sample_bit_8 ){
    for( i=0; i < length ; i++ ){
      if( BEEP[num].port40 & 0x20 ){
	BEEP[num].cnt += SCALE;
	if     ( BEEP[num].cnt < BEEP[num].cnt_of_clock/2 ) data = 0x20;
	else if( BEEP[num].cnt < BEEP[num].cnt_of_clock   ) data = 0x00;
	else{    BEEP[num].cnt -= BEEP[num].cnt_of_clock;   data = 0x20; }
      }else{
	data = 0x00;
      }
      if( BEEP[num].port40 & 0x80 ) data = 0x20;
      *buf8 ++ = data;
    }
  }else
#endif
  {
    for( i=0; i < length ; i++ ){
      if( BEEP[num].port40 & 0x20 ){
	BEEP[num].cnt += SCALE;
	if     ( BEEP[num].cnt < BEEP[num].cnt_of_clock/2 ) data = 0x2000;
	else if( BEEP[num].cnt < BEEP[num].cnt_of_clock   ) data = 0x0000;
	else{    BEEP[num].cnt -= BEEP[num].cnt_of_clock;   data = 0x2000; }
      }else{
	data = 0x0000;
      }
      if( BEEP[num].cmd_sing ){
	if( BEEP[num].port40 & 0x80 ) data = 0x2000;
      }
      *buf16 ++ = data;
    }
  }
}


/* ---------- reset one of chip ---------- */
void BEEP88ResetChip(int num)
{
  /*printf("Beep Reset %d\n",num);*/

  BEEP[num].cnt = 0;
  BEEP[num].port40 = 0;
}


/* ----------  Initialize BEEP emulator(s) ---------- */
int BEEP88Init(int num, int clock, int sample_rate )
{
  int	i;
  /*printf("Beep Init ");*/

  if (num>MAX_BEEP88) return (-1);

  /* clear */
  memset(BEEP,0,sizeof(Beep88) * num);

  for ( i = 0 ; i < num; i++ ) {
    BEEP[i].status	 = i;
    BEEP[i].clock	 = clock;
    BEEP[i].sample_rate  = sample_rate;
    BEEP[i].sample_bit_8 = FALSE;   /* ...always false... */
    BEEP[i].cnt_of_clock = sample_rate * SCALE / clock;
  }

  /*printf(" %d %d %d %d %d\n",num, clock,sample_rate,sample_bits,BEEP[0].cnt_of_clock);*/
  return(0);
}


/* ---------- shut down BEEP emulator ----------- */
void BEEP88Shutdown(void)
{
  /*printf("Beep Shutdown\n");*/
}


/* ---------- BEEP I/O interface ---------- */
void BEEP88Write(int n,int v)
{
  /*printf("Beep Write %x\n",v);*/

  BEEP88UpdateRequest(n);

  if( !(BEEP[n].port40 & 0x20) && (v & 0x20) ){
    BEEP[n].cnt = 0;
  }
  BEEP[n].port40 = v;
}


void BEEP88Control(int n,int v)
{
  BEEP[n].cmd_sing = v;
}
