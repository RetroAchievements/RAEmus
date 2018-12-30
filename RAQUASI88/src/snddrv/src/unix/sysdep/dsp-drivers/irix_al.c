/* Sysdep Irix_al sound dsp driver

   Copyright 2001 by Brandon Corey
   
   This file and the acompanying files in this directory are free software;
   you can redistribute them and/or modify them under the terms of the GNU
   Library General Public License as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   These files are distributed in the hope that they will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with these files; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Addendum: This file may also be used under the terms of the MAME license, by
   permission of Brandon Corey.  (For the text of the license, see 
   http://www.mame.net/readme.html.)
*/
/* Changelog
Version 0.1, April 15, 2001
- initial release
Version 0.2, April 15, 2001
- added sample frequency code so that a sample frequency other than the
  system default is usable
Version 0.3, August 25, 2004
- fixed 8-bit output and updated rate setup to deal with unsupported
  audio hardware rates
*/

/* Notes/Future
1) No mixer support
2) Use IRIX_DEBUG for Extra Info
3) Use FORCEMONO to force MONO output

Email: brandon@blackboxcentral.com
*/

/* #define IRIX_DEBUG */
/* #define IRIX_DEBUG_VERBOSE */
/* #define FORCEMONO */
/* #define SYSDEP_DSP_IRIX */

#ifdef SYSDEP_DSP_IRIX
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dmedia/audio.h>
#include "sysdep/sysdep_dsp.h"
#include "sysdep/sysdep_dsp_priv.h"
#include "sysdep/plugin_manager.h"

/* our per instance private data struct */
struct irix_dsp_priv_data {
    ALport devAudio;
    unsigned int buffer_samples;
    int sampwidth;
    int sampchan;
    int port_status;
};

/*
 * public methods prototypes (static but exported through the sysdep_dsp or
 * plugin struct)
 */
static void *irix_dsp_create(const void *flags);
static void irix_dsp_destroy(struct sysdep_dsp_struct *dsp);
static int irix_dsp_get_freespace(struct sysdep_dsp_struct *dsp);
static int irix_dsp_write(struct sysdep_dsp_struct *dsp, unsigned char *data,
                          int count);
static int RateSupported(int device, float streamrate);

/* public variables */
const struct plugin_struct sysdep_dsp_irix = {
   "irix_al",
   "sysdep_dsp",
   "IrixAL DSP plugin",
   NULL, /* no options */
   NULL, /* no init */
   NULL, /* no exit */
   irix_dsp_create,
   3     /* high priority as direct device access */
};


/*
 * public methods (static but exported through the sysdep_dsp or plugin struct)
 */
