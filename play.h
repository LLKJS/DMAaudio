#ifndef __PLAY_H__
#define __PLAY_H__

#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>

void print_help();
int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access, snd_pcm_uframes_t period_size);
int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
int xrun_recovery(snd_pcm_t *handle, int err);

void write_normal();
void write_mmap_normal();
void write_mmap();

#endif
