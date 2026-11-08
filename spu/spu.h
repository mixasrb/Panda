#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "../debugger/debugger.h"

constexpr auto LS_SIZE = 512 * 1024;
constexpr auto XA_ADPCM_SECTOR_SIZE = 0x930;

class BusInterface;

struct Output {
	float channel0;
	float channel1;

	Output operator+(Output a) {
		Output output;
		output.channel0 = channel0 + a.channel0;
		output.channel1 = channel1 + a.channel1;
		return output;
	}
};

class Spu {
public:
	BusInterface* busPtr;

	Spu();

	void Read32(const uint32_t& addr, uint32_t& data);
	void Write32(const uint32_t& addr, const uint32_t& data);

	void Read16(const uint32_t& addr, uint16_t& data);
	void Write16(const uint32_t& addr, const uint16_t& data);

	void Read8(const uint32_t& addr, uint8_t& data);

	Output Clock();

	std::unique_ptr<uint8_t[]> ls = std::make_unique<uint8_t[]>(LS_SIZE);
	int16_t reverb[32];

	uint16_t LsRead16(const uint32_t& addr);
	void LsWrite16(const uint32_t& addr, const uint16_t& data);

	uint8_t LsRead8(const uint32_t& addr);

	struct Voices {

		inline static Spu* spuPtr;

		struct Register {

			mutable uint8_t INDEX;

			union Volume {
				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;
			}volume;

			uint16_t adpcmSampleRate;

			uint16_t adpcmStartAddress;

			union Adsr {
				struct {
					uint8_t l, h1, h2, h3;
				}b;

				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;

				struct {
					uint32_t sustainLevel : 4;
					uint32_t decayShift : 4;
					uint32_t attackStep : 2;
					uint32_t attackShift : 5;
					uint32_t attackMode : 1;
					uint32_t releaseShift : 5;
					uint32_t releaseMode : 1;
					uint32_t sustainStep : 2;
					uint32_t sustainShift : 5;
					uint32_t notUsed : 1;
					uint32_t sustainDirection : 1;
					uint32_t sustainMode : 1;
				}elem;
			}adsr;

			uint16_t adsrCurrentVolume;

			uint16_t adpcmRepeatAddress;

			union CurrentVolume {
				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;
			}currentVolume;

			uint32_t currentAddress;

			int16_t older = 0;
			int16_t old = 0;

			uint8_t blockBytes[16];

			uint8_t shiftFilter;

			uint8_t flagBits;

			uint8_t compressedSamples[28];

			uint8_t compressedSample;

			int16_t samples[28];

			int16_t sample;

			uint16_t step;
			uint32_t counter;

			int16_t GetNextSample();
			void DecodeCompressedSamples();

			uint32_t adsrCycles = 0;
			uint32_t adsrCyclesNumber = 0;

			int32_t adsrStep = 0;
			int32_t adsrLevel = 0;

			bool isEnvelopeAttack = false;
			bool isEnvelopeDecay = false;
			bool isEnvelopeSustain = false;
			bool isEnvelopeRelease = false;

			void StartEnvelopeAttack();
			void StartEnvelopeDecay();
			void StartEnvelopeSustain();
			void StartEnvelopeRelease();

			Output Envelope(const int16_t& input);

		}reg;

		inline static union KeyOn {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}keyOn, tempKeyOn;

		inline static union KeyOff {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}keyOff, tempKeyOff;


