#pragma once

#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "../defines/defines.h"

#include "../debug_utilities/debug_utilities.h"

#define DRAM_SIZE 512 * 1024 
#define XA_ADPCM_SECTOR_SIZE 0x930

class busInterface_t;

struct output_t {
	float channel0;
	float channel1;

	output_t operator+(output_t a) {
		output_t output;
		output.channel0 = channel0 + a.channel0;
		output.channel1 = channel1 + a.channel1;
		return output;
	}
};

class spu_t {
public:

	spu_t();

	void read32(const uint32_t& addr, uint32_t& data);
	void write32(const uint32_t& addr, const uint32_t& data);

	void read16(const uint32_t& addr, uint16_t& data);
	void write16(const uint32_t& addr, const uint16_t& data);

	void read8(const uint32_t& addr, uint8_t& data);
	void write8(const uint32_t& addr, const uint8_t& data);

	output_t clock();

	busInterface_t* pBus;

	uint8_t* dram = new uint8_t[DRAM_SIZE];
	int16_t reverb[32];

	uint32_t readDram32(const uint32_t& addr);
	void writeDram32(const uint32_t& addr, const uint32_t& data);

	uint16_t readDram16(const uint32_t& addr);
	void writeDram16(const uint32_t& addr, const uint16_t& data);

	void readDram8(const uint32_t& addr, uint8_t& data);
	void writeDram8(const uint32_t& addr, const uint8_t& data);

	class voices_t {
	public:
		spu_t* p_spu;

		struct voiceRegister_t {

			union voiceVolumeLeftRight_t {
				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;
			}volumeLeftRight;

			union voice_ADPCM_SampleRate_t {
				uint16_t data;
			}adpcmSampleRate;

			union voice_ADPCM_StartAddress_t {
				uint16_t data;
			}adpcmStartAddress;

			union voice_ADSR_t {
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

			union voice_ADSR_CurrentVolume_t {
				uint16_t data;
			}adsrCurrentVolume;

			union voice_ADPCM_RepeatAddress_t {
				uint16_t data;
			}adpcmRepeatAddress;

			union voice_CurrentVolumeLeftRight_t {
				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;
			}currentVolumeLeftRight;

			uint32_t currentVoiceAddress;

			int16_t older = 0;
			int16_t old = 0;

			int16_t voiceOldest = 0;
			int16_t voiceOlder = 0;
			int16_t voiceOld = 0;
			int16_t voiceNewest = 0;

			void pushSample(int16_t sample) {
				voiceOldest = voiceOlder;
				voiceOlder = voiceOld;
				voiceOld = voiceNewest;
				voiceNewest = sample;
			}

			uint8_t blockBytes[16];

			union {
				uint8_t data;
			}shiftFilter;

			union {
				uint8_t data;
				struct {
					uint8_t loopEnd : 1;
					uint8_t loopRepeat : 1;
					uint8_t loopStart : 1;
					uint8_t : 5;
				}elem;
			}flagBits;

			uint8_t code;

			uint8_t compressedSamples[28];

			uint8_t compressedSample;

			int16_t samples[28];

			int16_t sample;

			uint16_t step;
			uint32_t counter;

			int16_t out;

			uint32_t adsrCycles = 0;
			uint32_t adsrCyclesNumber = 0;

			int32_t adsrStep = 0;
			int32_t adsrLevel = 0;

			bool bEnvelopeAttack = false;
			bool bEnvelopeDecay = false;
			bool bEnvelopeSustain = false;
			bool bEnvelopeRelease = false;

			void startEnvelopeAttack();
			void startEnvelopeDecay();
			void startEnvelopeSustain();
			void startEnvelopeRelease();

			output_t envelope(const int16_t& input);
		};

		voiceRegister_t voiceRegisters[24];

		union voiceKeyOn_t {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
			struct {
				uint32_t voice0On : 1;
				uint32_t voice1On : 1;
				uint32_t voice2On : 1;
				uint32_t voice3On : 1;
				uint32_t voice4On : 1;
				uint32_t voice5On : 1;
				uint32_t voice6On : 1;
				uint32_t voice7On : 1;
				uint32_t voice8On : 1;
				uint32_t voice9On : 1;
				uint32_t voice10On : 1;
				uint32_t voice11On : 1;
				uint32_t voice12On : 1;
				uint32_t voice13On : 1;
				uint32_t voice14On : 1;
				uint32_t voice15On : 1;
				uint32_t voice16On : 1;
				uint32_t voice17On : 1;
				uint32_t voice18On : 1;
				uint32_t voice19On : 1;
				uint32_t voice20On : 1;
				uint32_t voice21On : 1;
				uint32_t voice22On : 1;
				uint32_t voice23On : 1;
				uint32_t : 8;
			}elem;
		};

		voiceKeyOn_t keyOn, tempKeyOn;

