#include <alsa/asoundlib.h>

#include "capture.h"

unsigned int PCMDevice::sample_rate = 44100;
constexpr char PCMDevice::pcm_name[];

PCMDevice::PCMDevice()
{
	int err;

	/* open PCM */
	if ((err = snd_pcm_open(&pcm_handle, pcm_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
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
	if ((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to initialize hw parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* set the access mode of the hw parameters */
	if ((err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "Unable to set access type (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* set the sample format of the hardware parameters */
	if ((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, format)) < 0) {
		fprintf(stderr, "Unable to set sample format (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* set sampling rate */
	if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &sample_rate, 0)) < 0) {
		fprintf(stderr, "Unable to set sample rate (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* set sampling channels */
	if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 1)) < 0) {
		fprintf(stderr, "Unable to set channel count (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	/* actually set the hw_params */
	if ((err = snd_pcm_hw_params(pcm_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to set hardware parameters (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	printf("Audio interface prepared\n");
}

PCMDevice::~PCMDevice()
{
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_close(pcm_handle);
}

void PCMDevice::get_samples(short *chunk, size_t nsamples)
{
	int err;

	while (nsamples) {
		err = snd_pcm_readi(pcm_handle, chunk, nsamples);

		if (!err) {
			printf("End of device: (%s)\n", pcm_name);
			exit(1);
		}

		if (err < 0) {
			fprintf(stderr, "Read from audio interface %s failed: (%s)\n",
				pcm_name, snd_strerror(err));
			exit(1);
		}

		chunk += err;
		nsamples -= err;
	}
}
