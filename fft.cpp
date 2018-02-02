#include <algorithm>
#include <cmath>
#include <utility>
#include <fftw3.h>

#include "fft.h"

double * hamming(int fft_size)
{
	double *window = new double[fft_size];

	for (int i = 0; i < fft_size; ++i) {
		window[i] = alpha - beta * cos(2.0 * M_PI * double(i) / double(fft_size - 1));
	}

	return window;
}

double interpolate_peak_bin(double left, double mid, double right)
{
	return (left - right) / (2.0 * (right + left - 2.0 * mid));
}

double bin_to_frequency(double bin, double sample_rate, double frame_size)
{
	return double(bin) * sample_rate / double(frame_size);
}

double find_peak_frequency(double *bins, int frame_size)
{
	int peak_bin = 0;
	double peak = bins[peak_bin];

	// amount of bins is frame_size / 2, once again
	for (int i = 0; i < frame_size / 2; ++i) {
		if (bins[i] > peak) {
			peak = bins[i];
			peak_bin = i;
		}
	}

	double interpolated_peak_bin = interpolate_peak_bin(bins[peak_bin - 1],
							    bins[peak_bin],
							    bins[peak_bin + 1]);
	return bin_to_frequency(peak_bin + interpolated_peak_bin, sample_rate, frame_size);
}

double find_median_frequency(std::vector<double> &peaks)
{
	size_t n = peaks.size() / 2;
	std::nth_element(peaks.begin(), peaks.begin() + n, peaks.end());
	return peaks[n];
}

double fft_median_peak_frequency(short *samples, size_t nsamples)
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
			// bins[i] = fmin(1, bins[i] / 96.0);
		}

		peak = find_peak_frequency(bins, fft_size);
		peaks.push_back(peak);
	}

	free(hamming_window);
	fftw_free(in); fftw_free(out);
	fftw_destroy_plan(plan);

	return find_median_frequency(peaks);
}
