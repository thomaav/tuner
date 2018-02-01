#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "capture.h"
#include "wav.h"

// hamming window
constexpr double alpha = 0.54;
constexpr double beta  = 0.46;

constexpr int sample_rate = 44100;
constexpr int fft_size = 1024;
constexpr int num_bins = fft_size / 2;
constexpr int step_size = fft_size / 2; // 50% window overlap

double * hamming(int fft_size)
{
	double *window = new double[fft_size];

	for (int i = 0; i < fft_size; ++i) {
		window[i] = alpha - beta * cos(2.0 * M_PI * double(i) / double(fft_size - 1));
	}

	return window;
}

double bin_to_frequency(int bin, int sample_rate, int frame_size)
{
	return double(bin) * sample_rate / double(frame_size);
}

double find_peak_frequency(double *bins, int frame_size)
{
	int peak_bin = 0;
	double peak = bins[peak_bin];

	for (int i = 0; i < frame_size; ++i) {
		if (bins[i] > peak) {
			peak = bins[i];
			peak_bin = i;
		}
	}

	return bin_to_frequency(peak_bin, sample_rate, frame_size);
}

double find_median_frequency(std::vector<double> &peaks)
{
	size_t n = peaks.size() / 2;
	std::nth_element(peaks.begin(), peaks.begin() + n, peaks.end());
	return peaks[n];
}

void fft(short *samples, size_t nsamples)
{
	double peak;
	std::vector<double> peaks;

	double *in;
	fftw_complex *out;
	double bins[num_bins];
	fftw_plan plan;

	double *hamming_window = hamming(fft_size);

	in = fftw_alloc_real(fft_size);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * fft_size);
	plan = fftw_plan_dft_r2c_1d(fft_size, in, out, FFTW_ESTIMATE);

	for (int x = 0; x < nsamples / step_size; ++x) {
		// process all samples of one FFT period, i.e. period
		// of fft_size amount of samples, windows will overlap
		// 50% by using step_size as fft_size / 2
		for (int i = 0, j = x * step_size; j < x * step_size + fft_size; ++i, ++j)
			in[i] = samples[j] * hamming_window[i];

		fftw_execute(plan);

		// process values into values relevant to create
		// spectrogram, only the first fft_size / 2 bins are
		// useful
		for (int i = 0; i < num_bins; ++i) {
			out[i][0] *= (2.0 / fft_size);
			out[i][1] *= (2.0 / fft_size);
			bins[i] = out[i][0] * out[i][0] + out[i][1] * out[i][1];
			bins[i] = 10.0 / log(10.0) * log(bins[i] + 1e-6);

			// normalize values in range 0dB to 96dB to be values
			// between 0 and 1
			bins[i] = fmax(0, bins[i]);
			bins[i] = fmin(1, bins[i] / 96.0);
		}

		peak = find_peak_frequency(bins, num_bins);
		peaks.push_back(peak);
	}

	printf("%f\n", find_median_frequency(peaks));

	fftw_destroy_plan(plan);
	free(hamming_window);
	fftw_free(in); fftw_free(out);
}

int main(int argc, char *argv[1])
{
	size_t samples_per_period = 22050;

	snd_pcm_t *pcm_handle = get_pcm_capture_handle();
	short *chunk = new short[samples_per_period * snd_pcm_format_width(format) / 8];

	for ( ;; ) {
		get_samples(pcm_handle, chunk, samples_per_period);
		fft(chunk, samples_per_period);
	}

	snd_pcm_close(pcm_handle);
	free(chunk);
}
