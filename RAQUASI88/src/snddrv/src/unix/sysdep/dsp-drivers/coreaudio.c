/*
 * Driver for Mac OS X CoreAudio architecture.
 * written for Mac OS X 10.3 (panther).
 * this version uses AudioUnit interface which seems to be called
 * the 'DefaultOutputUnit'.
 * The DefaultOutputUnit provides universal conversions
 * between the input and the output format.
 *
 * app. 09/19/2004
 */

// following is the original document.
/*
 * Driver for Mac OS X core audio interface, primarily for use
 * with the OpenStep driver on this system. This driver was written for
 * the OS X public beta, and emulated lower sample rates and mono sound as
 * the device did not appear to support anything other than full rate stereo.
 * Interestingly the released code also does not appear to do this, and thus
 * this driver has inherited these abilities in order to continue working. If
 * we discover a way to set the device at some layter date then this code can
 * be updated then.
 *
 * -bat. 12/04/2001 
 */     

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <Carbon/Carbon.h>
#include <CoreAudio/AudioHardware.h>
#include <AudioUnit/AudioUnit.h>

#include "sysdep/sysdep_dsp.h"
#include "sysdep/sysdep_dsp_priv.h"
#include "sysdep/plugin_manager.h"

// #define NAME_LEN 256		/* max length of a device name */

/*
 * We have to implement a FIFO of blocks of floats, without using up
 * vast amounts of memory, or being too inefficient. We thus require
 * the queued blocks to be created by malloc (so we do not have to copy
 * the data) but free them ourself upon dequeue.
 */

struct audio_block {
	SInt8 *block_base;		/* base of the memory block */
	SInt8 *current_base;		/* current unread position */
	int samples_left;		/* number left in block */
	struct audio_block *next;	/* next block in list */
};

struct audio_queue {
	struct audio_block *head;	/* head of queue */
	struct audio_block *tail;	/* tail of queue */
	pthread_mutex_t	mutex;		/* queue locking mutex */
};

/*
 * Adds a block of audio samples to the end of the given queue. This
 * function is 'unsafe' and is thus wrapped by a function that handles
 * the queue mutex lock.
 */

static void
unsafe_queue_audio_block(struct audio_queue *queue, SInt8 *block, int len)
{
	struct audio_block *new = malloc(sizeof(struct audio_block));

	/* if malloc fails we ditch the block */
	if(!new)
		return;

	/* fill it out */
	new->block_base = block;
	new->current_base = block;
	new->samples_left = len;
	new->next = NULL;

	/* add it to the end */
	if(!queue->head) {
		queue->head = new;
		queue->tail = new;
	} else {
		queue->tail->next = new;
		queue->tail = new;
	}
}

/*
 * Wrapper to handle mutex locking on queue
 */

static void
queue_audio_block(struct audio_queue *queue, SInt8 *block, int len)
{
	pthread_mutex_lock(&queue->mutex);
	unsafe_queue_audio_block(queue, block, len);
	pthread_mutex_unlock(&queue->mutex);
}

/*
 * Dequeue a certain number of audio bytes from the fifo. For blocks
 * smaller than the remaining number we simply copy them and reduce
 * the count. For blocks that go over this limit we have to make a
 * recursive call on the function to fill out the rest. When there
 * are no blocks left the buffer is zero padded. As with the queueing function
 * this is unsafe and wrapped by a mutex handling funcion.
 */

static void
unsafe_dequeue_audio_block(struct audio_queue *queue, SInt8 *block, int len)
{
	struct audio_block *head = queue->head;

	/* anything to do ? */
	if(len < 1)
		return;

	/* zero padding ? */
	if(!head) {
		int i;
		for(i=0;i<len;i++)
			*block++ = 0;
		return;
	}

	/* smaller than current */
	if(len < head->samples_left) {
		memcpy(block, head->current_base, len);
		head->samples_left -= len;
		head->current_base += len;
		return;
	}

	/* copy all of this block */
	memcpy(block, head->current_base, head->samples_left);
	len -= head->samples_left;
	block += head->samples_left;

	/* free it up and recuurse */
	queue->head = head->next;
	if(!queue->head)
		queue->tail = NULL;
	free(head->block_base);
	free(head);
	unsafe_dequeue_audio_block(queue, block, len);
}

/*
 * Wrapper to handle mutex locking on queue
 */

static void
dequeue_audio_block(struct audio_queue *queue, SInt8 *block, int len)
{
	pthread_mutex_lock(&queue->mutex);
	unsafe_dequeue_audio_block(queue, block, len);
	pthread_mutex_unlock(&queue->mutex);
}

/*
 * This is the private data structure containing the audio device ID,
 * the queue and some flags dictating how we adapt the sound samples
 * to work with the setting on this device.
 */

struct coreaudio_private {
	AudioUnit unit;
	struct audio_queue queue;
	// int num_channels;
	int num_bytes_per_frame; // frame == time slice of sampling frequency
};

