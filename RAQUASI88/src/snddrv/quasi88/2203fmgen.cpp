//
// QUASI88 - xmame - fmgen interface of YM2203(fmgen)
//

extern "C" {
#include "sndintrf.h"
#include "streams.h"
#include "driver.h"
#include "wait.h"
}

#include "headers.h"
#include "2203fmgen.h"


#include "opna.h"


struct fmgen2203_info
{
	sound_stream *	stream;
	FM::OPN	*		opn;
	uint32			last_state;
	INT16 *			buf;
	size_t			buf_size;
	int				control_port_w;
};


extern "C" {
/* update callback from stream.c */
static void fmgen2203_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)param;
	int i;
	INT16 *p;
	stream_sample_t *bufL = buffer[0];
	stream_sample_t *bufR = buffer[1];
	uint32 last_count;

	if (info->buf_size < length) {
		if (info->buf) {
			free(info->buf);
			info->buf = NULL;
			info->buf_size = 0;
		}
	}
	if (info->buf == NULL) {
		info->buf = (INT16*)malloc((length + 512) * 2 * sizeof(INT16));
		if (info->buf) {
			info->buf_size = length + 512;
		}
	}

	// test
	//info->opn->Count( int(1/wait_freq_hz * 1000*1000) );

	last_count = (uint32)((state_of_cpu - info->last_state) / cpu_clock_mhz);
	if (last_count > 0) info->opn->Count(last_count);
	info->last_state = 0;

	if (info->buf) {

		memset(info->buf, 0, (length * 2) * sizeof(INT16));
		info->opn->Mix(info->buf, length);

		p = info->buf;
		for (i=0; i<length; i++) {
			*bufL++ = (stream_sample_t)*p++;
			*bufR++ = (stream_sample_t)*p++;
		}

	} else {
		memset(bufL, 0, length * sizeof(stream_sample_t));
		memset(bufR, 0, length * sizeof(stream_sample_t));
	}
}


static void *fmgen2203_start(int sndindex, int clock, const void *config)
{
	struct fmgen2203_info *info;

	info = (struct fmgen2203_info *)auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));


	/* stream system initialize */
	info->stream = stream_create(0,2,Machine->sample_rate,info,fmgen2203_stream_update);

	info->opn = new FM::OPN;

	if (info->opn->Init(clock,
						Machine->sample_rate ? Machine->sample_rate :44100)) {
		return info;
	}

	/* error */
	/* stream close */
	delete info->opn;

	return NULL;
}
static void fmgen2203_stop(void *token)
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)token;

	delete info->opn;
}
static void fmgen2203_reset(void *token)
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)token;

	info->opn->Reset();
}



READ8_HANDLER( FMGEN2203_status_port_0_r )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 0);
	return info->opn->ReadStatus();
}
READ8_HANDLER( FMGEN2203_status_port_1_r )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 1);
	return info->opn->ReadStatus();
}
READ8_HANDLER( FMGEN2203_status_port_2_r )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 2);
	return info->opn->ReadStatus();
}
READ8_HANDLER( FMGEN2203_status_port_3_r )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 3);
	return info->opn->ReadStatus();
}
READ8_HANDLER( FMGEN2203_status_port_4_r )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 4);
	return info->opn->ReadStatus();
}

READ8_HANDLER( FMGEN2203_read_port_0_r ) { return 0; }
READ8_HANDLER( FMGEN2203_read_port_1_r ) { return 0; }
READ8_HANDLER( FMGEN2203_read_port_2_r ) { return 0; }
READ8_HANDLER( FMGEN2203_read_port_3_r ) { return 0; }
READ8_HANDLER( FMGEN2203_read_port_4_r ) { return 0; }

WRITE8_HANDLER( FMGEN2203_control_port_0_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 0);
	info->control_port_w = data;
}
WRITE8_HANDLER( FMGEN2203_control_port_1_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 1);
	info->control_port_w = data;
}
WRITE8_HANDLER( FMGEN2203_control_port_2_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 2);
	info->control_port_w = data;
}
WRITE8_HANDLER( FMGEN2203_control_port_3_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 3);
	info->control_port_w = data;
}
WRITE8_HANDLER( FMGEN2203_control_port_4_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 4);
	info->control_port_w = data;
}

WRITE8_HANDLER( FMGEN2203_write_port_0_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 0);
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	info->opn->Count( uint32( (total_state-info->last_state)/cpu_clock_mhz ) );
	info->last_state = total_state;

	info->opn->SetReg( info->control_port_w, data );
}
WRITE8_HANDLER( FMGEN2203_write_port_1_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 1);
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	info->opn->Count( uint32( (total_state-info->last_state)/cpu_clock_mhz ) );
	info->last_state = total_state;

	info->opn->SetReg( info->control_port_w, data );
}
WRITE8_HANDLER( FMGEN2203_write_port_2_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 2);
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	info->opn->Count( uint32( (total_state-info->last_state)/cpu_clock_mhz ) );
	info->last_state = total_state;

	info->opn->SetReg( info->control_port_w, data );
}
WRITE8_HANDLER( FMGEN2203_write_port_3_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 3);
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	info->opn->Count( uint32( (total_state-info->last_state)/cpu_clock_mhz ) );
	info->last_state = total_state;

	info->opn->SetReg( info->control_port_w, data );
}
WRITE8_HANDLER( FMGEN2203_write_port_4_w )
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 4);
	uint32 total_state = (state_of_cpu + z80main_cpu.state0);
	info->opn->Count( uint32( (total_state-info->last_state)/cpu_clock_mhz ) );
	info->last_state = total_state;

	info->opn->SetReg( info->control_port_w, data );
}


WRITE8_HANDLER( FMGEN2203_word_0_w )
{
}

WRITE8_HANDLER( FMGEN2203_word_1_w )
{
}

void FMGEN2203_set_volume_0(float volume)
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 0);
	stream_set_output_gain(info->stream, 0, volume);
	stream_set_output_gain(info->stream, 1, volume);
}
void FMGEN2203_set_volume_1(float volume)
{
	struct fmgen2203_info *info = (struct fmgen2203_info *)sndti_token(SOUND_FMGEN2203, 1);
	stream_set_output_gain(info->stream, 0, volume);
	stream_set_output_gain(info->stream, 1, volume);
}






/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void fmgen2203_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void fmgen2203_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = fmgen2203_set_info;	break;
		case SNDINFO_PTR_START:							info->start = fmgen2203_start;			break;
		case SNDINFO_PTR_STOP:							info->stop = fmgen2203_stop;			break;
		case SNDINFO_PTR_RESET:							info->reset = fmgen2203_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "FMGEN2203";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "ym2203";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "008a";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2003, cisc";	break;
	}
}
}

