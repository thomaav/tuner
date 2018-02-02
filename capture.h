#pragma once

constexpr char pcm_name[] = "default";
constexpr snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // signed 16-bit little endian

snd_pcm_t *get_pcm_capture_handle();
void get_samples(snd_pcm_t *handle, short *buf, std::size_t nsamples);

class PCMDevice {
public:
	PCMDevice();
	~PCMDevice();
	void get_samples(short *chunk, size_t nsamples);
private:
	unsigned int sample_rate;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *hw_params;
};