/*
 * This is the callback function which the audio device calls whenever it
 * wants some new blocks of data to process. For each data block in the
 * passed buffer list we dequeue the appropriate number of samples, padding
 * with zeroes in the case of an underrun. 
 */
static OSStatus
coreaudio_dsp_play(void 				*inRefCon, 
	AudioUnitRenderActionFlags 	*ioActionFlags, 
	const AudioTimeStamp 		*inTimeStamp, 
	UInt32 						inBusNumber, 
	UInt32 						inNumberFrames, 
	AudioBufferList 			*ioData)
{
	// inNumberFrames == requested number of samples per channel
	struct coreaudio_private *priv = (struct coreaudio_private*)inRefCon;
	// int num_channels = priv->num_channels;
	int num_bytes_per_frame = priv->num_bytes_per_frame;

	// when not kLinearPCMFormatFlagIsNonInterleaved
	/* in interleaved mode both the Left and the Right channels are
	   stored in the same buffer. */

	/* According to the headers there can only ever be one buffer in the list. */
	if(ioData->mNumberBuffers != 1){
		fprintf(stderr, "Error: This is unexpected. at line %d of %s.\n", __LINE__, __FILE__);
		return -1;
	}
	if(ioData->mBuffers[0].mDataByteSize != num_bytes_per_frame * inNumberFrames){
		fprintf(stderr, "Error: This is unexpected. at line %d of %s.\n", __LINE__, __FILE__);
		return -1;
	}

	dequeue_audio_block(&(priv->queue),
		ioData->mBuffers[0].mData,
		num_bytes_per_frame * inNumberFrames);

#if 0
	// when kLinearPCMFormatFlagIsNonInterleaved
	/* in non interleaved mode mBuffers[0] and mBuffers[1] represent
	   the Left and the Right channels respectively. */

	if(ioData->mNumberBuffers != 2){
		fprintf(stderr, "Error: This is unexpected. at line %d of %s.\n", __LINE__, __FILE__);
		return -1;
	}
	if(ioData->mBuffers[0].mDataByteSize != ioData->mBuffers[1].mDataByteSize){
		fprintf(stderr, "Error: This is unexpected. at line %d of %s.\n", __LINE__, __FILE__);
		return -1;
	}
	if(ioData->mBuffers[0].mDataByteSize != sizeof(float) * inNumberFrames){
		fprintf(stderr, "Error: This is unexpected. at line %d of %s.\n", __LINE__, __FILE__);
		return -1;
	}

	dequeue_audio_block(&(priv->queue),
		ioData->mBuffers[0].mData,
		ioData->mBuffers[1].mData,
		inNumberFrames);
#endif

	return noErr;
}

/*
 * Destroy function. We stop the sound device from playing and detach
 * ourselves from it. We do not free up the dsp structure as we dont
 * entriely trust OS X not to make a final call to the callback.
 */

static void
coreaudio_dsp_destroy(struct sysdep_dsp_struct *dsp)
{
	struct coreaudio_private *priv = (struct coreaudio_private*)dsp->_priv;
	OSStatus audio_err;

	verify_noerr (AudioOutputUnitStop (priv->unit));
	
    audio_err = AudioUnitUninitialize (priv->unit);
	if (audio_err) { printf ("AudioUnitUninitialize=%ld\n", audio_err); return; }

	CloseComponent (priv->unit);
}

static int
coreaudio_dsp_write(struct sysdep_dsp_struct *dsp,
		unsigned char *data, int count)
{
	struct coreaudio_private *priv = (struct coreaudio_private*)dsp->_priv;
	// int num_channels = priv->num_channels;
	int num_bytes_per_frame = priv->num_bytes_per_frame;
	SInt8 *data_block;
	
	/* make the data block */
	data_block = malloc(num_bytes_per_frame * count);
	if(!data_block) {
		fprintf(stderr, "out of memory queueing audio block");
		return count;
	}
	memcpy(data_block, data, num_bytes_per_frame * count);

	/* and queue it */
	queue_audio_block(&(priv->queue), data_block, num_bytes_per_frame * count);
	return count;
}

/*
 * Creation function. Attach ourselves to the default audio device if
 * we can, and set up the various parts of our internal structure.
 */

