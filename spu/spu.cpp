#include "spu.h"

extern bool g_emulationPaused;
extern uint32_t g_systemClocks;
extern uint32_t g_systemFrames;
extern uint32_t g_systemSeconds;
extern uint32_t g_systemMinutes;

spu_t::spu_t() {
	voices.p_spu = this;

	pos_adpcm_table.resize(5);
	pos_adpcm_table = { 0,+60,+115,+98,+122 };

	neg_adpcm_table.resize(5);
	neg_adpcm_table = { 0,0,-52,-55,-60 };

	gauss.resize(0x200);
	gauss = {
	  -0x001,-0x001,-0x001,-0x001,-0x001,-0x001,-0x001,-0x001,
	  -0x001,-0x001,-0x001,-0x001,-0x001,-0x001,-0x001,-0x001,
	  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,
	  0x0001,0x0001,0x0001,0x0002,0x0002,0x0002,0x0003,0x0003,
	  0x0003,0x0004,0x0004,0x0005,0x0005,0x0006,0x0007,0x0007,
	  0x0008,0x0009,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,
	  0x000F,0x0010,0x0011,0x0012,0x0013,0x0015,0x0016,0x0018,
	  0x0019,0x001B,0x001C,0x001E,0x0020,0x0021,0x0023,0x0025,
	  0x0027,0x0029,0x002C,0x002E,0x0030,0x0033,0x0035,0x0038,
	  0x003A,0x003D,0x0040,0x0043,0x0046,0x0049,0x004D,0x0050,
	  0x0054,0x0057,0x005B,0x005F,0x0063,0x0067,0x006B,0x006F,
	  0x0074,0x0078,0x007D,0x0082,0x0087,0x008C,0x0091,0x0096,
	  0x009C,0x00A1,0x00A7,0x00AD,0x00B3,0x00BA,0x00C0,0x00C7,
	  0x00CD,0x00D4,0x00DB,0x00E3,0x00EA,0x00F2,0x00FA,0x0101,
	  0x010A,0x0112,0x011B,0x0123,0x012C,0x0135,0x013F,0x0148,
	  0x0152,0x015C,0x0166,0x0171,0x017B,0x0186,0x0191,0x019C,
	  0x01A8,0x01B4,0x01C0,0x01CC,0x01D9,0x01E5,0x01F2,0x0200,
	  0x020D,0x021B,0x0229,0x0237,0x0246,0x0255,0x0264,0x0273,
	  0x0283,0x0293,0x02A3,0x02B4,0x02C4,0x02D6,0x02E7,0x02F9,
	  0x030B,0x031D,0x0330,0x0343,0x0356,0x036A,0x037E,0x0392,
	  0x03A7,0x03BC,0x03D1,0x03E7,0x03FC,0x0413,0x042A,0x0441,
	  0x0458,0x0470,0x0488,0x04A0,0x04B9,0x04D2,0x04EC,0x0506,
	  0x0520,0x053B,0x0556,0x0572,0x058E,0x05AA,0x05C7,0x05E4,
	  0x0601,0x061F,0x063E,0x065C,0x067C,0x069B,0x06BB,0x06DC,
	  0x06FD,0x071E,0x0740,0x0762,0x0784,0x07A7,0x07CB,0x07EF,
	  0x0813,0x0838,0x085D,0x0883,0x08A9,0x08D0,0x08F7,0x091E,
	  0x0946,0x096F,0x0998,0x09C1,0x09EB,0x0A16,0x0A40,0x0A6C,
	  0x0A98,0x0AC4,0x0AF1,0x0B1E,0x0B4C,0x0B7A,0x0BA9,0x0BD8,
	  0x0C07,0x0C38,0x0C68,0x0C99,0x0CCB,0x0CFD,0x0D30,0x0D63,
	  0x0D97,0x0DCB,0x0E00,0x0E35,0x0E6B,0x0EA1,0x0ED7,0x0F0F,
	  0x0F46,0x0F7F,0x0FB7,0x0FF1,0x102A,0x1065,0x109F,0x10DB,
	  0x1116,0x1153,0x118F,0x11CD,0x120B,0x1249,0x1288,0x12C7,
	  0x1307,0x1347,0x1388,0x13C9,0x140B,0x144D,0x1490,0x14D4,
	  0x1517,0x155C,0x15A0,0x15E6,0x162C,0x1672,0x16B9,0x1700,
	  0x1747,0x1790,0x17D8,0x1821,0x186B,0x18B5,0x1900,0x194B,
	  0x1996,0x19E2,0x1A2E,0x1A7B,0x1AC8,0x1B16,0x1B64,0x1BB3,
	  0x1C02,0x1C51,0x1CA1,0x1CF1,0x1D42,0x1D93,0x1DE5,0x1E37,
	  0x1E89,0x1EDC,0x1F2F,0x1F82,0x1FD6,0x202A,0x207F,0x20D4,
	  0x2129,0x217F,0x21D5,0x222C,0x2282,0x22DA,0x2331,0x2389,
	  0x23E1,0x2439,0x2492,0x24EB,0x2545,0x259E,0x25F8,0x2653,
	  0x26AD,0x2708,0x2763,0x27BE,0x281A,0x2876,0x28D2,0x292E,
	  0x298B,0x29E7,0x2A44,0x2AA1,0x2AFF,0x2B5C,0x2BBA,0x2C18,
	  0x2C76,0x2CD4,0x2D33,0x2D91,0x2DF0,0x2E4F,0x2EAE,0x2F0D,
	  0x2F6C,0x2FCC,0x302B,0x308B,0x30EA,0x314A,0x31AA,0x3209,
	  0x3269,0x32C9,0x3329,0x3389,0x33E9,0x3449,0x34A9,0x3509,
	  0x3569,0x35C9,0x3629,0x3689,0x36E8,0x3748,0x37A8,0x3807,
	  0x3867,0x38C6,0x3926,0x3985,0x39E4,0x3A43,0x3AA2,0x3B00,
	  0x3B5F,0x3BBD,0x3C1B,0x3C79,0x3CD7,0x3D35,0x3D92,0x3DEF,
	  0x3E4C,0x3EA9,0x3F05,0x3F62,0x3FBD,0x4019,0x4074,0x40D0,
	  0x412A,0x4185,0x41DF,0x4239,0x4292,0x42EB,0x4344,0x439C,
	  0x43F4,0x444C,0x44A3,0x44FA,0x4550,0x45A6,0x45FC,0x4651,
	  0x46A6,0x46FA,0x474E,0x47A1,0x47F4,0x4846,0x4898,0x48E9,
	  0x493A,0x498A,0x49D9,0x4A29,0x4A77,0x4AC5,0x4B13,0x4B5F,
	  0x4BAC,0x4BF7,0x4C42,0x4C8D,0x4CD7,0x4D20,0x4D68,0x4DB0,
	  0x4DF7,0x4E3E,0x4E84,0x4EC9,0x4F0E,0x4F52,0x4F95,0x4FD7,
	  0x5019,0x505A,0x509A,0x50DA,0x5118,0x5156,0x5194,0x51D0,
	  0x520C,0x5247,0x5281,0x52BA,0x52F3,0x532A,0x5361,0x5397,
	  0x53CC,0x5401,0x5434,0x5467,0x5499,0x54CA,0x54FA,0x5529,
	  0x5558,0x5585,0x55B2,0x55DE,0x5609,0x5632,0x565B,0x5684,
	  0x56AB,0x56D1,0x56F6,0x571B,0x573E,0x5761,0x5782,0x57A3,
	  0x57C3,0x57E2,0x57FF,0x581C,0x5838,0x5853,0x586D,0x5886,
	  0x589E,0x58B5,0x58CB,0x58E0,0x58F4,0x5907,0x5919,0x592A,
	  0x593A,0x5949,0x5958,0x5965,0x5971,0x597C,0x5986,0x598F,
	  0x5997,0x599E,0x59A4,0x59A9,0x59AD,0x59B0,0x59B2,0x59B3
	};

	voices.channelOnOff.data = 0x00ffffff;
}

