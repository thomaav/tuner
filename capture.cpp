#include <alsa/asoundlib.h>

#include "capture.h"

snd_pcm_t *get_pcm_capture_handle()
{
	int err;

	unsigned int sampling_rate = 44100;

	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;

	/* open PCM */
	if ((err = snd_pcm_open(&capture_handle, pcm_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf(stderr, "Unable to open audio device %s (%s)\n",
			pcm_name,
			snd_strerror(err));
		exit(1);
	}

	/* allocate PCM hardware parameters */
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "Unable to allocate hw parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* initialize the PCM hardware parameters */
	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to initialize hw parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* set the access mode of the hw parameters */
	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "Unable to set access type (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	/* set the sample format of the hardware parameters */
	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
		fprintf(stderr, "Unable to set sample format (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	/* set sampling rate */
	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sampling_rate, 0)) < 0) {
		fprintf(stderr, "Unable to set sample rate (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	/* set sampling channels */
	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1)) < 0) {
		fprintf(stderr, "Unable to set channel count (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	/* actually set the hw_params */
	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to set hardware parameters (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	snd_pcm_hw_params_free(hw_params);

	printf("Audio interface prepared\n");

	return capture_handle;
}

void get_samples(snd_pcm_t *handle, short *buf, size_t nsamples)
{
	int err;

	while (nsamples) {
		err = snd_pcm_readi(handle, buf, nsamples);

		if (!err) {
			printf("End of device: (%s)\n", pcm_name);
			exit(1);
		}

		if (err < 0) {
			fprintf(stderr, "Read from audio interface %s failed: (%s)\n", pcm_name, snd_strerror(err));
			exit(1);
		}

		buf += err;
		nsamples -= err;
	}
}