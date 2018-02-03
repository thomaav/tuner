#pragma once

class PCMDevice {
public:
	static constexpr snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

	PCMDevice();
	~PCMDevice();
	void get_samples(short *chunk, size_t nsamples);
private:
	static unsigned int sample_rate;
	static constexpr char pcm_name[] = "default";
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *hw_params;
};
