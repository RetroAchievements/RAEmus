//
// QUASI88 - xmame - fmgen interface of YM2608(fmgen)
//

extern "C" {
#include "driver.h"
#include "wait.h"
}

#include "headers.h"
#include "2608fmgen.h"


#include "opna.h"


#define YM2608_NUMBUF 2

static	FM::OPNA	*opna[MAX_2608];
static	uint32	last_state[MAX_2608];
static	INT16	*buf = NULL;
static	size_t	buf_size = 0;

static int stream[MAX_2608];

static const struct YM2608interface *intf;


extern "C" {
/* update callback from stream.c */
static void FMGEN2608UpdateCallback(int num, INT16 **buffer, int length)
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
  //opna[num]->Count( int(1/wait_freq_hz * 1000*1000) );

  last_count = (uint32)( (state_of_cpu - last_state[num])/cpu_clock_mhz );
  if( last_count > 0 ) opna[num]->Count( last_count );
  last_state[num] = 0;

  if( buf ){

    memset( buf, 0, (length*2)*sizeof(INT16) );
	opna[num]->Mix( buf, length );

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


int FMGEN2608_sh_start(const struct MachineSound *msound)
{
	int i,j;
	int rate = Machine->sample_rate;
	char buf[YM2608_NUMBUF][40];
	const char *name[YM2608_NUMBUF];
	int mixed_vol,vol[YM2608_NUMBUF];

	intf = (YM2608interface *)msound->sound_interface;
	if( intf->num > MAX_2608 ) return 1;

	/* stream system initialize */
	for (i = 0;i < intf->num;i++)
	{
		mixed_vol = intf->volumeFM[i];
		/* stream setup */
		for (j = 0 ; j < YM2608_NUMBUF ; j++)
		{
			name[j]=buf[j];
			vol[j] = mixed_vol & 0xffff;
			mixed_vol>>=16;
			sprintf(buf[j],"%s #%d Ch%d",sound_name(msound),i,j+1);
		}
		stream[i] = stream_init_multi(YM2608_NUMBUF,name,vol,rate,i,FMGEN2608UpdateCallback);

		opna[i] = new FM::OPNA;

		if (opna[i]->Init( intf->baseclock, 
						   (Machine->sample_rate ?Machine->sample_rate :44100),
						   NULL )==false )
		{
		  /* error */
		  /* stream close */
		  return 1;
		}
		if( sound2_adpcm ){
		  uint8* adpcmbuf = opna[i]->GetADPCMBuffer();
		  if( adpcmbuf ){
			memcpy( adpcmbuf, sound2_adpcm, 256*1024 );
		  }
		}
	}
	/* Ready */
	return 0;
}
void FMGEN2608_sh_stop(void)
{
	int i;
	for (i = 0;i < intf->num;i++)
	  delete opna[i];
}

void FMGEN2608_sh_reset(void)
{
	int i;

	for (i = 0;i < intf->num;i++)
	  opna[i]->Reset();
}



READ_HANDLER( FMGEN2608_status_port_0_A_r ) { return opna[0]->ReadStatus(); }
READ_HANDLER( FMGEN2608_status_port_0_B_r ) { return opna[0]->ReadStatusEx(); }
READ_HANDLER( FMGEN2608_status_port_1_A_r ) { return opna[1]->ReadStatus(); }
READ_HANDLER( FMGEN2608_status_port_1_B_r ) { return opna[1]->ReadStatusEx(); }

READ_HANDLER( FMGEN2608_read_port_0_r ) { return 0; }
READ_HANDLER( FMGEN2608_read_port_1_r ) { return 0; }

static int control_port_w[MAX_2608][2];
WRITE_HANDLER( FMGEN2608_control_port_0_A_w )
{
	control_port_w[0][0] = data;
}
WRITE_HANDLER( FMGEN2608_control_port_0_B_w )
{
	control_port_w[0][1] = data | 0x100;
}

WRITE_HANDLER( FMGEN2608_control_port_1_A_w )
{
	control_port_w[1][0] = data;
}
WRITE_HANDLER( FMGEN2608_control_port_1_B_w )
{
	control_port_w[1][1] = data | 0x100;
}

WRITE_HANDLER( FMGEN2608_data_port_0_A_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opna[0]->Count( uint32( (total_state-last_state[0])/cpu_clock_mhz ) );
	last_state[0] = total_state;

	opna[0]->SetReg( control_port_w[0][0], data );
}
WRITE_HANDLER( FMGEN2608_data_port_0_B_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opna[0]->Count( uint32( (total_state-last_state[0])/cpu_clock_mhz ) );
	last_state[0] = total_state;

	opna[0]->SetReg( control_port_w[0][1], data );
}

WRITE_HANDLER( FMGEN2608_write_port_1_A_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opna[1]->Count( uint32( (total_state-last_state[1])/cpu_clock_mhz ) );
	last_state[1] = total_state;

	opna[1]->SetReg( control_port_w[1][0], data );
}
WRITE_HANDLER( FMGEN2608_write_port_1_B_w )
{
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	opna[1]->Count( uint32( (total_state-last_state[1])/cpu_clock_mhz ) );
	last_state[1] = total_state;

	opna[1]->SetReg( control_port_w[1][1], data );
}

WRITE_HANDLER( FMGEN2608_data_port_1_A_w ){
}
WRITE_HANDLER( FMGEN2608_data_port_1_B_w ){
}


