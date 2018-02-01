#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <algorithm>
#include <cmath>
#include <utility>

#include "capture.h"
#include "wav.h"

// hamming window
constexpr double alpha = 0.54;
constexpr double beta  = 0.46;

constexpr int fft_size = 1024;
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

std::pair<int, double> find_peak_frequency(double *bins, int frame_size)
{
	int peak_bin = 0;
	double peak = bins[peak_bin];

	for (int i = 0; i < frame_size; ++i) {
		if (bins[i] > peak) {
			peak = bins[i];
			peak_bin = i;
		}
	}

	return std::make_pair(peak_bin, peak);
}

void fft(short *samples, size_t nsamples)
{
	double *in;
	fftw_complex *out;
	double bins[useful_bins];
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
		// spectrogram, only the first fft_size / 2 bins are
		// useful
		for (int i = 0; i < useful_bins; ++i) {
			out[i][0] *= (2.0 / fft_size);
			out[i][1] *= (2.0 / fft_size);
			bins[i] = out[i][0] * out[i][0] + out[i][1] * out[i][1];
			bins[i] = 10.0 / log(10.0) * log(bins[i] + 1e-6);

			// normalize values in range 0dB to 96dB to be values
			// between 0 and 1
			bins[i] = fmax(0, bins[i]);
			bins[i] = fmax(1, bins[i] / 96.0);
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
