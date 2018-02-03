#include <algorithm>
#include <cmath>
#include <utility>
#include <fftw3.h>
#include <samplerate.h>

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
	if (2.0 * (right + left - 2.0 * mid) == 0.0)
		return 0.0;

	return (left - right) / (2.0 * (right + left - 2.0 * mid));
}

double bin_to_frequency(double bin, double sample_rate, double frame_size)
{
	return double(bin) * sample_rate / double(frame_size);
}

double find_peak_frequency(double *bins, int frame_size)
{
	// ignore the first 7 bins, i.e. ones that are less than ~75Hz
	// with sample rate 44100 and fft size of 4096, since low E is
	// 82 Hz
	int peak_bin = 7;
	double peak = bins[peak_bin];

	// amount of bins is frame_size / 2, once again
	for (int i = peak_bin; i < frame_size / 2; ++i) {
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
	int err;

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

	// downsampling for HPS
	SRC_STATE *src_state = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
	if (err < 0) {
		printf("Unable to initialize sample rate converter object (%s)\n", src_strerror(err));
	}

	float base_bins[num_bins];
	float downsampled_bins[num_bins];
	float hps_bins_acc[num_bins];

	SRC_DATA src_data;
	src_data.end_of_input = 1;
	src_data.input_frames = num_bins;

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
		for (int i = 0; i < num_bins; ++i)
			bins[i] = 10.0 * log10(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
			// bins[i] = fmax(0, bins[i]);

		// libresample expects floats, use an accumulator to
		// iterate easily
		std::copy(bins, bins + num_bins, base_bins);
		std::copy(base_bins, base_bins + num_bins, hps_bins_acc);

		// decimate bins of dB values
		for (int i = 2; i < 6; ++i) {
			err = src_reset(src_state);

			src_data.src_ratio = 1.0 / i;
			src_data.data_in = base_bins;
			src_data.data_out = downsampled_bins;
			src_data.output_frames = num_bins / i;

			if ((err = src_process(src_state, &src_data))) {
				printf("Unable to downsample sampling data (%s)\n",
				       src_strerror(err));
				exit(1);
			}

			for (int j = 0; j < num_bins / i; ++j) {
				hps_bins_acc[j] *= downsampled_bins[j];
			}
		}

		// accumulator now holds the results of our HPS, which
		// we can pass as doubles to our peak finder
		std::copy(hps_bins_acc, hps_bins_acc + num_bins, bins);

		peak = find_peak_frequency(bins, fft_size);
		peaks.push_back(peak);
	}

	free(hamming_window);
	fftw_free(in); fftw_free(out);
	fftw_destroy_plan(plan);
	src_delete(src_state);

	return find_median_frequency(peaks);
}
