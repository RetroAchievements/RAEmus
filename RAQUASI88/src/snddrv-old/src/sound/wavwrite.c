/* This file is ported from MAME 0.112 */

#include "mame-quasi88.h"
#include "wavwrite.h"

#define		FILE						OSD_FILE
#define		fopen(filename, mode)		osd_fopen(FTYPE_WRITE, filename, mode)
#define		fclose(fp)					osd_fclose(fp)
#define		fflush(fp)					osd_fflush(fp)
#define		fseek(fp, offset, whence)	osd_fseek(fp, offset, whence)
#define		ftell(fp)					osd_ftell(fp)
#define		fwrite(p, size, nobj, fp)	osd_fwrite(p, size, nobj, fp)

struct _wav_file
{
	FILE *file;
	UINT32 total_offs;
	UINT32 data_offs;
};

#ifdef LSB_FIRST
#define intel_long(x) (x)
#define intel_short(x) (x)
#else
#define intel_long(x) (((x << 24) | (((unsigned long) x) >> 24) | (( x & 0x0000ff00) << 8) | (( x & 0x00ff0000) >> 8)))
#define intel_short(x) (((x) << 8) | ((x) >> 8))
#endif

wav_file *wav_open(const char *filename, int sample_rate, int channels)
{
	wav_file *wav;
	UINT32 bps, temp32;
	UINT16 align, temp16;

	/* allocate memory for the wav struct */
	wav = (wav_file *) malloc(sizeof(struct _wav_file));
	if (!wav)
		return NULL;

	/* create the file */
	wav->file = fopen(filename, "wb");
	if (!wav->file)
	{
		free(wav);
		return NULL;
	}

	/* write the 'RIFF' header */
	fwrite("RIFF", 1, 4, wav->file);

	/* write the total size */
	temp32 = 0;
	wav->total_offs = ftell(wav->file);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the 'WAVE' type */
	fwrite("WAVE", 1, 4, wav->file);

	/* write the 'fmt ' tag */
	fwrite("fmt ", 1, 4, wav->file);

	/* write the format length */
	temp32 = intel_long(16);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the format (PCM) */
	temp16 = intel_short(1);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the channels */
	temp16 = intel_short(channels);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the sample rate */
	temp32 = intel_long(sample_rate);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the bytes/second */
	bps = sample_rate * 2 * channels;
	temp32 = intel_long(bps);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the block align */
	align = 2 * channels;
	temp16 = intel_short(align);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the bits/sample */
	temp16 = intel_short(16);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the 'data' tag */
	fwrite("data", 1, 4, wav->file);

	/* write the data length */
	temp32 = 0;
	wav->data_offs = ftell(wav->file);
	fwrite(&temp32, 1, 4, wav->file);

	return wav;
}


void wav_close(wav_file *wav)
{
	UINT32 total = ftell(wav->file);
	UINT32 temp32;

	/* update the total file size */
	fseek(wav->file, wav->total_offs, SEEK_SET);
	temp32 = total - (wav->total_offs + 4);
	temp32 = intel_long(temp32);
	fwrite(&temp32, 1, 4, wav->file);

	/* update the data size */
	fseek(wav->file, wav->data_offs, SEEK_SET);
	temp32 = total - (wav->data_offs + 4);
	temp32 = intel_long(temp32);
	fwrite(&temp32, 1, 4, wav->file);

	fclose(wav->file);
	free(wav);
}


void wav_add_data_16(wav_file *wav, INT16 *data, int samples)
{
#ifdef LSB_FIRST
	/* just write and flush the data */
	fwrite(data, 2, samples, wav->file);
	fflush(wav->file);
#else
	INT16 *temp;
	int i;

	/* allocate temp memory */
	temp = malloc(samples * sizeof(temp[0]));
	if (!temp)
		return;

	/* swap */
	for (i = 0; i < samples; i++)
	{
		temp[i] = intel_short((UINT16) data[i]);
	}

	/* write and flush */
	fwrite(temp, 2, samples, wav->file);
	fflush(wav->file);

	/* free memory */
	free(temp);
#endif
}
