#pragma once

#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "../defines/defines.h"

#include "../debug_utilities/debug_utilities.h"

#define DRAM_SIZE 512 * 1024 

class spu_t {
public:

	spu_t();

	void read32(const uint32_t& addr, uint32_t& data);
	void write32(const uint32_t& addr, const uint32_t& data);

	void read16(const uint32_t& addr, uint16_t& data);
	void write16(const uint32_t& addr, const uint16_t& data);


	float clock();


	uint8_t* dram = new uint8_t[DRAM_SIZE];

	void readDram16(const uint32_t& addr, uint16_t& data);
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
				uint32_t data;
				struct {
					uint16_t l, h;
				}hw;
			}adsr;

			union voice_ADSR_CurrentVolume_t {
				uint16_t data;
			}adsrCurrentVolume;

			union voice_ADPCM_RepeatAddress_t {
				uint16_t data;
			}adpcmRepeatAddress;

			union voice_CurrentVolumeLeftRight_t {
				uint32_t data;
			}currentVolumeLeftRight;

			uint32_t currentVoiceAddress;

			int16_t oldest = 0;
			int16_t older = 0;
			int16_t old = 0;
			int16_t newest = 0;

			void pushSample(int16_t sample) {
				oldest = older;
				older = old;
				old = newest;
				newest = sample;
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

			bool startSampleBlockProcesing = false;
			bool release = true;
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
		}keyOn;

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
		}keyOff;

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
			uint16_t muteSPU : 1;
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



	int i = 0;
	//debug
	debugUtilities* pDebugger = nullptr;
};