static void*
coreaudio_dsp_create(const void *flags)
{
	const struct sysdep_dsp_create_params *params = flags;
	struct sysdep_dsp_struct *dsp = NULL;
	struct coreaudio_private *priv = NULL;
	int samplerate, num_channels, num_bits, num_bytes_per_frame;
	OSStatus audio_err;
	UInt32 audio_count, audio_buff_len;
	// char audio_device[NAME_LEN];

	/* allocate the dsp struct */
	if (!(dsp = calloc(1, sizeof(struct sysdep_dsp_struct))))
	{
		perror("malloc failed for struct sysdep_dsp_struct\n");
		return NULL;
	}

	/* make the private structure */
	if (!(priv = calloc(1, sizeof(struct coreaudio_private)))) {
		perror("malloc failed for struct coreaudio_private\n");
		return NULL;
	}

	/* set up with queue, no device, and zero length buffer */
	// priv->unit = ???;
	priv->queue.head = NULL;
	priv->queue.tail = NULL;
	if(pthread_mutex_init(&(priv->queue.mutex),NULL)) {
		perror("failed to create mutex\n");
		return NULL;
	}

	samplerate = params->samplerate;
	// if(samplerate == 22050) samplerate =22000; // XXXX this makes CPS2 audio work finer.

	num_channels = (params->type & SYSDEP_DSP_STEREO) ? 2 : 1;
	num_bits = (params->type & SYSDEP_DSP_16BIT) ? 16 : 8;
	num_bytes_per_frame = num_bits / 8 * num_channels;
	priv->num_bytes_per_frame = num_bytes_per_frame;
	// priv->num_channels = num_channels;
	// priv->num_bits = num_bits;


	fprintf(stderr, "info: requesting %s %dbit sound at %d hz\n",
		num_channels == 1 ? "mono" : "stereo",
		num_bits, samplerate);

	{
		// Open the default output unit

		ComponentDescription desc;
		desc.componentType = kAudioUnitType_Output;
		desc.componentSubType = kAudioUnitSubType_DefaultOutput;
		desc.componentManufacturer = kAudioUnitManufacturer_Apple;
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;
	
		Component comp = FindNextComponent(NULL, &desc);
		if(comp == NULL){
		    fprintf(stderr, "Error: FindNextComponent()\n");
		    return NULL;
		}
	
		audio_err = OpenAComponent(comp, &priv->unit);
		if(audio_err != noErr){
		    fprintf(stderr, "Error: OpenAComponent(): %ld\n", audio_err);
		    return NULL;
		}

		// Set up a callback function to generate output to the output unit
		AURenderCallbackStruct input;
		input.inputProc = coreaudio_dsp_play;
		input.inputProcRefCon = (void *)priv;

		audio_err = AudioUnitSetProperty (priv->unit, 
			kAudioUnitProperty_SetRenderCallback, 
			kAudioUnitScope_Input,
			0, 
			&input, 
			sizeof(input));
		if(audio_err != noErr){
			fprintf(stderr, "Error: AudioUnitSetProperty(): CallBack: %ld\n", audio_err);
			return NULL;
		}
	}

	{
		AudioStreamBasicDescription format;
		format.mSampleRate = samplerate;
		format.mFormatID = kAudioFormatLinearPCM;
		format.mFormatFlags = 0 // kLinearPCMFormatFlagIsFloat 
			| kLinearPCMFormatFlagIsSignedInteger
			| kLinearPCMFormatFlagIsBigEndian
			| kLinearPCMFormatFlagIsPacked
			; // | kLinearPCMFormatFlagIsNonInterleaved;
		format.mBytesPerPacket = num_bytes_per_frame;
		format.mFramesPerPacket = 1;
		format.mBytesPerFrame = num_bytes_per_frame;
		format.mChannelsPerFrame = num_channels;
		format.mBitsPerChannel = num_bits;	

		audio_err = AudioUnitSetProperty (priv->unit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Input,
			0,
			&format,
			sizeof(format));
		if(audio_err != noErr){
			fprintf(stderr, "Error: AudioUnitSetProperty(): StreamFormat: '%4.4s', %ld\n",
					 (char *)&audio_err, audio_err);
			return NULL;
		}
	
		// Initialize unit
		audio_err = AudioUnitInitialize(priv->unit);
		if(audio_err != noErr){
			fprintf(stderr, "Error: AudioUnitInitialize(): %ld\n", audio_err);
			return NULL;
		}
    
#if 0
		Float64 out_samplerate;
		UInt32 size = sizeof(out_samplerate);
		audio_err = AudioUnitGetProperty (priv->unit,
			kAudioUnitProperty_SampleRate,
			kAudioUnitScope_Output,
			0,
			&out_samplerate,
			&size);
		if(audio_err != noErr){
			fprintf(stderr, "Error: AudioUnitGetProperty(): SampleRate: '%4.4s' %ld\n",
					(char *)&audio_err, audio_err);
			return NULL;
		}
		fprintf(stderr, "out_samplerate = %f\n", out_samplerate);
#endif

		audio_err = AudioOutputUnitStart (priv->unit);
		if(audio_err != noErr){
			fprintf(stderr, "Error: AudioOutputUnitStart(): %ld\n", audio_err);
			return NULL;
		}
	}

	/* fill in the functions and private data */
	dsp->_priv = priv;
	dsp->write = coreaudio_dsp_write;
	dsp->destroy = coreaudio_dsp_destroy;
	dsp->hw_info.type = params->type;
	dsp->hw_info.samplerate = samplerate;

	return dsp;
}

/*
 * The public variables structure
 */

const struct plugin_struct sysdep_dsp_coreaudio = {
	"coreaudio",
	"sysdep_dsp",
	"Apple OS X CoreAudio plugin",
	NULL, 				/* no options */
	NULL,				/* no init */
	NULL,				/* no exit */
	coreaudio_dsp_create,
	3				/* high priority */
};