		inline static union ChannelFmMode {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelFmMode;

		inline static union ChannelNoiseMode {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelNoiseMode;

		inline static union ChannelReverbMode {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelReverbMode;

		inline static union ChannelOnOff {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelOnOff;
	};

	uint32_t reverbCurrentBufferAddress;
	bool bReverbCycle = false;
	void ReverbClock(const Output& input);
	Output reverbOutput = { 0, 0 };

	Voices voices[24];

	union MainVolume {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}mainVolume;

	union ReverbVolume {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}reverbVolume;

	uint16_t lsReverbAddress;

	uint16_t lsIrqAddress;

	uint16_t lsTransferAddress;

	uint32_t currentAddress;

	uint16_t lsTransferFifo;

	union SpuControl {
		uint16_t data;
		struct {
			uint16_t cdAudioEnable : 1;
			uint16_t externalAudioEnable : 1;
			uint16_t cdAudioReverb : 1;
			uint16_t externalAudioReverb : 1;
			uint16_t soundRAMTransferMode : 2;
			uint16_t irq9Enable : 1;
			uint16_t reverbMasterEnable : 1;
			uint16_t noiseFrequencyStep : 2;
			uint16_t noiseFrequencyShift : 4;
			uint16_t unmuteSPU : 1;
			uint16_t spuEnable : 1;
		}elem;
	}spuControl;

	union LsTransferControl {
		uint16_t data;
	}lsTransferControl;

	union SpuStatus {
		uint16_t data;
		struct {
			uint16_t currentSPUMode : 6;
			uint16_t irq9Flag : 1;
			uint16_t dataTransferDMAReadWriteRequest : 1;
			uint16_t dataTransferDMAWriteRequest : 1;
			uint16_t dataTransferDMAReadRequest : 1;
			uint16_t dataTransferBusyFlag : 1;
			uint16_t writingFirstSecondHalfCaptureBuffers : 1;
			uint16_t unused : 4;
		}elem;
	}spuStatus;

	union CdVolume {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}cdVolume;

	union ExternVolume {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}externVolume;

	union CurrentMainVolume {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}currentMainVolume;

	std::vector<int32_t> pos_adpcm_table;
	std::vector<int32_t> neg_adpcm_table;

	std::vector<int16_t> gauss;
	std::vector<int16_t> table;

	uint32_t irqAddress = 0;
	void IrqSend();

	Output ClockXaAdpcm();

	Output outputSamplesBufferXA[7];
	Output OldOutputSampleXA;

	uint8_t asectXA;
	uint8_t assXA;
	uint8_t ammXA;

	uint8_t* pBufferXA_ADPCM = new uint8_t[XA_ADPCM_SECTOR_SIZE];

	int16_t samplesMonoXA[56 * 0x12 * 8];
	int16_t samplesLeftXA[28 * 0x12 * 8];
	int16_t samplesRightXA[28 * 0x12 * 8];

	int16_t oldLeft = 0;
	int16_t olderLeft = 0;
	int16_t oldRight = 0;
	int16_t olderRight = 0;
	int16_t oldMono = 0;
	int16_t olderMono = 0;

	uint32_t currentSample = 0;
	uint32_t counterMono = 0;
	uint32_t counterLeft = 0;
	uint32_t counterRight = 0;

	int16_t ringBuffer[32];

	void DecodeSectorXaAdpcm(uint8_t*& src);
	void Decode28NibblesXaAdpcm(uint8_t* src, const uint8_t& block, const uint8_t& nibble,
		int16_t* dst, int16_t& old, int16_t& older, uint32_t& counter);
	void Decode28BytesXaAdpcm(uint8_t* src, const uint8_t& block, const uint8_t& byte,
		int16_t* dst, int16_t& old, int16_t& older, uint32_t& counter);

	union submode_t {
		uint8_t data;
		struct {
			uint8_t endOfRecord : 1;
			uint8_t video : 1;
			uint8_t audio : 1;
			uint8_t data : 1;
			uint8_t trigger : 1;
			uint8_t form2 : 1;
			uint8_t realTime : 1;
			uint8_t endOfFile : 1;
		}elem;
	}submode;

	union codinginfo_t {
		uint8_t data;
		struct {
			uint8_t monoStereo : 2;
			uint8_t sampleRate : 2;
			uint8_t bitsPerSample : 2;
			uint8_t emphasis : 1;
			uint8_t reserved : 1;
		}elem;
	}codinginfo;

	bool bStereo = false;
	bool bSampleRate18900Hz = false;
	bool b8BitsPerSample = false;
	bool bCycle = false;

	const float step44kHz = 1.0f / 44100.0f;
	const float step37kHz = 1.0f / 37800.0f;
	float counter37kHz = 0.0f;
	Output outputCDXA;
	Output outputCD_DriveAudio;

	uint16_t cdAudioBufferCounter = 0;
	uint16_t voice1MonoBufferCounter = 0;
	uint16_t voice3MonoBufferCounter = 0;

	uint8_t asectCdda;
	uint8_t assCdda;
	uint8_t ammCdda;

	uint32_t counterCdda = 0x930;

	Output ClockCdda();

	uint8_t* pBufferCdda = nullptr;

	//debug

	inline static const char* TAG = TAG_SPU;
};