void spu_t::read32(const uint32_t& addr, uint32_t& data) {
	data = 0;
	std::cout << "[SPU] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
	g_emulationPaused = true;
}

void spu_t::write32(const uint32_t& addr, const uint32_t& data) {
	if ((addr >= 0x1f801c00) && (addr <= 0x1f801d7e)) {
		uint8_t voiceIndex = (addr >> 4) - 0x1f801c0;
		switch (addr & 0xf) {
		case 0x0:
			voices.voiceRegisters[voiceIndex].volumeLeftRight.data = data;
			break;

		case 0x8:
			/*std::cout << "[SPU] adsr 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.voiceRegisters[voiceIndex].adsr.data = data;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801d80) && (addr <= 0x1f801dbc)) {
		switch (addr) {
		case 0x1f801d80:
			mainVolumeLeftRight.data = data;
			break;

		case 0x1f801d84:
			reverbOutputVolumeLeftRight.data = data;
			break;

		case 0x1f801d88:
			/*	std::cout << "[SPU] Key On 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
					<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOn.data |= data;
			voices.tempKeyOn.data = data;
			for (int voice = 0; voice < 24; voice++) {
				if ((data >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;
					voices.voiceRegisters[voice].startSampleBlockProcesing = true;
					voices.voiceRegisters[voice].attack = true;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = false;
					voices.voiceRegisters[voice].adsrLevel = 0;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//std::cout << "voice " << std::dec << (uint16_t)voice << "   " << std::hex << voices.voiceRegisters[voice].currentVoiceAddress << std::endl;
				}
			}
			voices.channelOnOff.data &= ~voices.keyOn.data;
			break;

		case 0x1f801d8c:
			/*std::cout << "[SPU] Key Off 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOff.data |= data;
			voices.tempKeyOff.data = data;
			for (uint32_t voice = 0; voice < 24; voice++) {
				if (((uint32_t)data >> voice) & 0x1) {
					//if (voices.voiceRegisters[voice].sustain) {
					voices.voiceRegisters[voice].attack = false;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = true;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//}
				}
			}
			break;

		case 0x1f801d90:
			voices.channel_FM_Mode.data = data;
			break;

		case 0x1f801d94:
			voices.channelNoiseMode.data = data;
			break;

		case 0x1f801d98:
			voices.channelReverbMode.data = data;
			break;

		case 0x1f801db0:
			cdVolumeLeftRight.data = data;
			break;

		case 0x1f801db4:
			externVolumeLeftRight.data = data;
			break;

		case 0x1f801db8:
			currentMainVolumeLeftRight.data = data;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801dc0) && (addr <= 0x1f801dfc)) {
		reverb[addr - 0x1f801dc0] = (uint16_t)data;
		reverb[addr + 1 - 0x1f801dc0] = (uint16_t)(data >> 16);
	}
	else if ((addr >= 0x1f801e00) && (addr < 0x1f801e60)) {
		std::cout << "[SPU] EMULATION PASUED! unhandled write32 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
}

void spu_t::read16(const uint32_t& addr, uint16_t& data) {
	data = 0;

	if ((addr >= 0x1f801c00) && (addr <= 0x1f801d7e)) {
		uint8_t voiceIndex = (addr >> 4) - 0x1f801c0;
		switch (addr & 0xf) {
		case 0x4:
			data = voices.voiceRegisters[voiceIndex].adpcmSampleRate.data;
			break;

		case 0x8:
			data = voices.voiceRegisters[voiceIndex].adsr.hw.l;
			break;

		case 0xa:
			data = voices.voiceRegisters[voiceIndex].adsr.hw.h;
			break;

		case 0xc:
			data = voices.voiceRegisters[voiceIndex].adsrCurrentVolume.data;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled addr read16 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801d80) && (addr <= 0x1f801dbc)) {
		switch (addr) {
		case 0x1f801d80:
			data = mainVolumeLeftRight.hw.l;
			break;

		case 0x1f801d82:
			data = mainVolumeLeftRight.hw.h;
			break;

		case 0x1f801d88:
			data = voices.tempKeyOn.hw.l;
			break;

		case 0x1f801d8a:
			data = voices.tempKeyOn.hw.h;
			break;

		case 0x1f801d8c:
			data = voices.tempKeyOff.hw.l;
			break;

		case 0x1f801d8e:
			data = voices.tempKeyOff.hw.h;
			break;

		case 0x1f801d90:
			data = voices.channel_FM_Mode.hw.l;
			break;

		case 0x1f801d92:
			data = voices.channel_FM_Mode.hw.h;
			break;

		case 0x1f801d94:
			data = voices.channelNoiseMode.hw.l;
			break;

		case 0x1f801d96:
			data = voices.channelNoiseMode.hw.h;
			break;

		case 0x1f801d98:
			data = voices.channelReverbMode.hw.l;
			break;

		case 0x1f801d9a:
			data = voices.channelReverbMode.hw.h;
			break;

		case 0x1f801da6:
			data = dramDataTransferAddress.data;
			break;

		case 0x1f801da8:
			readDram16(currentAddress, data);
			currentAddress += 2;
			break;

		case 0x1f801daa:
			data = spuControlRegister.data;
			break;

		case 0x1f801dae:
			data = spuStatusRegister.data;
			break;

		case 0x1f801dac:
			data = dramDataTransferControl.data;
			break;

		case 0x1f801db0:
			data = cdVolumeLeftRight.hw.l;
			break;

		case 0x1f801db2:
			data = cdVolumeLeftRight.hw.h;
			break;

		case 0x1f801db4:
			data = externVolumeLeftRight.hw.l;
			break;

		case 0x1f801db6:
			data = externVolumeLeftRight.hw.h;
			break;

		case 0x1f801db8:
			data = currentMainVolumeLeftRight.hw.l;
			break;

		case 0x1f801dba:
			data = currentMainVolumeLeftRight.hw.h;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled addr read16 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801dc0) && (addr <= 0x1f801dfc)) {
		data = reverb[addr - 0x1f801dc0];
	}
	else if ((addr >= 0x1f801e00) && (addr < 0x1f801e60)) {
		std::cout << "[SPU] EMULATION PASUED! unhandled addr read16 0x" << std::hex << addr << "\n";
		g_emulationPaused = true;
	}
}

void spu_t::write16(const uint32_t& addr, const uint16_t& data) {
	if ((addr >= 0x1f801c00) && (addr <= 0x1f801d7e)) {
		uint8_t voiceIndex = (addr >> 4) - 0x1f801c0;
		switch (addr & 0xf) {
		case 0x0:
			voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.l = data;
			break;

		case 0x2:
			voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.h = data;
			break;

		case 0x4:
			voices.voiceRegisters[voiceIndex].adpcmSampleRate.data = data;
			break;

		case 0x6:
			voices.voiceRegisters[voiceIndex].adpcmStartAddress.data = data;
			//std::cout << "voice " << std::dec << (uint16_t)voiceIndex << "   " << std::hex << voices.voiceRegisters[voiceIndex].adpcmStartAddress.data << std::endl;
			break;

		case 0x8:
			/*std::cout << "[SPU] adsr 0x8 0x" << std::hex << data << " voice index " << (uint16_t)voiceIndex << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.voiceRegisters[voiceIndex].adsr.hw.l = data;
			break;

		case 0xa:
			/*std::cout << "[SPU] adsr 0xa 0x" << std::hex << data << " voice index " << (uint16_t)voiceIndex << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.voiceRegisters[voiceIndex].adsr.hw.h = data;
			break;

		case 0xc:
			voices.voiceRegisters[voiceIndex].adsrCurrentVolume.data = data;
			break;

		case 0xe:
			voices.voiceRegisters[voiceIndex].adpcmRepeatAddress.data = data;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801d80) && (addr <= 0x1f801dbc)) {
		switch (addr) {
		case 0x1f801d80:
			mainVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801d82:
			mainVolumeLeftRight.hw.h = data;
			break;

		case 0x1f801d84:
			reverbOutputVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801d86:
			reverbOutputVolumeLeftRight.hw.h = data;
			break;

		case 0x1f801d88:
			/*std::cout << "[SPU] Key On8 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOn.hw.l |= data;
			voices.tempKeyOn.hw.l = data;
			for (int voice = 0; voice < 24; voice++) {
				if (((uint32_t)data >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;
					voices.voiceRegisters[voice].startSampleBlockProcesing = true;
					voices.voiceRegisters[voice].attack = true;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = false;
					voices.voiceRegisters[voice].adsrLevel = 0;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//std::cout << "voice " << std::dec << (uint16_t)voice << "   " << std::hex << voices.voiceRegisters[voice].currentVoiceAddress << std::endl;
				}
			}
			voices.channelOnOff.hw.l &= ~voices.keyOn.hw.l;
			break;

		case 0x1f801d8a:
			/*std::cout << "[SPU] Key Ona 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOn.hw.h |= data;
			voices.tempKeyOn.hw.h = data;
			for (int voice = 0; voice < 24; voice++) {
				if ((((uint32_t)data << 16) >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;
					voices.voiceRegisters[voice].startSampleBlockProcesing = true;
					voices.voiceRegisters[voice].attack = true;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = false;
					voices.voiceRegisters[voice].adsrLevel = 0;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//std::cout << "voice " << std::dec << (uint16_t)voice << "   " << std::hex << voices.voiceRegisters[voice].currentVoiceAddress << std::endl;
				}
			}
			voices.channelOnOff.hw.h &= ~voices.keyOn.hw.h;
			break;

		case 0x1f801d8c:
			/*std::cout << "[SPU] Key Offc 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOff.hw.l |= data;
			voices.tempKeyOff.hw.l = data;
			for (uint32_t voice = 0; voice < 24; voice++) {
				if (((uint32_t)data >> voice) & 0x1) {
					//if (voices.voiceRegisters[voice].sustain) {
					voices.voiceRegisters[voice].attack = false;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = true;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//}
				}
			}
			break;

		case 0x1f801d8e:
			/*std::cout << "[SPU] Key Offe 0x" << std::hex << data << " system clocks: " << std::dec << g_systemClocks << " system frames: " << std::dec << g_systemFrames
				<< " system seconds: " << std::dec << g_systemSeconds << " system minutes: " << std::dec << g_systemMinutes << std::endl;*/
			voices.keyOff.hw.h |= data;
			voices.tempKeyOff.hw.h = data;
			for (uint32_t voice = 0; voice < 24; voice++) {
				if ((((uint32_t)data << 16) >> voice) & 0x1) {
					//if (voices.voiceRegisters[voice].sustain) {
					voices.voiceRegisters[voice].attack = false;
					voices.voiceRegisters[voice].decay = false;
					voices.voiceRegisters[voice].sustain = false;
					voices.voiceRegisters[voice].release = true;
					voices.voiceRegisters[voice].adsrCycles = 0;
					//	}
				}
			}
			break;

		case 0x1f801d90:
			voices.channel_FM_Mode.hw.l = data;
			break;

		case 0x1f801d92:
			voices.channel_FM_Mode.hw.h = data;
			break;

		case 0x1f801d94:
			voices.channelNoiseMode.hw.l = data;
			break;

		case 0x1f801d96:
			voices.channelNoiseMode.hw.h = data;
			break;

		case 0x1f801d98:
			voices.channelReverbMode.hw.l = data;
			break;

		case 0x1f801d9a:
			voices.channelReverbMode.hw.h = data;
			break;

		case 0x1f801d9c:
			break;

		case 0x1f801d9e:
			break;

		case 0x1f801da2:
			dramReverbWorkAreaStartAddress.data = data;
			break;

		case 0x1f801da4:
			dram_IRQ_Address.data = data;
			break;

		case 0x1f801da6:
			dramDataTransferAddress.data = data;
			currentAddress = dramDataTransferAddress.data * 8;
			break;

		case 0x1f801da8:
			dramDataTransferFifo.data = data;
			writeDram16(currentAddress, data);
			currentAddress += 2;
			break;

		case 0x1f801daa:
			spuControlRegister.data = data;

			spuStatusRegister.elem.currentSPUMode = spuControlRegister.data & 0x003f;
			if (spuControlRegister.elem.soundRAMTransferMode == 2)
				spuStatusRegister.elem.dataTransferDMAWriteRequest = 1;
			spuStatusRegister.elem.dataTransferDMAReadWriteRequest = (spuControlRegister.data & 0x0020) ? 1 : 0;

			if (spuControlRegister.elem.soundRAMTransferMode == 3) {
				spuStatusRegister.elem.dataTransferDMAReadRequest = 1;
				//std::cout << "[SPU] EMULATION PASUED! unhandled DMA read" << "\n";
				//g_emulationPaused = true;
			}

			if (spuControlRegister.elem.irq9Enable == 0) {
				//std::cout << "[SPU] EMULATION PASUED! unhandled IRQ" << "\n";
				//g_emulationPaused = true;
				spuStatusRegister.elem.irq9Flag = 0;
			}

			break;

		case 0x1f801dac:
			dramDataTransferControl.data = data;
			if (data != 0x0004) {
				std::cout << "[SPU] EMULATION PASUED! unhandled dramDataTransferControl = 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
			break;

		case 0x1f801db0:
			cdVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801db2:
			cdVolumeLeftRight.hw.h = data;
			break;

		case 0x1f801db4:
			externVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801db6:
			externVolumeLeftRight.hw.h = data;
			break;

		case 0x1f801db8:
			currentMainVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801dba:
			currentMainVolumeLeftRight.hw.h = data;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else if ((addr >= 0x1f801dc0) && (addr <= 0x1f801dfc)) {
		reverb[addr - 0x1f801dc0] = data;
	}
	else if ((addr >= 0x1f801e00) && (addr < 0x1f801e60)) {
		std::cout << "[SPU] EMULATION PASUED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
}

float spu_t::clock() {
	output_t out;
	out.channel1 = 0.0f;
	out.channel2 = 0.0f;
	for (int i = 0; i < 24; i++) {
		out = out + voices.voiceRegisters[i].envelope(voices.output(i));
	}
	out.channel1 = out.channel1 * (float)(mainVolumeLeftRight.hw.l << 1) / 32767.0f;
	out.channel2 = out.channel2 * (float)(mainVolumeLeftRight.hw.h << 1) / 32767.0f;
	return out.channel1 / 2.0f + out.channel2 / 2.0f;
}

void spu_t::readDram16(const uint32_t& addr, uint16_t& data) {
	data = (uint16_t)dram[addr];
	data |= (uint16_t)dram[addr + 1] << 8;
}

void spu_t::writeDram16(const uint32_t& addr, const uint16_t& data) {
	dram[addr] = (uint8_t)data;
	dram[addr + 1] = (uint8_t)(data >> 8);
}

void spu_t::readDram8(const uint32_t& addr, uint8_t& data) {
	data = (uint8_t)dram[addr];
}

void spu_t::writeDram8(const uint32_t& addr, const uint8_t& data) {
	dram[addr] = (uint8_t)data;
}

void spu_t::voices_t::decodeCompressedSample(voiceRegister_t& currentVoiceRegisters) {
	//should be the same as XA-ADPCM
	int8_t shift = 12 - ((int8_t)currentVoiceRegisters.shiftFilter.data & 0x0f);
	int8_t filter = ((int8_t)currentVoiceRegisters.shiftFilter.data & 0x70) >> 4; //?? SPU-ADPCM has 5 filters
	if (filter > 5) {
		std::cout << "[SPU] EMULATION PASUED! ADPCM filter > 5 filter = " << std::dec << (uint16_t)filter << "\n";
		//g_emulationPaused = true;
	}

	int32_t fo = p_spu->pos_adpcm_table[filter];
	int32_t f1 = p_spu->neg_adpcm_table[filter];

	//this values should be 0
	int32_t old = 0;
	int32_t older = 0;
	for (int i = 0; i < 28; i++) {
		int32_t t = (int8_t)(currentVoiceRegisters.compressedSamples[i] << 4) >> 4;
		int32_t s = (t << shift) + ((old * fo + older * f1 + 32) / 64);

		if (s > 0x7fff) s = 0x7fff;
		if (s < -0x8000) s = -0x8000;
		older = old;
		old = s;

		currentVoiceRegisters.samples[i] = (int16_t)s;
	}
}

int16_t spu_t::voices_t::output(const uint8_t& voice) {
	if (voiceRegisters[voice].attack || voiceRegisters[voice].decay ||
		voiceRegisters[voice].sustain || voiceRegisters[voice].release) {
		voiceRegister_t currentVoiceRegisters = voiceRegisters[voice];

		if (currentVoiceRegisters.startSampleBlockProcesing) {
			currentVoiceRegisters.startSampleBlockProcesing = false;

			currentVoiceRegisters.counter = 0;

			for (int i = 0; i < 16; i++) {
				uint8_t data;
				p_spu->readDram8(currentVoiceRegisters.currentVoiceAddress + i, data);
				currentVoiceRegisters.blockBytes[i] = data;;
			}

			currentVoiceRegisters.shiftFilter.data = currentVoiceRegisters.blockBytes[0];

			currentVoiceRegisters.flagBits.data = currentVoiceRegisters.blockBytes[1];

			if (currentVoiceRegisters.flagBits.elem.loopStart)
				currentVoiceRegisters.adpcmRepeatAddress.data = currentVoiceRegisters.currentVoiceAddress / 8;

			if (currentVoiceRegisters.flagBits.elem.loopEnd)
				currentVoiceRegisters.currentVoiceAddress = currentVoiceRegisters.adpcmRepeatAddress.data * 8;

			currentVoiceRegisters.code = currentVoiceRegisters.flagBits.data & 0x3;

			switch (currentVoiceRegisters.code) {
			case 0:
			case 2:
				currentVoiceRegisters.currentVoiceAddress += 16;
				break;
			case 1:
				currentVoiceRegisters.currentVoiceAddress = currentVoiceRegisters.adpcmRepeatAddress.data * 8;
				p_spu->voices.channelOnOff.data |= (uint32_t)1 << voice;
				voiceRegisters[voice].attack = false;
				voiceRegisters[voice].decay = false;
				voiceRegisters[voice].sustain = false;
				voiceRegisters[voice].release = true;
				voiceRegisters[voice].adsrCycles = 0;
				//currentVoiceRegisters.adsr.data = 0x0000;
				currentVoiceRegisters.adsrLevel = 0;//??
				break;
			case 3:
				currentVoiceRegisters.currentVoiceAddress = currentVoiceRegisters.adpcmRepeatAddress.data * 8;
				p_spu->voices.channelOnOff.data |= (uint32_t)1 << voice;
				break;
			}

			for (int i = 0; i < 28; i++) {
				currentVoiceRegisters.compressedSamples[i] = (currentVoiceRegisters.blockBytes[2 + i / 2] >> ((i % 2) * 4)) & 0xf;
			}

			decodeCompressedSample(currentVoiceRegisters);

			//??
			currentVoiceRegisters.oldest = 0;
			currentVoiceRegisters.older = 0;
			currentVoiceRegisters.old = 0;
			currentVoiceRegisters.newest = 0;
		}

		uint16_t currentSampleNumber = currentVoiceRegisters.counter >> 12;
		uint16_t i = (currentVoiceRegisters.counter >> 4) & 0x1f;

		currentVoiceRegisters.step = currentVoiceRegisters.adpcmSampleRate.data;
		if (((channel_FM_Mode.data >> voice) & 0x1) && (voice > 1)) {
			int16_t factor = (int16_t)voiceRegisters[voice - 1].adsrLevel;
			factor = factor + 0x8000;
			int32_t tempStep = (int32_t)(currentVoiceRegisters.step);
			tempStep = (tempStep * (uint16_t)factor) >> 15;
			currentVoiceRegisters.step = tempStep & 0x0000ffff;
		}
		if (currentVoiceRegisters.step > 0x3fff)
			currentVoiceRegisters.step = 0x4000;
		currentVoiceRegisters.counter += currentVoiceRegisters.step;

		if (currentSampleNumber > 27) {
			currentVoiceRegisters.counter = 0;
			currentSampleNumber = 27;
			currentVoiceRegisters.startSampleBlockProcesing = true;
		}

		currentVoiceRegisters.sample = currentVoiceRegisters.samples[currentSampleNumber];
		currentVoiceRegisters.pushSample(currentVoiceRegisters.sample);

		currentVoiceRegisters.out = ((int32_t)(p_spu->gauss[0x0ff - i] * currentVoiceRegisters.oldest) >> 15);
		currentVoiceRegisters.out = currentVoiceRegisters.out + (int16_t)((int32_t)(p_spu->gauss[0x1ff - i] * currentVoiceRegisters.older) >> 15);
		currentVoiceRegisters.out = currentVoiceRegisters.out + (int16_t)((int32_t)(p_spu->gauss[0x100 + i] * currentVoiceRegisters.old) >> 15);
		currentVoiceRegisters.out = currentVoiceRegisters.out + (int16_t)((int32_t)(p_spu->gauss[0x000 + i] * currentVoiceRegisters.newest) >> 15);

		voiceRegisters[voice] = currentVoiceRegisters;

		return currentVoiceRegisters.out;
	}

	return 0;
}

static int32_t saturate(int32_t a, int32_t min, int32_t max) {
	if (a < min)
		return min;
	else if (a > max)
		return max;
	return a;
}

static int32_t max(int32_t a, int32_t b) {
	return (a > b) ? a : b;
}

output_t spu_t::voices_t::voiceRegister_t::envelope(const int16_t& input) {

	if (release) {
		if (adsrCycles == 0) {
			uint32_t shiftValue = adsr.elem.releaseShift;
			adsrCycles = 1 << max(0, shiftValue - 11);
			//adsrCycles *= 4000;
			int32_t stepValue = -8;
			adsrStep = stepValue << max(0, 11 - shiftValue);

			bool exponential = adsr.elem.releaseMode;
			if (exponential)
				adsrStep = adsrStep * adsrLevel / 0x8000;

			adsrLevel = saturate(adsrLevel + adsrStep, 0, 0x7fff);

			if (adsrLevel == 0) {
				//release = false;
				adsrCycles = 1;
			}
		}
		adsrCycles--;
	}

	if (attack) {
		if (adsrCycles == 0) {
			uint32_t shiftValue = adsr.elem.attackShift;
			adsrCycles = 1 << max(0, shiftValue - 11);

			int32_t stepValue = 7 - adsr.elem.attackStep;
			adsrStep = stepValue << max(0, 11 - adsr.elem.attackShift);

			bool exponential = adsr.elem.attackMode;
			if (exponential && (adsrLevel > 0x6000))
				adsrCycles *= 4;

			adsrLevel = saturate(adsrLevel + adsrStep, 0, 0x7fff);

			if (adsrLevel == 0x7fff) {
				attack = false;
				decay = true;
				adsrCycles = 1;
			}
		}
		adsrCycles--;
	}
	else if (decay) {
		if (adsrCycles == 0) {
			uint32_t shiftValue = adsr.elem.decayShift;
			adsrCycles = 1 << max(0, shiftValue - 11);

			int32_t stepValue = -8;
			adsrStep = stepValue << max(0, 11 - adsr.elem.decayShift);

			adsrStep = adsrStep * adsrLevel / 0x8000;

			adsrLevel = saturate(adsrLevel + adsrStep, 0, 0x7fff);

			if (adsrLevel <= (adsr.elem.sustainLevel + 1) * 0x800) {
				adsrLevel = (adsr.elem.sustainLevel + 1) * 0x800;
				decay = false;
				sustain = true;
				adsrCycles = 1;
			}
		}
		adsrCycles--;

	}
	else if (sustain) {
		if (adsrCycles == 0) {
			uint32_t shiftValue = adsr.elem.sustainShift;
			adsrCycles = 1 << max(0, shiftValue - 11);

			bool increase = !adsr.elem.sustainDirection;
			int32_t stepValue =
				(increase) ? -8 + adsr.elem.sustainStep : 7 - adsr.elem.sustainStep;
			adsrStep = stepValue << max(0, 11 - shiftValue);


			bool exponential = adsr.elem.sustainMode;
			if (exponential && increase && (adsrLevel > 0x6000))
				adsrCycles *= 4;
			else if (exponential && !increase)
				adsrStep = adsrStep * adsrLevel / 0x8000;

			adsrLevel = saturate(adsrLevel + adsrStep, 0, 0x7fff);
		}
		adsrCycles--;
	}

	output_t output;
	output.channel1 = (float)input / 32767.0f * /*(float)adsrLevel / 32767.0f*/  (float)(volumeLeftRight.hw.l << 1) / 32767.0f;
	output.channel2 = (float)input / 32767.0f * /*(float)adsrLevel / 32767.0f*/  (float)(volumeLeftRight.hw.h << 1) / 32767.0f;

	return output;
}
