/*
 * Test to  record and play audio data in the same format
 * as it's going to be used in the final project.
 * Direct access using mmap functions for decreased
 * latencies in the audio processing path.
 *
 * Codec is G.711 therefore 8kHz sampling rate
 * and unsigned 8 bit format mono recording.
 *
 * Default recording device on the dev platform
 * is "hw:0,1".
 * Default playback device on the dev platform
 * is "hw:0,0".
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>

#include <alsa/asoundlib.h>

static char *rdevice = "plughw:0,1";                         /* recording device */
static char *pdevice = "plughw:0,0";                         /* playback device */
static snd_pcm_access_t acc = SND_PCM_ACCESS_MMAP_NONINTERLEAVED; /* access mode */
static snd_pcm_format_t format = SND_PCM_FORMAT_U8 ;    /* sample format */
static unsigned int rate = 8000;                        /* stream rate */
static unsigned int channels = 1;                       /* count of channels */
static snd_pcm_sframes_t period_size = 80;              /* 10ms period size */

enum MODE {
	RECORD,
	PLAY
};
/**
 * \brief Set the pcm hardware parameters from global variables
 * \param *pcm PCM handle
 * \param *params hw parameters
 * \return 0 upon success, otherwise a negative error code
 *
 * All settings are predefined in global variables beforehand!
 *
 */
int set_hwparams(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int err,dir;
	int val = rate;

	/* Fill it in with default values. */
	err = snd_pcm_hw_params_any(pcm, params);
	if (err < 0)
	{
		fprintf(stderr, "Error filling default params: %s\n", snd_strerror(err));
		return err;
	}
	/* Set the desired hardware parameters. */
	/* Set the predefined access mode */
	snd_pcm_hw_params_set_access(pcm, params, acc);
	if (err < 0)
	{
		fprintf(stderr, "Error setting access: %s\n", snd_strerror(err));
		return err;
	}
	/* Set the predefined format*/
	snd_pcm_hw_params_set_format(pcm, params, format);
	if (err < 0)
	{
		fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
		return err;
	}
	/* Set the predefined number of channels */
	snd_pcm_hw_params_set_channels(pcm, params, channels);
	if (err < 0)
	{
		fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
		return err;
	}
	/* Set the predefined sampling rate*/
	snd_pcm_hw_params_set_rate_near(pcm, params, &val, &dir);
	if (err < 0)
	{
		fprintf(stderr, "Error setting rate: %s\n", snd_strerror(err));
		return err;
	}
	/* Set the predefined period size */
	snd_pcm_hw_params_set_period_size(pcm, params, &period_size, &dir);
	if (err < 0)
	{
		fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
		return err;
	}
	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(pcm, params);
	if (err < 0) {
		fprintf(stderr, "Unable to write hw params to pcm device: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

/**
 * \brief Set the pcm software parameters from global variables
 * \param *pcm PCM handle
 * \param *params sw parameters
 * \return 0 upon success, otherwise a negative error code
 *
 * All settings are predefined in global variables beforehand!
 *
 */
int set_swparams(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
	return 0;
}

/**
 * \brief Read audio data directly from PCM device to audio buffer
 * \param *pcm PCM handle
 * \param *bufs Audio buffer locations
 * \param size Number of frames to read
 * \return number of read frames upon success, otherwise a negative error code
 *
 * Directly access the PCM device, requires the access mode to be
 * SND_PCM_ACCESS_MMAP_NONINTERLEAVED
 */
snd_pcm_sframes_t dma_read(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{	/*
	// create areas (one per channel)
	snd_pcm_channel_area_t areas[channels];
	// set these areas to be where the buffer is
	// all further operations are handled with these areas
	snd_pcm_areas_from_bufs(pcm, areas, bufs);
	// read areas
	// This function handles the pcm states and makes use of the
	// snd_mmap_begin, snd_pcm_areas_copy and snd_ocm_mmap_commit
	// functions intended for the use with direct memory access.
	return snd_pcm_read_areas(pcm, areas, 0, size,snd_pcm_mmap_read_areas);
	*/
	return snd_pcm_mmap_readn(pcm, bufs, size);
}

/**
 * \brief Write audio data directly from audio buffer to PCM device
 * \param *pcm PCM handle
 * \param *bufs Audio buffer locations
 * \param size Number of frames to read
 * \return number of read frames upon success, otherwise a negative error code
 *
 * Directly access the PCM device, requires the access mode to be
 * SND_PCM_ACCESS_MMAP_NONINTERLEAVED
 */
snd_pcm_sframes_t dma_write(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{	/*
	// create areas (one per channel)
	snd_pcm_channel_area_t areas[channels];
	// set these areas to be where the buffer is
	// all further operations are handled with these areas
	snd_pcm_areas_from_bufs(pcm, areas, bufs);
	// read areas
	// This function handles the pcm states and makes use of the
	// snd_mmap_begin, snd_pcm_areas_copy and snd_ocm_mmap_commit
	// functions intended for the use with direct memory access.
	return snd_pcm_write_areas(pcm, areas, 0, size,snd_pcm_mmap_read_areas);
	*/
	return snd_pcm_mmap_writen(pcm, bufs, size);
}
int main(int argc, char *argv[])
{
	// PCM
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);
	// Audio buffer
	char *buffer;
	//other
	int err,option;
	char *device;
	int mode = RECORD;
	while((option = getopt(argc, argv, "m:")) != -1)
	{
		switch(option) {
		case 'm':
			sscanf(optarg, "%i", &mode);
			break;
		default:
			break;
		}
	}

	if(mode == RECORD)
	{
		device = rdevice;
		fprintf(stderr, "device is %s\n", device);
		/*Start PCM device*/
		if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
		{
			fprintf(stderr, "device open error: %s\n", snd_strerror(err));
			return 0;
		}
	}
	if(mode == PLAY)
	{
		device = pdevice;
		fprintf(stderr, "device is %s\n", device);
		/*Start PCM device*/
		if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
		{
			fprintf(stderr, "device open error: %s\n", snd_strerror(err));
			return 0;
		}
	}
	/*Configure PCM hardware*/
	if ((err = set_hwparams(handle, hwparams)) < 0) {
		fprintf(stderr, "Setting of hwparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	/*Configure PCM Software*/
	if ((err = set_swparams(handle, swparams)) < 0) {
		fprintf(stderr, "Setting of swparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	buffer = (char *) malloc(period_size);
	fprintf(stderr, "buffer size in frames %d\n", period_size);

	if(mode == RECORD)
	{
		while(1)
		{
			err = dma_read(handle, buffer, period_size);
			if (err < 0)
			{
				fprintf(stderr, "error while reading: %s\n",snd_strerror(err));
			}
			else if (err != (int)period_size)
			{
				fprintf(stderr, "short read, read %d frames\n", err);
			}
			err = write(1, buffer, period_size);
			if (err != period_size)
			{
				fprintf(stderr, "short write: wrote %d bytes\n", err);
			}
		}
	}
	if(mode == PLAY)
	{
		while(1)
		{
			err = read(0, buffer, period_size);
			if (err == 0)
			{
				fprintf(stderr, "end of file on input\n");
				break;
			}
			else if (err != period_size)
			{
				fprintf(stderr,"short read: read %d bytes\n", err);
			}
			err = dma_write(handle, buffer, period_size);
			if (err < 0)
			{
				fprintf(stderr, "error while writing: %s\n",snd_strerror(err));
			}
			else if (err != (int)period_size)
			{
				fprintf(stderr, "short read, read %d frames\n", err);
			}
		}
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);

	return 0;
}
