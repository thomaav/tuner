#include <stdio.h>
#include <alsa/asoundlib.h>

int main(int argc, char **argv)
{
	int err;

	unsigned int sampling_rate = 44100;

	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // signed 16-bit little endian

	/* open PCM */
	if ((err = snd_pcm_open(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf(stderr, "Unable to open audio device %s (%s)\n",
			argv[1],
			snd_strerror(err));
		exit(1);
	}

	printf("Audio interface %s\ opened\n", argv[1]);

	/* allocate PCM hardware parameters */
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "Unable to allocate hw parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	printf("Hardware params allocated\n");

	/* initialize the PCM hardware parameters */
	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to initialize hw parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	printf("Hardware params initialized\n");

	/* set the access mode of the hw parameters */
	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "Unable to set access type (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	printf("Hardware params access mode set\n");

	/* set the sample format of the hardware parameters */
	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
		fprintf(stderr, "Unable to set sample format (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	printf("Hardware sample format set\n");

	/* set sampling rate */
	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sampling_rate, 0)) < 0) {
		fprintf(stderr, "Unable to set sample rate (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	printf("Hardware sample rate set\n");

	/* set sampling channels */
	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 2)) < 0) {
		fprintf(stderr, "Unable to set channel count (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	printf("Hardware sample channel count set\n");

	/* actually set the hw_params */
	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "Unable to set hardware parameters (%s)\n",
			snd_strerror(err));
		exit(1);

	}

	printf("Hardware parameters set\n");

	snd_pcm_hw_params_free(hw_params);
	printf("hw_parms freed\n");

	printf("Audio interface prepared\n");
}
