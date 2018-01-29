#include <alsa/asoundlib.h>

#include "capture.h"
#include "wav.h"

int main(int argc, char *argv[1])
{
	int seconds = 3;
	size_t samples = 44100 * seconds;

	snd_pcm_t *pcm_handle = get_pcm_capture_handle();
	short *buf = new short[samples * snd_pcm_format_width(format) / 8];
	get_samples(pcm_handle, buf, samples);

	write_wav(buf, samples);

	snd_pcm_close(pcm_handle);
	free(buf);
}
