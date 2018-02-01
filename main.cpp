#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <algorithm>
#include <cmath>

#include "capture.h"
#include "wav.h"

// hamming window
constexpr double alpha = 0.54;
constexpr double beta  = 0.46;

constexpr int fft_size = 4096;
constexpr int useful_bins = fft_size / 2;
constexpr int step_size = fft_size / 2; // 50% window overlap

double * hamming(int fft_size)
{
	double *window = new double[fft_size];

	for (int i = 0; i < fft_size; ++i) {
		window[i] = alpha - beta * cos(2.0 * M_PI * double(i) / double(fft_size - 1));
	}

	return window;
}

void fft(short *samples, size_t nsamples)
{
	double *in;
	fftw_complex *out;
	double processed[useful_bins];
	fftw_plan plan;

	in = fftw_alloc_real(fft_size);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * fft_size);
	plan = fftw_plan_dft_r2c_1d(fft_size, in, out, FFTW_ESTIMATE);

	double *hamming_window = hamming(fft_size);

	for (int x = 0; x < nsamples / step_size; ++x) {
		// process all samples of one FFT period, i.e. period
		// of fft_size amount of samples, windows will overlap
		// 50% by using step_size as fft_size / 2
		for (int i = 0, j = x * step_size; j < x * step_size + fft_size; ++i, ++j) {
			in[i] = samples[j] * hamming_window[i];
		}

		fftw_execute(plan);

		// process values into values relevant to create
		// spectogram, only the first fft_size / 2 bins are
		// useful
		for (int i = 0; i < useful_bins; ++i) {
			out[i][0] *= (2.0 / fft_size);
			out[i][1] *= (2.0 / fft_size);
			processed[i] = out[i][0] * out[i][0] + out[i][1] * out[i][1];
			processed[i] = 10.0 / log(10.0) * log(processed[i] + 1e-6);

			// normalize values in range 0dB to 96dB to be values
			// between 0 and 1
			processed[i] = fmax(0, processed[i]);
			processed[i] = fmax(1, processed[i] / 96.0);
		}
	}

	fftw_destroy_plan(plan);
	fftw_free(in); fftw_free(out);
}

int main(int argc, char *argv[1])
{
	size_t samples = 44100;

	snd_pcm_t *pcm_handle = get_pcm_capture_handle();
	short *chunk = new short[samples * snd_pcm_format_width(format) / 8];
	get_samples(pcm_handle, chunk, samples);

	write_wav(chunk, samples);

	fft(chunk, samples);

	snd_pcm_close(pcm_handle);
	free(chunk);
}
