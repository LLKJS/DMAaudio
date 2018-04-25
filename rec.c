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



int main(int argc, char *argv[])
{
	snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    int option;

    int err, size, dir;
    unsigned int val;
    long loops;
    char *buffer;
    while((option = getopt(argc, argv, "r:")) != -1) {
    	switch(option) {
    	case 'r':
    		sscanf(optarg, "%i", &rate);
    		break;
    	default:
    		break;
    	}
    }

    fprintf(stderr, "Capture device is %s\n", device);
    fprintf(stderr, "Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);


    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            fprintf(stderr, "Capture open error: %s\n", snd_strerror(err));
            return 0;
    }

    if ((err = set_hwparams(handle, hwparams, acc)) < 0) {
            fprintf(stderr, "Setting of hwparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
    }
    if ((err = set_swparams(handle, swparams)) < 0) {
            fprintf(stderr, "Setting of swparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(hwparams, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);
    fprintf(stderr, "frames %d\n", frames);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(hwparams, &val, &dir);

    loops = 500000000;
    frames = 80;
    size =  frames * 4;

    while (loops > 0)
    {
    	loops--;
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
            fprintf(stderr, "Unable to set hw params for playback: %s\n", snd_strerror(err));
            return err;
    }
    return 0;
}

int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	return 0;
}

