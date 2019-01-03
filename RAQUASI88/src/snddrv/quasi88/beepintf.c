#include "sndintrf.h"
#include "streams.h"
#include "beepintf.h"
#include "beep.h"

struct beep88_info
{
	sound_stream *	stream;
	void *			chip;
};


/* update request from beep.c */
void BEEP88UpdateRequest(void *param)
{
	struct beep88_info *info = param;
	stream_update(info->stream);
}


static void beep88_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct beep88_info *info = param;
	BEEP88UpdateOne(info->chip, buffer[0], length);
}


static void *beep88_start(int sndindex, int clock, const void *config)
{
	struct beep88_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* stream system initialize */
	info->stream = stream_create(0,1,Machine->sample_rate,info,beep88_stream_update);

	/* Initialize beep emurator */
	info->chip = BEEP88Init(info,sndindex,clock,Machine->sample_rate);

	if (info->chip)
		return info;

	/* error */
	/* stream close */
	return NULL;
}

static void beep88_stop(void *token)
{
	struct beep88_info *info = token;
	BEEP88Shutdown(info->chip);
}

static void beep88_reset(void *token)
{
	struct beep88_info *info = token;
    BEEP88ResetChip(info->chip);
}


WRITE8_HANDLER( BEEP88_write_port_0_w )
{
	struct beep88_info *info = sndti_token(SOUND_BEEP88, 0);
	BEEP88Write(info->chip,data);
}

WRITE8_HANDLER( BEEP88_control_port_0_w )
{
	struct beep88_info *info = sndti_token(SOUND_BEEP88, 0);
	BEEP88Control(info->chip,data);
}

void BEEP88_set_volume(float volume)
{
	struct beep88_info *info = sndti_token(SOUND_BEEP88, 0);
	stream_set_output_gain(info->stream, 0, volume);
}





/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void beep88_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void beep88_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = beep88_set_info;		break;
		case SNDINFO_PTR_START:							info->start = beep88_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = beep88_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = beep88_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "BEEP88";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Quasi88";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2010, S.F";	break;
	}
}
