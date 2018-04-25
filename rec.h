#ifndef __REC_H__
#define __REC_H__

#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

static char *device = "hw:0,1";                    		/* playback device */
static snd_pcm_access_t acc = SND_PCM_ACCESS_RW_INTERLEAVED; /* access mode */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */
static unsigned int rate = 16000;                       /* stream rate */
static unsigned int channels = 1;                       /* count of channels */
snd_pcm_uframes_t frames;

int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);

#endif
