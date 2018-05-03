/*
 * Program to play audio via ALSA
 */

#include <alsa/asoundlib.h>
#include "play.h"


static char *device = "plughw:0,0";                                /* playback device */
static snd_pcm_access_t acc = SND_PCM_ACCESS_RW_INTERLEAVED; /* access mode */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;           /* sample format */
static unsigned int rate = 16000;                              /* stream rate */
static unsigned int channels = 1;                              /* count of channels */

int debug = 5;

enum MODE {
	NORMAL,
	MMAP_NORMAL,
	MMAP
};

int main(int argc, char *argv[])
{
	int option,err,dir,size;
	int mode = NORMAL;

	char *buffer;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);
	snd_pcm_uframes_t period_size = 80;//CHECKME with recording - should be the same
	/*Handle parameters*/
	while((option = getopt(argc, argv, "c:d:m:r:h")) != -1)
	{
			switch(option) {
			case 'c':
				sscanf(optarg, "%i", &channels);
				break;
			case 'd':
				sscanf(optarg, "%i", &debug);
				break;
			case 'r':
				sscanf(optarg, "%i", &rate);
				break;
			case 'm':
				sscanf(optarg, "%i", &mode);
				break;
			case 'h':
				print_help();
				break;
			default:
				break;
			}
	}
	/*Start PCM device*/
	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
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
	if ((err = set_hwparams(handle, hwparams, acc, period_size)) < 0) {
		fprintf(stderr, "Setting of hwparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	/*Configure PCM Software*/
	if ((err = set_swparams(handle, swparams)) < 0) {
		fprintf(stderr, "Setting of swparams failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Playback device is %s\n", device);
	fprintf(stderr, "Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);

	/*Play audio file*/
	snd_pcm_hw_params_get_period_size(hwparams, &period_size,&dir);
	size = period_size * channels * 2; //2 bytes per sample per channel per frame
	buffer = (char *) malloc(size);
	fprintf(stderr, "frames in period: %d\n", period_size);
	switch(mode){
		case NORMAL:
			write_normal(handle, buffer, period_size);
			break;
		case MMAP_NORMAL:
			write_mmap_normal();
			break;
		case MMAP:
			write_mmap();
			break;
		}
	/*End audio*/
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);

	return 0;
}

int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params,snd_pcm_access_t access, snd_pcm_uframes_t period_size)
{
	int err, dir;
	/* choose all parameters */
	fprintf(stderr, "now setting HW params\n");
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		fprintf(stderr, "Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set default\n");
	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if (err < 0)
	{
		fprintf(stderr, "Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set access type\n");
	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0)
	{
		fprintf(stderr, "Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set format\n");
	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0)
	{
		fprintf(stderr, "Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set channel count\n");
	/* set the stream rate */
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	if (err < 0)
	{
		fprintf(stderr, "Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set sampling rate\n");
	err = snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, &dir);
	if (err < 0)
	{
		fprintf(stderr, "Unable to get period size for playback: %s\n", snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set period size\n");
	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0)
	{
		fprintf(stderr, "Unable to set hw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	fprintf(stderr, "HW params set\n");
	return 0;
}
int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	return 0;
}
/*
 *   Underrun and suspend recovery
 */

int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE)/* under-run */
	{
		err = snd_pcm_prepare(handle);
		if (err < 0)
			fprintf(stderr, "Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	}
	else if (err == -ESTRPIPE)
	{
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);       /* wait until the suspend flag is released */
		if (err < 0)
		{
			err = snd_pcm_prepare(handle);
			if (err < 0)
				fprintf(stderr, "Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

void print_help()
{
	printf(
			"Usage: [OPTION] > [FILE]\n"
			"-h,      help\n"
			"-m,      operational modes: 0 (normal), 1 (normal_mmap), 2 (mmap)\n"
			"\n");
	exit(0);
}

void write_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t period_size)
{
	int err;
	while(1)
	{
		/*read from file*/
		err = read (0,buffer, period_size);
		if (err == 0)
		{
			fprintf(stderr, "end of file on input\n");
			break;
		}
		else if (err != period_size)
		{
			fprintf(stderr,"short read: read %d bytes\n", err);
		}
		/*write to soundcard*/
		err = snd_pcm_writei(handle, buffer, period_size);
		if (err < 0)
		{
			if (err = xrun_recovery(handle,err)<0)
			{
				fprintf(stderr, "xrun recovery failed: %s\n", snd_strerror(err));
				exit(EXIT_FAILURE);
			}
		}
		if (err != period_size)
		{
			printf("Write error: written %i expected %li\n", err, period_size);
			exit(EXIT_FAILURE);
		}
	}
}

void write_mmap_normal(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t period_size)
{
	int err;
	while(1)
	{
		/*read from file*/
		err = read (0,buffer, period_size);
		if (err == 0)
		{
			fprintf(stderr, "end of file on input\n");
			break;
		}
		else if (err != period_size)
		{
			fprintf(stderr,"short read: read %d bytes\n", err);
		}
		/*write to soundcard*/
		err = snd_pcm_mmap_writei(handle, buffer, period_size);
		if(err < 0)
		{
			if(err = xrun_recovery(handle,err)<0)
			{
				fprintf(stderr, "xrun recovery failed: %s\n", snd_strerror(err));
				exit(EXIT_FAILURE);
			}
		}
		if(err != period_size)
		{
			printf("Write error: written %i expected %li\n", err, period_size);
			exit(EXIT_FAILURE);
		}
	}
}

void write_mmap()
{

}
