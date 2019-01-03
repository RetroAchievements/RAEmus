#ifndef BEEPINTF_H
#define BEEPINTF_H

#define MAX_BEEP88 1

struct BEEP88interface
{
	int num;
	int baseclock;
	int mixing_level[MAX_BEEP88];
	int (*port40read[MAX_BEEP88])(void);
	void (*port40write[MAX_BEEP88])(int data);
};


WRITE_HANDLER( BEEP88_write_port_0_w );
WRITE_HANDLER( BEEP88_control_port_0_w );

int BEEP88_sh_start(const struct MachineSound *msound);
void BEEP88_sh_stop(void);
void BEEP88_sh_reset(void);

void BEEP88UpdateRequest(int chip);



int BEEP88Init(int num, int clock, int sample_rate );
void BEEP88Shutdown(void);
void BEEP88ResetChip(int num);
void BEEP88UpdateOne(int num, INT16 *buffer, int length);

void BEEP88Write(int n,int v);
void BEEP88Control(int n,int v);

#endif
