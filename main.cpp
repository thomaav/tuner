#include <alsa/asoundlib.h>

#include "capture.h"
#include "wav.h"
#include "fft.h"

int main(int argc, char *argv[1])
{
	size_t samples_per_period = 11025;

	snd_pcm_t *pcm_handle = get_pcm_capture_handle();
	short *chunk = new short[samples_per_period * snd_pcm_format_width(format) / 8];

	for ( ;; ) {
		get_samples(pcm_handle, chunk, samples_per_period);
		fft(chunk, samples_per_period);
	}

	snd_pcm_close(pcm_handle);
	free(chunk);
}