static void *
irix_dsp_create(const void *flags)
{
   ALpv pvs[4];
   long tempbits, tempchan;
   int oldrate;
   struct irix_dsp_priv_data *priv = NULL;
   struct sysdep_dsp_struct *dsp = NULL;
   const struct sysdep_dsp_create_params *params = flags;
   ALconfig devAudioConfig;	
   
   /* allocate the dsp struct */
   if (!(dsp = calloc(1, sizeof(struct sysdep_dsp_struct))))
   {
      fprintf(stderr, "Error: malloc failed for struct sysdep_dsp_struct\n"); 
      return NULL;
   }
   
   /* allocate private data */
   if(!(priv = calloc(1, sizeof(struct irix_dsp_priv_data))))
   {
      fprintf(stderr, "Error: malloc failed for struct irix_dsp_priv_data\n");
      free(dsp);
      return NULL;
   }
   
   /* fill in the functions and some data */
   priv->port_status = -1;
   dsp->_priv = priv;
   dsp->get_freespace = irix_dsp_get_freespace;
   dsp->write = irix_dsp_write;
   dsp->destroy = irix_dsp_destroy;
   dsp->hw_info.type = params->type;
   dsp->hw_info.samplerate = params->samplerate;

   tempchan = (dsp->hw_info.type & SYSDEP_DSP_STEREO) ? 2 : 1;
   tempbits = (dsp->hw_info.type & SYSDEP_DSP_16BIT) ? 2 : 1;

#ifdef IRIX_DEBUG
   fprintf(stderr, "Source Format is %dHz, %d bit, %s, with bufsize %f\n",
           dsp->hw_info.samplerate, 
           tempbits * 8, (tempchan == 2) ? "stereo" : "mono",
           params->bufsize);
#endif

   /*
    * Since AL wants signed data in either case, and 8-bit data from
    * core xmame is unsigned, let the core xmame convert everything
    * to 16-bit signed.
    */
   if (tempbits == 1)
   {
      dsp->hw_info.type |= SYSDEP_DSP_16BIT;
      tempbits = 2;
   }

   /*
    * Get the current hardware sampling rate
    */
   pvs[0].param = AL_RATE;
   if (alGetParams(AL_DEFAULT_OUTPUT, pvs, 1) < 0)
   {
      fprintf(stderr, "alGetParams failed: %s\n", alGetErrorString(oserror()));
      irix_dsp_destroy(dsp);
      return NULL;
   }

   oldrate = pvs[0].value.i;

   /*
    * If requested samplerate is different than current hardware rate,
    * set it.
    */
   if (oldrate != dsp->hw_info.samplerate)
   {
      int audioHardwareRate = oldrate;

      fprintf(stderr, "System sample rate was %dHz, forcing %dHz instead.\n",
              oldrate, dsp->hw_info.samplerate);

      /*
       * If the desired rate is unsupported, most devices (such as RAD) will
       * force the device rate to be as close as possible to the desired rate.
       * Since close isn't going to help us here, we avoid the call entirely,
       * and let core xmame audio convert to our rate.
       */
      if (RateSupported(AL_DEFAULT_OUTPUT, (float) dsp->hw_info.samplerate))
      {
         /* Set desired sample rate */
         pvs[0].param = AL_MASTER_CLOCK;
         pvs[0].value.i = AL_CRYSTAL_MCLK_TYPE;
         pvs[1].param = AL_RATE;
         pvs[1].value.i = dsp->hw_info.samplerate;
         alSetParams(AL_DEFAULT_OUTPUT, pvs, 2);

         /* Get the new sample rate */
         pvs[0].param = AL_RATE;
         if (alGetParams(AL_DEFAULT_OUTPUT, pvs, 1) < 0)
         {
            fprintf(stderr, "alGetParams failed: %s\n",
                    alGetErrorString(oserror()));
            irix_dsp_destroy(dsp);
            return NULL;
         }

         audioHardwareRate = pvs[0].value.i;
      }

      if (audioHardwareRate != dsp->hw_info.samplerate)
      {
         fprintf(stderr, "Requested rate of %dHz is not supported by "
                 "the audio hardware, so forcing\n"
                 "playback at %dHz.\n",
                 dsp->hw_info.samplerate, audioHardwareRate);
         dsp->hw_info.samplerate = audioHardwareRate;
      }
   }

   /* create a config descriptor */
   devAudioConfig = alNewConfig();
   if (devAudioConfig == NULL) {
      fprintf(stderr, "Cannot get a Descriptor. Exiting..\n");
      irix_dsp_destroy(dsp);
      return NULL;
   }

#ifdef FORCEMONO
   dsp->hw_info.type &= ~SYSDEP_DSP_STEREO;
   tempchan = 1;
#endif

   priv->buffer_samples = dsp->hw_info.samplerate * params->bufsize;

   priv->buffer_samples *= tempchan;

   priv->sampwidth = tempbits;
   priv->sampchan = tempchan;

   fprintf(stderr, "Setting sound to %dHz, %d bit, %s\n",
           dsp->hw_info.samplerate,
           tempbits * 8, (tempchan == 2) ? "stereo" : "mono");

   /* source specific audio parameters */
   alSetChannels(devAudioConfig, tempchan);
   alSetQueueSize(devAudioConfig, priv->buffer_samples);
   alSetWidth(devAudioConfig, tempbits);
   alSetSampFmt(devAudioConfig, AL_SAMPFMT_TWOSCOMP);

   /* Open the audio port with the parameters we setup */
   priv->devAudio = alOpenPort("audio_fd", "w", devAudioConfig);
   if (priv->devAudio == NULL)
   {
       fprintf(stderr, "Error: Cannot get an audio channel descriptor.\n");
       irix_dsp_destroy(dsp);
       return NULL;
   }

   alFreeConfig(devAudioConfig);

   /*
    * Since we don't use FD's with AL, we use this to inform us
    * of success
    */
   priv->port_status = 0;

   return dsp;
}

static void
irix_dsp_destroy(struct sysdep_dsp_struct *dsp)
{
   struct irix_dsp_priv_data *priv = dsp->_priv;
 
#ifdef IRIX_DEBUG 
   fprintf(stderr, "Destroying sound channel.\n");
#endif
 
   if (priv)
   {
      if (priv->port_status >= 0)
         alClosePort(priv->devAudio);

      free(priv);
   }

   free(dsp);
}


static int
irix_dsp_get_freespace(struct sysdep_dsp_struct *dsp)
{
   struct irix_dsp_priv_data *priv = dsp->_priv;

   return alGetFillable(priv->devAudio);;
}

   
static int
irix_dsp_write(struct sysdep_dsp_struct *dsp, unsigned char *data, int count)
{
   struct irix_dsp_priv_data *priv = dsp->_priv;
   int playcnt;
   int maxsize;

   /*
    * We write as many samples as possible (up to count) without blocking
    */
   maxsize = alGetFillable(priv->devAudio);

   playcnt = (count <= maxsize) ? count : maxsize;

   alWriteFrames(priv->devAudio, data, playcnt);

   return playcnt;
}


static int
RateSupported(int device, float streamrate)
{
   ALparamInfo pinfo;
   float minrate, maxrate;

   /* Find rate range for specified device */
   alGetParamInfo(device, AL_RATE, &pinfo);

   minrate = (float) alFixedToDouble(pinfo.min.ll);
   maxrate = (float) alFixedToDouble(pinfo.max.ll);

   if ((streamrate >= minrate) && (streamrate <= maxrate))
      return 1;

   return 0;
}

#endif /* ifdef SYSDEP_DSP_IRIX */

