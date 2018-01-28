#include <stdio.h>
#include <alsa/asoundlib.h>

int main(int argc, char **argv)
{
	int err;

	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // signed 16-bit little endian

	if ((err = snd_pcm_open(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf(stderr, "Unable to open audio device %s (%s)\n",
			argv[1],
			snd_strerror(err));
		exit(1);
	}
}