		union voiceKeyOff_t {
			uint32_t data;
			struct {
				uint32_t voice0Off : 1;
				uint32_t voice1Off : 1;
				uint32_t voice2Off : 1;
				uint32_t voice3Off : 1;
				uint32_t voice4Off : 1;
				uint32_t voice5Off : 1;
				uint32_t voice6Off : 1;
				uint32_t voice7Off : 1;
				uint32_t voice8Off : 1;
				uint32_t voice9Off : 1;
				uint32_t voice10Off : 1;
				uint32_t voice11Off : 1;
				uint32_t voice12Off : 1;
				uint32_t voice13Off : 1;
				uint32_t voice14Off : 1;
				uint32_t voice15Off : 1;
				uint32_t voice16Off : 1;
				uint32_t voice17Off : 1;
				uint32_t voice18Off : 1;
				uint32_t voice19Off : 1;
				uint32_t voice20Off : 1;
				uint32_t voice21Off : 1;
				uint32_t voice22Off : 1;
				uint32_t voice23Off : 1;
				uint32_t : 8;
			}elem;
			struct {
				uint16_t l, h;
			}hw;
		};

		voiceKeyOff_t keyOff, tempKeyOff;

		union voiceChannel_FM_Mode_t {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channel_FM_Mode;

		union voiceChannelNoiseMode_t {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelNoiseMode;

		union voiceChannelReverbMode_t {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
		}channelReverbMode;

		union voiceChannelOnOff_t {
			uint32_t data;
			struct {
				uint16_t l, h;
			}hw;
			struct {
				uint32_t voice0Status : 1;
				uint32_t voice1Status : 1;
				uint32_t voice2Status : 1;
				uint32_t voice3Status : 1;
				uint32_t voice4Status : 1;
				uint32_t voice5Status : 1;
				uint32_t voice6Status : 1;
				uint32_t voice7Status : 1;
				uint32_t voice8Status : 1;
				uint32_t voice9Status : 1;
				uint32_t voice10Status : 1;
				uint32_t voice11Status : 1;
				uint32_t voice12Status : 1;
				uint32_t voice13Status : 1;
				uint32_t voice14Status : 1;
				uint32_t voice15Status : 1;
				uint32_t voice16Status : 1;
				uint32_t voice17Status : 1;
				uint32_t voice18Status : 1;
				uint32_t voice19Status : 1;
				uint32_t voice20Status : 1;
				uint32_t voice21Status : 1;
				uint32_t voice22Status : 1;
				uint32_t voice23Status : 1;
				uint32_t : 8;
			}elem;
		}channelOnOff;

		void decodeCompressedSample(voiceRegister_t& currentVoiceRegisters);
		int16_t output(const uint8_t& voice);
	};

	uint32_t reverbCurrentBufferAddress;
	bool bReverbCycle = false;
	void reverbClock(const output_t& input);
	output_t reverbOutput = { 0, 0 };

	voices_t voices;

	union mainVolumeLeftRight_t {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}mainVolumeLeftRight;

	union reverbOutputVolumeLeftRight_t {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}reverbOutputVolumeLeftRight;

	union dramReverbWorkAreaStartAddress_t {
		uint16_t data;
	}dramReverbWorkAreaStartAddress;

	union dram_IRQ_Address_t {
		uint16_t data;
	}dram_IRQ_Address;

	union dramDataTransferAddress_t {
		uint16_t data;
	}dramDataTransferAddress;

	uint32_t currentAddress;

	union dramDataTransferFifo_t {
		uint16_t data;
	}dramDataTransferFifo;

	union spuControlRegister_t {
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
	}spuControlRegister;

	union dramDataTransferControl_t {
		uint16_t data;
	}dramDataTransferControl;

	union spuStatusRegister_t {
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
	}spuStatusRegister;

	union cdVolumeLeftRight_t {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}cdVolumeLeftRight;

	union externVolumeLeftRight_t {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}externVolumeLeftRight;

	union currentMainVolumeLeftRight_t {
		uint32_t data;
		struct {
			uint16_t l, h;
		}hw;
	}currentMainVolumeLeftRight;

	std::vector<int32_t> pos_adpcm_table;
	std::vector<int32_t> neg_adpcm_table;

	std::vector<int16_t> gauss;
	std::vector<int16_t> table;

	uint32_t irqAddress = 0;
	void irqSend();

	output_t clockXA_ADPCM();

	output_t outputSamplesBufferXA[7];
	output_t OldOutputSampleXA;

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

	void decodeSectorXA_ADPCM(uint8_t*& src);
	void decode28NibblesXA_ADPCM(uint8_t* src, const uint8_t& block, const uint8_t& nibble,
		int16_t* dst, int16_t& old, int16_t& older, uint32_t& counter);
	void decode28BytesXA_ADPCM(uint8_t* src, const uint8_t& block, const uint8_t& byte,
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
	output_t outputCDXA;
	output_t outputCD_DriveAudio;

	uint16_t cdAudioBufferCounter = 0;
	uint16_t voice1MonoBufferCounter = 0;
	uint16_t voice3MonoBufferCounter = 0;


	uint8_t asectCDDA;
	uint8_t assCDDA;
	uint8_t ammCDDA;

	uint32_t counterCDDA = 0x930;

	output_t clockCDDA();

	uint8_t* pBufferCDDA = nullptr;

	//debug
	debugUtilities* pDebugger = nullptr;
};