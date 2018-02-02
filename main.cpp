#include <alsa/asoundlib.h>

#include "capture.h"
#include "wav.h"
#include "fft.h"
#include "notes.h"

int main(int argc, char *argv[1])
{
	size_t samples_per_period = 11025;

	PCMDevice pcm_device{};
	short *chunk = new short[samples_per_period * snd_pcm_format_width(format) / 8];

	for ( ;; ) {
		pcm_device.get_samples(chunk, samples_per_period);
		printf("%f\n", fft_median_peak_frequency(chunk, samples_per_period));
	}

	free(chunk);
}
