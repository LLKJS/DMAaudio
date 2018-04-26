/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Program to record audio via ALSA
 * This software is designed to record audio
 * with minimal delay using memory mapped
 * ALSA functions
 *
 * Copyright 2018 LL
 */
#include <unistd.h>
#include <stdio.h>
#include "rec.h"

#define ALSA_PCM_NEW_HW_PARAMS_API

static char *device = "hw:0,1";                         /* playback device */
static snd_pcm_access_t acc = SND_PCM_ACCESS_MMAP_INTERLEAVED; /* access mode */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */
static unsigned int rate = 16000;                       /* stream rate */
static unsigned int channels = 1;                       /* count of channels */
snd_pcm_uframes_t frames;
int frame_size,size;


enum MODE {
	NORMAL,
	MMAP_NORMAL,
	MMAP
};

int main(int argc, char *argv[])
{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);
	int option,help;
	int mode = NORMAL;

	int err,dir;
	unsigned int val;
	char *buffer;
	while((option = getopt(argc, argv, "c:m:r:h")) != -1) {
		switch(option) {
		case 'c':
			sscanf(optarg, "%i", &channels);
			break;
		case 'r':
			sscanf(optarg, "%i", &rate);
			break;
		case 'm':
			sscanf(optarg, "%i", &mode);
		case 'h':
			print_help();
		default:
			break;
		}
	}

	fprintf(stderr, "Capture device is %s\n", device);
	fprintf(stderr, "Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);

    /*Start PCM device*/
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            fprintf(stderr, "Capture open error: %s\n", snd_strerror(err));
            return 0;
    }
	/*Handle access mode*/
	switch(mode){
	case NORMAL:
		fprintf(stderr, "Normal mode - ACCESS_RW_INTERLEAVED\n");
		acc = SND_PCM_ACCESS_RW_INTERLEAVED;
		break;
	case MMAP_NORMAL:
		fprintf(stderr, "Normal mode - ACCESS_MMAP_INTERLEAVED\n");
		acc = SND_PCM_ACCESS_MMAP_INTERLEAVED;
		break;
	case MMAP:
		fprintf(stderr, "Normal mode - ACCESS_MMAP_INTERLEAVED\n");
		acc = SND_PCM_ACCESS_MMAP_INTERLEAVED;
		break;
	}
	/*Configure PCM hardware*/
	if ((err = set_hwparams(handle, hwparams, acc)) < 0) {
		fprintf(stderr, "Setting of hwparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	/*Configure PCM Software*/
	if ((err = set_swparams(handle, swparams)) < 0) {
		fprintf(stderr, "Setting of swparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(hwparams, &frames, &dir);

	buffer = (char *) malloc(size);
	fprintf(stderr, "frames %d\n", frames);

	/* We want to loop for 5 seconds */
	snd_pcm_hw_params_get_period_time(hwparams, &val, &dir);
	frame_size = 2 /* only for S16 samples */ * channels;
	size = frames * frame_size; /* 2 bytes/sample, 2 channels */
	frames = 80;
	size =  frames * frame_size;
	switch(mode){
	case NORMAL:
		read_normal(handle, buffer, frames);
		break;
	case MMAP_NORMAL:
		read_mmap_normal(handle, buffer, frames);
		break;
	case MMAP:
		read_mmap();
		break;
	}

      snd_pcm_drain(handle);
      snd_pcm_close(handle);
      free(buffer);

	return 0;
}

int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access)
{
	int err,dir;
	int val = rate;

	/* Fill it in with default values. */
	err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
    	fprintf(stderr, "Error filling default params: %s\n", snd_strerror(err));
    	return err;
    }

    /* Set the desired hardware parameters. */
	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0)
    {
    	fprintf(stderr, "Error setting access: %s\n", snd_strerror(err));
    	return err;
    }
	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0)
    {
    	fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
    	return err;
    }
	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0)
    {
    	fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
    	return err;
    }
	/*  */
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if (err < 0)
    {
    	fprintf(stderr, "Error setting rate: %s\n", snd_strerror(err));
    	return err;
    }
	/* Set period size to 32 frames. */
	frames = 4;
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (err < 0)
    {
    	fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
    	return err;
    }
	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
            fprintf(stderr, "Unable to set hw params for capture: %s\n", snd_strerror(err));
            return err;
    }
    return 0;
}

int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	return 0;
}

void read_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t frames )
{
	int err;
	long repetitions = 500000000;
	while (repetitions > 0)
	{
		repetitions--;
		err = snd_pcm_readi(handle, buffer, frames);
		if (err == -EPIPE)
		{
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occurred: %s\n",snd_strerror(err));
			snd_pcm_prepare(handle);
		}
		else if (err < 0)
		{
			fprintf(stderr, "error from read: %s\n",snd_strerror(err));
		}
		else if (err != (int)frames)
		{
			fprintf(stderr, "short read, read %d frames\n", err);
		}
		err = write(1, buffer, size);
		if (err != size)
		{
			fprintf(stderr, "short write: wrote %d bytes\n", err);
		}
	}
}

void read_mmap_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t frames)
{
	int err;
	long repetitions = 500000000;
	while (repetitions > 0)
	{
		repetitions--;
		err = snd_pcm_mmap_readi(handle, buffer, frames);
		if (err == -EPIPE)
		{
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occurred: %s\n",snd_strerror(err));
			snd_pcm_prepare(handle);
		}
		else if (err < 0)
		{
			fprintf(stderr, "error from read: %s\n",snd_strerror(err));
		}
		else if (err != (int)frames)
		{
			fprintf(stderr, "short read, read %d frames\n", err);
		}
		err = write(1, buffer, size);
		if (err != size)
		{
			fprintf(stderr, "short write: wrote %d bytes\n", err);
		}
	}
}

void read_mmap()
{

}

void print_help()
{
	printf(
			"Usage: [OPTION] > [FILE]\n"
			"-h,      help\n"
			"-m,      operational modes: 0 (normal), 1 (normal_mmap), 2 (mmap)\n"
			"-r,      sampling rate in Hz\n"
			"-c,      count of channels in stream\n"
			"\n");
	exit(0);
}
