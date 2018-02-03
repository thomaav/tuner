#include <alsa/asoundlib.h>
#include <signal.h>

#include "capture.h"
#include "wav.h"
#include "fft.h"
#include "notes.h"

static volatile int run = 1;

void enable_cursor()
{
	printf("\e[?25h");
	fflush(stdout);
}

void disable_cursor()
{
	printf("\e[?25l");
	fflush(stdout);
}

void sigbye(int _)
{
	run = 0;
}

int main(int argc, char *argv[1])
{
	size_t samples_per_period = 11025;
	double peak;

	PCMDevice pcm_device{};
	short *chunk = new short[samples_per_period * snd_pcm_format_width(PCMDevice::format) / 8];

	disable_cursor();
	signal(SIGINT, sigbye);
	signal(SIGTERM, sigbye);
	printf("\n\n\n");

	while (run) {
		pcm_device.get_samples(chunk, samples_per_period);
		peak = fft_median_peak_frequency(chunk, samples_per_period);

		if (peak < 60.0)
			peak = 0.0;

		printf("Peak frequency: %f\r", peak);
		fflush(stdout);
	}

	enable_cursor();
	free(chunk);
}
