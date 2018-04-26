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

void print_help();
int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
void read_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t frames);
void read_mmap_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t frames);
void read_mmap();

#endif
