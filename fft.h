#pragma once

#include <vector>

// hamming window
constexpr double alpha = 0.54;
constexpr double beta  = 0.46;

constexpr int sample_rate = 44100;
constexpr int fft_size = 4096;
constexpr int num_bins = fft_size / 2;
constexpr int step_size = fft_size / 2; // 50% window overlap

double * hamming(int fft_size);
double interpolate_peak_bin(double left, double mid, double right);
double bin_to_frequency(double bin, double sample_rate, double frame_size);
double find_peak_frequency(double *bins, int frame_size);
double find_median_frequency(std::vector<double> &peaks);
double fft_median_peak_frequency(short *samples, size_t nsamples);
