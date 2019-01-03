#include "driver.h"


/* src/mame.c ============================================================== */

/*-------------------------------------------------
	machine_add_sound - add a sound system during 
	machine driver expansion
-------------------------------------------------*/

struct MachineSound *machine_add_sound(struct InternalMachineDriver *machine, const char *tag, int type, void *sndintf)
{
	int soundnum;

	for (soundnum = 0; soundnum < MAX_SOUND; soundnum++)
		if (machine->sound[soundnum].sound_type == 0)
		{
			machine->sound[soundnum].tag = tag;
			machine->sound[soundnum].sound_type = type;
			machine->sound[soundnum].sound_interface = sndintf;
			return &machine->sound[soundnum];
		}

	logerror("Out of sounds!\n");
	return NULL;

}

/* src/fileio.c ============================================================ */

/***************************************************************************
	mame_fread_swap
***************************************************************************/

UINT32 mame_fread_swap(mame_file *file, void *buffer, UINT32 length)
{
	UINT8 *buf;
	UINT8 temp;
	int res, i;

	/* standard read first */
	res = mame_fread(file, buffer, length);

	/* swap the result */
	buf = buffer;
	for (i = 0; i < res; i += 2)
	{
		temp = buf[i];
		buf[i] = buf[i + 1];
		buf[i + 1] = temp;
	}

	return res;
}

/* src/common.c ============================================================ */

/***************************************************************************

	Sample handling code

	This function is different from readroms() because it doesn't fail if
	it doesn't find a file: it will load as many samples as it can find.

***************************************************************************/

#ifdef LSB_FIRST
#define intelLong(x) (x)
#else
#define intelLong(x) (((x << 24) | (((unsigned long) x) >> 24) | (( x & 0x0000ff00) << 8) | (( x & 0x00ff0000) >> 8)))
#endif

/*-------------------------------------------------
	read_wav_sample - read a WAV file as a sample
-------------------------------------------------*/

static struct GameSample *read_wav_sample(mame_file *f)
{
	unsigned long offset = 0;
	UINT32 length, rate, filesize, temp32;
	UINT16 bits, temp16;
	char buf[32];
	struct GameSample *result;

	/* read the core header and make sure it's a WAVE file */
	offset += mame_fread(f, buf, 4);
	if (offset < 4)
		return NULL;
	if (memcmp(&buf[0], "RIFF", 4) != 0)
		return NULL;

	/* get the total size */
	offset += mame_fread(f, &filesize, 4);
	if (offset < 8)
		return NULL;
	filesize = intelLong(filesize);

	/* read the RIFF file type and make sure it's a WAVE file */
	offset += mame_fread(f, buf, 4);
	if (offset < 12)
		return NULL;
	if (memcmp(&buf[0], "WAVE", 4) != 0)
		return NULL;

	/* seek until we find a format tag */
	while (1)
	{
		offset += mame_fread(f, buf, 4);
		offset += mame_fread(f, &length, 4);
		length = intelLong(length);
		if (memcmp(&buf[0], "fmt ", 4) == 0)
			break;

		/* seek to the next block */
		mame_fseek(f, length, SEEK_CUR);
		offset += length;
		if (offset >= filesize)
			return NULL;
	}

	/* read the format -- make sure it is PCM */
	offset += mame_fread_lsbfirst(f, &temp16, 2);
	if (temp16 != 1)
		return NULL;

	/* number of channels -- only mono is supported */
	offset += mame_fread_lsbfirst(f, &temp16, 2);
	if (temp16 != 1)
		return NULL;

	/* sample rate */
	offset += mame_fread(f, &rate, 4);
	rate = intelLong(rate);

	/* bytes/second and block alignment are ignored */
	offset += mame_fread(f, buf, 6);

	/* bits/sample */
	offset += mame_fread_lsbfirst(f, &bits, 2);
	if (bits != 8 && bits != 16)
		return NULL;

	/* seek past any extra data */
	mame_fseek(f, length - 16, SEEK_CUR);
	offset += length - 16;

	/* seek until we find a data tag */
	while (1)
	{
		offset += mame_fread(f, buf, 4);
		offset += mame_fread(f, &length, 4);
		length = intelLong(length);
		if (memcmp(&buf[0], "data", 4) == 0)
			break;

		/* seek to the next block */
		mame_fseek(f, length, SEEK_CUR);
		offset += length;
		if (offset >= filesize)
			return NULL;
	}

	/* allocate the game sample */
	result = auto_malloc(sizeof(struct GameSample) + length);
	if (result == NULL)
		return NULL;

	/* fill in the sample data */
	result->length = length;
	result->smpfreq = rate;
	result->resolution = bits;

	/* read the data in */
	if (bits == 8)
	{
		mame_fread(f, result->data, length);

		/* convert 8-bit data to signed samples */
		for (temp32 = 0; temp32 < length; temp32++)
			result->data[temp32] ^= 0x80;
	}
	else
	{
		/* 16-bit data is fine as-is */
		mame_fread_lsbfirst(f, result->data, length);
	}

	return result;
}


/*-------------------------------------------------
	readsamples - load all samples
-------------------------------------------------*/

struct GameSamples *readsamples(const char **samplenames,const char *basename)
/* V.V - avoids samples duplication */
/* if first samplename is *dir, looks for samples into "basename" first, then "dir" */
{
	int i;
	struct GameSamples *samples;
	int skipfirst = 0;

	/* if the user doesn't want to use samples, bail */
	if (!options.use_samples) return 0;

	if (samplenames == 0 || samplenames[0] == 0) return 0;

	if (samplenames[0][0] == '*')
		skipfirst = 1;

	i = 0;
	while (samplenames[i+skipfirst] != 0) i++;

	if (!i) return 0;

	if ((samples = auto_malloc(sizeof(struct GameSamples) + (i-1)*sizeof(struct GameSample))) == 0)
		return 0;

	samples->total = i;
	for (i = 0;i < samples->total;i++)
		samples->sample[i] = 0;

	for (i = 0;i < samples->total;i++)
	{
		mame_file *f;

		if (samplenames[i+skipfirst][0])
		{
			if ((f = mame_fopen(basename,samplenames[i+skipfirst],FILETYPE_SAMPLE,0)) == 0)
				if (skipfirst)
					f = mame_fopen(samplenames[0]+1,samplenames[i+skipfirst],FILETYPE_SAMPLE,0);
			if (f != 0)
			{
				samples->sample[i] = read_wav_sample(f);
				mame_fclose(f);
			}
#if 1		/* QUASI88 */
			if (verbose_proc) {
				if (f == NULL) {
					printf( "  %-12s ... Not Found\n",samplenames[i+skipfirst]);
				} else {
					printf( "  Found %-12s : Load...", samplenames[i+skipfirst]);
					if (samples->sample[i] == NULL) printf("FAILED\n");
					else                            printf("OK\n");
				}
			}
#endif		/* QUASI88 */

		}
	}

	return samples;
}
