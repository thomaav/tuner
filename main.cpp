#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <algorithm>

#include "capture.h"
#include "wav.h"

constexpr fft_size = 4096;

void hamming_window(double *in, size_t nsamples)
{
	;
}

void fft(short *buf, size_t nsamples)
{
	double *in;
	fftw_complex *out;
	fftw_plan plan;

	in = fftw_alloc_real(nsamples);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (nsamples / 2 + 1));
	plan = fftw_plan_dft_r2c_1d(nsamples, in, out, FFTW_ESTIMATE);

	std::copy(buf, buf + nsamples, in);
	fftw_execute(plan);

	fftw_destroy_plan(plan);
	fftw_free(in); fftw_free(out);
}

int main(int argc, char *argv[1])
{
	int seconds = 1;
	size_t samples = 44100 * seconds;

	snd_pcm_t *pcm_handle = get_pcm_capture_handle();
	short *buf = new short[samples * snd_pcm_format_width(format) / 8];
	get_samples(pcm_handle, buf, samples);

	write_wav(buf, samples);

	fft(buf, samples);

	snd_pcm_close(pcm_handle);
	free(buf);
}
