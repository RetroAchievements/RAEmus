//
// QUASI88 - xmame - fmgen interface of YM2203(fmgen)
//

extern "C" {
#include "driver.h"
#include "wait.h"
}

#include "headers.h"
#include "2203fmgen.h"


#include "opna.h"


#define YM2203_NUMBUF 2

static	FM::OPN	*opn[MAX_2203];
static	uint32	last_state[MAX_2203];
static	INT16	*buf = NULL;
static	size_t	buf_size = 0;

static int stream[MAX_2203];

static const struct YM2203interface *intf;


extern "C" {
/* update callback from stream.c */
static void FMGEN2203UpdateCallback(int num, INT16 **buffer, int length)
{
  int i;
  INT16 *p;
  INT16 *bufL = buffer[0];
  INT16 *bufR = buffer[1];
  uint32 last_count;

  if( buf_size < length ){
	if( buf ){
	  free( buf );
	  buf = NULL;
	  buf_size = 0;
	}
  }
  if( buf==NULL ){
	buf = (INT16*)malloc( (length+512)*2*sizeof(INT16) );
	if( buf ){
	  buf_size = length+512;
	}
  }

  // test
  //opn[num]->Count( int(1/wait_freq_hz * 1000*1000) );

  last_count = (uint32)( (state_of_cpu - last_state[num])/cpu_clock_mhz );
  if( last_count > 0 ) opn[num]->Count( last_count );
  last_state[num] = 0;

  if( buf ){

    memset( buf, 0, (length*2)*sizeof(INT16) );
	opn[num]->Mix( buf, length );

	p = buf;
	for( i=0; i<length; i++ ){
	  *bufL++ = *p++;
	  *bufR++ = *p++;
	}

  }else{
    memset( bufL, 0, length*2 );
    memset( bufR, 0, length*2 );
  }
}
}


int FMGEN2203_sh_start(const struct MachineSound *msound)
{
	int i,j;
	int rate = Machine->sample_rate;
	char buf[YM2203_NUMBUF][40];
	const char *name[YM2203_NUMBUF];
	int mixed_vol,vol[YM2203_NUMBUF];

	intf = (YM2203interface *)msound->sound_interface;
	if( intf->num > MAX_2203 ) return 1;

	/* stream system initialize */
	for (i = 0;i < intf->num;i++)
	{
		mixed_vol = intf->mixing_level[i];
		/* stream setup */
		for (j = 0 ; j < YM2203_NUMBUF ; j++)
		{
			name[j]=buf[j];
			vol[j] = mixed_vol & 0xffff;
			mixed_vol>>=16;
			sprintf(buf[j],"%s #%d Ch%d",sound_name(msound),i,j+1);
		}
		stream[i] = stream_init_multi(YM2203_NUMBUF,name,vol,rate,i,FMGEN2203UpdateCallback);

		opn[i] = new FM::OPN;

		if (opn[i]->Init( intf->baseclock, 
						  (Machine->sample_rate ?Machine->sample_rate :44100)
						  )==false )
		{
		  /* error */
		  /* stream close */
		  return 1;
		}
	}
	/* Ready */
	return 0;
}
void FMGEN2203_sh_stop(void)
{
	int i;
	for (i = 0;i < intf->num;i++)
	  delete opn[i];
}

void FMGEN2203_sh_reset(void)
{
	int i;

	for (i = 0;i < intf->num;i++)
	  opn[i]->Reset();
}



READ_HANDLER( FMGEN2203_status_port_0_r ) { return opn[0]->ReadStatus(); }
READ_HANDLER( FMGEN2203_status_port_1_r ) { return opn[1]->ReadStatus(); }
READ_HANDLER( FMGEN2203_status_port_2_r ) { return opn[2]->ReadStatus(); }
READ_HANDLER( FMGEN2203_status_port_3_r ) { return opn[3]->ReadStatus(); }
READ_HANDLER( FMGEN2203_status_port_4_r ) { return opn[4]->ReadStatus(); }

READ_HANDLER( FMGEN2203_read_port_0_r ) { return 0; }
READ_HANDLER( FMGEN2203_read_port_1_r ) { return 0; }
READ_HANDLER( FMGEN2203_read_port_2_r ) { return 0; }
READ_HANDLER( FMGEN2203_read_port_3_r ) { return 0; }
READ_HANDLER( FMGEN2203_read_port_4_r ) { return 0; }

static int control_port_w[MAX_2203];
WRITE_HANDLER( FMGEN2203_control_port_0_w )
{
	control_port_w[0] = data;
}
WRITE_HANDLER( FMGEN2203_control_port_1_w )
{
	control_port_w[1] = data;
}
WRITE_HANDLER( FMGEN2203_control_port_2_w )
{
	control_port_w[2] = data;
}
WRITE_HANDLER( FMGEN2203_control_port_3_w )
{
	control_port_w[3] = data;
}
WRITE_HANDLER( FMGEN2203_control_port_4_w )
{
	control_port_w[4] = data;
}

WRITE_HANDLER( FMGEN2203_write_port_0_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opn[0]->Count( uint32( (total_state-last_state[0])/cpu_clock_mhz ) );
	last_state[0] = total_state;

	opn[0]->SetReg( control_port_w[0], data );
}
WRITE_HANDLER( FMGEN2203_write_port_1_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opn[1]->Count( uint32( (total_state-last_state[1])/cpu_clock_mhz ) );
	last_state[1] = total_state;

	opn[1]->SetReg( control_port_w[1], data );
}
WRITE_HANDLER( FMGEN2203_write_port_2_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opn[2]->Count( uint32( (total_state-last_state[2])/cpu_clock_mhz ) );
	last_state[2] = total_state;

	opn[2]->SetReg( control_port_w[2], data );
}
WRITE_HANDLER( FMGEN2203_write_port_3_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opn[3]->Count( uint32( (total_state-last_state[3])/cpu_clock_mhz ) );
	last_state[3] = total_state;

	opn[3]->SetReg( control_port_w[3], data );
}
WRITE_HANDLER( FMGEN2203_write_port_4_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opn[4]->Count( uint32( (total_state-last_state[4])/cpu_clock_mhz ) );
	last_state[4] = total_state;

	opn[4]->SetReg( control_port_w[4], data );
}

WRITE_HANDLER( FMGEN2203_word_0_w )
{
}

WRITE_HANDLER( FMGEN2203_word_1_w )
{
}


