#include "driver.h"
#include "beep.h"

static	int	stream[MAX_BEEP88];

static const struct BEEP88interface *intf;


/* update request from beep.c */
void BEEP88UpdateRequest( int chip )
{
  stream_update( stream[chip], 0 );
}

int BEEP88_sh_start(const struct MachineSound *msound)
{
  int	i;
  intf = msound->sound_interface;

  /* stream system initialize */
  for (i = 0;i < intf->num;i++)
  {
    int volume;
    char name[20];
    sprintf(name,"%s #%d",sound_name(msound),i);
    volume = intf->mixing_level[i]&0xffff;
    stream[i] = stream_init(name,volume,Machine->sample_rate,i,BEEP88UpdateOne);
  }
  /* Initialize BEEP emurator */
  if (BEEP88Init(intf->num,intf->baseclock,Machine->sample_rate) == 0)
  {
    /* Ready */
    return 0;
  }
  /* error */
  /* stream close */
  return 1;
}


void BEEP88_sh_stop(void)
{
  BEEP88Shutdown();
}

void BEEP88_sh_reset(void)
{
  int i;

  for (i = 0;i < intf->num;i++)
    BEEP88ResetChip(i);
}


WRITE_HANDLER( BEEP88_write_port_0_w )
{
  BEEP88Write(0,data);
}

WRITE_HANDLER( BEEP88_control_port_0_w )
{
  BEEP88Control(0,data);
}
