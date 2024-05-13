#include "spu.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;
extern uint32_t g_systemClocks;
extern uint32_t g_systemFrames;
extern uint32_t g_systemSeconds;
extern uint32_t g_systemMinutes;

spu_t::spu_t() {
	voices.p_spu = this;

	pos_adpcm_table.resize(5);
	pos_adpcm_table = { 0, +60, +115, +98, +122 };

	neg_adpcm_table.resize(5);
	neg_adpcm_table = { 0, 0, -52, -55, -60 };

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

	table.resize(203);
	table = {
		+0x0000,+0x0000,+0x0000,+0x0000,-0x0001,+0x0002,-0x0005,
		+0x0000,+0x0000,+0x0000,-0x0001,+0x0003,-0x0008,+0x0011,
		+0x0000,+0x0000,-0x0001,+0x0003,-0x0008,+0x0010,-0x0023,
		+0x0000,-0x0002,+0x0003,-0x0008,+0x0011,-0x0023,+0x0046,
		+0x0000,+0x0000,-0x0002,+0x0006,-0x0010,+0x002B,-0x0017,
		-0x0002,+0x0003,-0x0005,+0x0005,+0x000A,+0x001A,-0x0044,
		+0x000A,-0x0013,+0x001F,-0x001B,+0x006B,-0x00EB,+0x015B,
		-0x0022,+0x003C,-0x004A,+0x00A6,-0x016D,+0x027B,-0x0347,
		+0x0041,-0x004B,+0x00B3,-0x01A8,+0x0350,-0x0548,+0x080E,
		-0x0054,+0x00A2,-0x0192,+0x0372,-0x0623,+0x0AFA,-0x1249,
		+0x0034,-0x00E3,+0x02B1,-0x05BF,+0x0BCD,-0x16FA,+0x3C07,
		+0x0009,+0x0132,-0x039E,+0x09B8,-0x1780,+0x53E0,+0x53E0,
		-0x010A,-0x0043,+0x04F8,-0x11B4,+0x6794,+0x3C07,-0x16FA,
		+0x0400,-0x0267,-0x05A6,+0x74BB,+0x234C,-0x1249,+0x0AFA,
		-0x0A78,+0x0C9D,+0x7939,+0x0C9D,-0x0A78,+0x080E,-0x0548,
		+0x234C,+0x74BB,-0x05A6,-0x0267,+0x0400,-0x0347,+0x027B,
		+0x6794,-0x11B4,+0x04F8,-0x0043,-0x010A,+0x015B,-0x00EB,
		-0x1780,+0x09B8,-0x039E,+0x0132,+0x0009,-0x0044,+0x001A,
		+0x0BCD,-0x05BF,+0x02B1,-0x00E3,+0x0034,-0x0017,+0x002B,
		-0x0623,+0x0372,-0x0192,+0x00A2,-0x0054,+0x0046,-0x0023,
		+0x0350,-0x01A8,+0x00B3,-0x004B,+0x0041,-0x0023,+0x0010,
		-0x016D,+0x00A6,-0x004A,+0x003C,-0x0022,+0x0011,-0x0008,
		+0x006B,-0x001B,+0x001F,-0x0013,+0x000A,-0x0005,+0x0002,
		+0x000A,+0x0005,-0x0005,+0x0003,-0x0001,+0x0000,+0x0000,
		-0x0010,+0x0006,-0x0002,+0x0000,+0x0000,+0x0000,+0x0000,
		+0x0011,-0x0008,+0x0003,-0x0002,+0x0001,+0x0000,+0x0000,
		-0x0008,+0x0003,-0x0001,+0x0000,+0x0000,+0x0000,+0x0000,
		+0x0003,-0x0001,+0x0000,+0x0000,+0x0000,+0x0000,+0x0000,
		-0x0001,+0x0000,+0x0000,+0x0000,+0x0000,+0x0000,+0x0000,
	};

	voices.channelOnOff.data = 0x00ffffff;
}

void spu_t::read32(const uint32_t& addr, uint32_t& data) {
	data = 0;

	if ((addr >= 0x1f801d80) && (addr <= 0x1f801dbc)) {
		switch (addr) {
		case 0x1f801d80:
			data = mainVolumeLeftRight.data;
			break;

		case 0x1f801d9c:
			data = voices.channelOnOff.data;
			break;

		case 0x1f801dbc:
			//Unknown??
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else {
		std::cout << "[SPU] EMULATION PASUED! unhandled addr read32 0x" << std::hex << addr << "\n";
		g_emulationPaused = true;
	}
}

void spu_t::write32(const uint32_t& addr, const uint32_t& data) {
	if ((addr >= 0x1f801c00) && (addr <= 0x1f801d7e)) {
		uint8_t voiceIndex = (addr >> 4) - 0x1f801c0;
		switch (addr & 0xf) {
		case 0x0:
			voices.voiceRegisters[voiceIndex].volumeLeftRight.data = data;

			if (voices.voiceRegisters[voiceIndex].volumeLeftRight.data & 0x80008000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode voice\n";
				g_emulationPaused = true;
			}
			break;

		case 0x8:
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

			if (mainVolumeLeftRight.data & 0x80008000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode main\n";
				g_emulationPaused = true;
			}
			break;

		case 0x1f801d84:
			reverbOutputVolumeLeftRight.data = data;
			break;

		case 0x1f801d88:
			voices.keyOn.data |= data;
			voices.tempKeyOn.data = data;

			for (int voice = 0; voice < 24; voice++)
				if ((data >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;

					voices.voiceRegisters[voice].counter = 0x1c000;

					voices.voiceRegisters[voice].startEnvelopeAttack();
				}

			voices.channelOnOff.data &= ~voices.keyOn.data;
			break;

		case 0x1f801d8c:
			voices.keyOff.data |= data;
			voices.tempKeyOff.data = data;

			for (uint32_t voice = 0; voice < 24; voice++)
				if (((uint32_t)data >> voice) & 0x1)
					voices.voiceRegisters[voice].startEnvelopeRelease();

			break;

		case 0x1f801d90:
			voices.channel_FM_Mode.data = data;

			if (data)
				std::cout << "[SPU] Pitch Modulation Enable enabled 0x" << std::hex << data << std::endl;
			break;

		case 0x1f801d94:
			voices.channelNoiseMode.data = data;

			if (data) {
				std::cout << "[SPU] EMULATION PASUED! Noise enabled 0x" << std::hex << data << std::endl;
				g_emulationPaused = true;
			}
			break;

		case 0x1f801d98:
			voices.channelReverbMode.data = data;
			break;

		case 0x1f801dac:
			dramDataTransferControl.data = (uint16_t)data;

			if (data != 0x0004) {
				std::cout << "[SPU] EMULATION PASUED! unhandled dramDataTransferControl = 0x" << std::hex << data << "\n";
				g_emulationPaused = true;
			}
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
		*((uint32_t*)(reverb + addr - 0x1f801dc0)) = data;
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
		case 0x0:
			data = voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.l;
			break;

		case 0x2:
			data = voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.h;
			break;

		case 0x4:
			data = voices.voiceRegisters[voiceIndex].adpcmSampleRate.data;
			break;

		case 0x6:
			data = voices.voiceRegisters[voiceIndex].adpcmStartAddress.data;
			break;

		case 0x8:
			data = voices.voiceRegisters[voiceIndex].adsr.hw.l;
			break;

		case 0xa:
			data = voices.voiceRegisters[voiceIndex].adsr.hw.h;
			break;

		case 0xc:
			data = voices.voiceRegisters[voiceIndex].adsrLevel;
			break;

		case 0xe:
			data = voices.voiceRegisters[voiceIndex].adpcmRepeatAddress.data;
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

		case 0x1f801d84:
			data = reverbOutputVolumeLeftRight.hw.l;
			break;

		case 0x1f801d86:
			data = reverbOutputVolumeLeftRight.hw.h;
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

		case 0x1f801d9c:
			data = voices.channelOnOff.hw.l;
			break;

		case 0x1f801d9e:
			data = voices.channelOnOff.hw.h;
			break;

		case 0x1f801da0:
			//Unknown??
			break;

		case 0x1f801da2:
			data = dramReverbWorkAreaStartAddress.data;
			break;

		case 0x1f801da4:
			data = dram_IRQ_Address.data;
			break;

		case 0x1f801da6:
			data = dramDataTransferAddress.data;
			break;

		case 0x1f801da8:
			data = readDram16(currentAddress);
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
	else if ((addr >= 0x1f801dc0) && (addr <= 0x1f801dfe)) {
		data = reverb[(addr - 0x1f801dc0) >> 1];
	}
	else if ((addr >= 0x1f801e00) && (addr < 0x1f801e60)) {
		uint8_t voiceIndex = (addr - 0x1f801e00) >> 2;
		switch (addr & 0x3) {
		case 0:
			data = voices.voiceRegisters[voiceIndex].currentVolumeLeftRight.hw.l;
			break;

		case 2:
			data = voices.voiceRegisters[voiceIndex].currentVolumeLeftRight.hw.h;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled addr read16 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else {
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

			if (voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.l & 0x8000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode voice\n";
				g_emulationPaused = true;
			}
			break;

		case 0x2:
			voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.h = data;

			if (voices.voiceRegisters[voiceIndex].volumeLeftRight.hw.h & 0x8000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode voice\n";
				g_emulationPaused = true;
			}
			break;

		case 0x4:
			voices.voiceRegisters[voiceIndex].adpcmSampleRate.data = data;
			break;

		case 0x6:
			voices.voiceRegisters[voiceIndex].adpcmStartAddress.data = data;
			break;

		case 0x8:
			voices.voiceRegisters[voiceIndex].adsr.hw.l = data;
			break;

		case 0xa:
			voices.voiceRegisters[voiceIndex].adsr.hw.h = data;
			break;

		case 0xc:
			voices.voiceRegisters[voiceIndex].adsrLevel = (int32_t)(int16_t)data;
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

			if (mainVolumeLeftRight.hw.l & 0x8000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode main\n";
				g_emulationPaused = true;
			}
			break;

		case 0x1f801d82:
			mainVolumeLeftRight.hw.h = data;

			if (mainVolumeLeftRight.hw.h & 0x8000) {
				std::cout << "[SPU] EMULATION PASUED! Sweep Volume Mode main\n";
				g_emulationPaused = true;
			}
			break;

		case 0x1f801d84:
			reverbOutputVolumeLeftRight.hw.l = data;
			break;

		case 0x1f801d86:
			reverbOutputVolumeLeftRight.hw.h = data;
			break;

		case 0x1f801d88:
			voices.keyOn.hw.l |= data;
			voices.tempKeyOn.hw.l = data;

			for (int voice = 0; voice < 16; voice++)
				if (((uint32_t)data >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;

					voices.voiceRegisters[voice].counter = 0x1c000;

					voices.voiceRegisters[voice].startEnvelopeAttack();
				}

			voices.channelOnOff.hw.l &= ~voices.keyOn.hw.l;
			break;

		case 0x1f801d8a:
			voices.keyOn.hw.h |= data;
			voices.tempKeyOn.hw.h = data;

			for (int voice = 16; voice < 24; voice++)
				if ((((uint32_t)data << 16) >> voice) & 0x1) {
					voices.voiceRegisters[voice].adpcmRepeatAddress.data =
						voices.voiceRegisters[voice].adpcmStartAddress.data;
					voices.voiceRegisters[voice].currentVoiceAddress =
						voices.voiceRegisters[voice].adpcmStartAddress.data * 8;

					voices.voiceRegisters[voice].counter = 0x1c000;

					voices.voiceRegisters[voice].startEnvelopeAttack();
				}

			voices.channelOnOff.hw.h &= ~voices.keyOn.hw.h;
			break;

		case 0x1f801d8c:
			voices.keyOff.hw.l |= data;
			voices.tempKeyOff.hw.l = data;

			for (uint32_t voice = 0; voice < 16; voice++)
				if (((uint32_t)data >> voice) & 0x1)
					voices.voiceRegisters[voice].startEnvelopeRelease();
			break;

		case 0x1f801d8e:
			voices.keyOff.hw.h |= data;
			voices.tempKeyOff.hw.h = data;

			for (uint32_t voice = 16; voice < 24; voice++)
				if ((((uint32_t)data << 16) >> voice) & 0x1)
					voices.voiceRegisters[voice].startEnvelopeRelease();
			break;

		case 0x1f801d90:
			voices.channel_FM_Mode.hw.l = data;

			if (data)
				std::cout << "[SPU] Pitch Modulation Enable enabled 0x" << std::hex << data << std::endl;
			break;

		case 0x1f801d92:
			voices.channel_FM_Mode.hw.h = data;

			if (data)
				std::cout << "[SPU] Pitch Modulation Enable enabled 0x" << std::hex << data << std::endl;
			break;

		case 0x1f801d94:
			voices.channelNoiseMode.hw.l = data;

			if (data) {
				std::cout << "[SPU] EMULATION PASUED! noise enabled 0x" << std::hex << data << std::endl;
				g_emulationPaused = true;
			}
			break;

		case 0x1f801d96:
			voices.channelNoiseMode.hw.h = data;

			if (data) {
				std::cout << "[SPU] EMULATION PASUED! noise enabled 0x" << std::hex << data << std::endl;
				g_emulationPaused = true;
			}
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

		case 0x1f801da0: //Unknown
			break;

		case 0x1f801da2:
			dramReverbWorkAreaStartAddress.data = data;
			reverbCurrentBufferAddress = dramReverbWorkAreaStartAddress.data << 3;
			break;

		case 0x1f801da4:
			dram_IRQ_Address.data = data;
			irqAddress = data << 3;
			break;

		case 0x1f801da6:
			dramDataTransferAddress.data = data;
			currentAddress = dramDataTransferAddress.data << 3;

			std::cout << "[SPU] dramDataTransferAddress : 0x"
				<< std::hex << currentAddress << "\n";

			if (currentAddress >= DRAM_SIZE) {
				std::cout << "[SPU] EMULATION PASUED! dramDataTransferAddress : 0x"
					<< std::hex << currentAddress << "\n";
				g_emulationPaused = true;
			}
			break;

		case 0x1f801da8:
			dramDataTransferFifo.data = data;

			if (currentAddress >= DRAM_SIZE) {  // spyro 1 sends 0x10 more hw's and also Monkey Hero //??maybe warping??
				currentAddress = dramDataTransferAddress.data << 3;//??

				std::cout << "[SPU] current address > DRAM_SIZE : 0x"
					<< std::hex << currentAddress << " data 0x" << std::hex << data << "\n";
			}
			else
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

			if (data != 0x04) {
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
	else if ((addr >= 0x1f801dc0) && (addr <= 0x1f801dfe)) {
		reverb[(addr - 0x1f801dc0) >> 1] = data;
	}
	else if ((addr >= 0x1f801e00) && (addr < 0x1f801e60)) {
		std::cout << "[SPU] EMULATION PASUED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
	else {
		std::cout << "[SPU] EMULATION PASUED! unhandled write16 adress 0x" << std::hex << addr << " data 0x" << std::hex << data << "\n";
		g_emulationPaused = true;
	}
}

void spu_t::read8(const uint32_t& addr, uint8_t& data) {
	data = 0;

	if ((addr >= 0x1f801c00) && (addr <= 0x1f801d7e)) {
		uint8_t voiceIndex = (addr >> 4) - 0x1f801c0;
		switch (addr & 0xf) {

		case 0x8:
			data = voices.voiceRegisters[voiceIndex].adsr.b.l;
			break;

		default:
			std::cout << "[SPU] EMULATION PASUED! unhandled addr read8 0x" << std::hex << addr << "\n";
			g_emulationPaused = true;
			break;
		}
	}
	else {
		std::cout << "[SPU] EMULATION PASUED! unhandled addr read8 0x" << std::hex << addr << "\n";
		g_emulationPaused = true;
	}
}

void spu_t::write8(const uint32_t& addr, const uint8_t& data) {
	std::cout << "[SPU] EMULATION PASUED! unhandled write8 adress 0x" << std::hex << addr << " data 0x" << std::hex << (uint16_t)data << "\n";
	g_emulationPaused = true;
}

void spu_t::irqSend() {
	if (spuControlRegister.elem.irq9Enable) //metal gear solid
		if (spuStatusRegister.elem.irq9Flag == 0) {
			pBus->pCpu->cp0.interruptHandler(_IRQ_SPU);
			spuStatusRegister.elem.irq9Flag = 1;
		}
}

uint16_t spu_t::readDram16(const uint32_t& addr) {
	if ((irqAddress == addr) || ((irqAddress & 0xfffffffe) == addr))
		irqSend();

	return *(uint16_t*)(dram + addr);
}

void spu_t::writeDram16(const uint32_t& addr, const uint16_t& data) {
	if ((irqAddress == addr) || ((irqAddress & 0xfffffffe) == addr))
		irqSend();

	*(uint16_t*)(dram + addr) = data;
}

void spu_t::readDram8(const uint32_t& addr, uint8_t& data) {
	if (irqAddress == addr)
		irqSend();

	data = dram[addr];
}

void spu_t::writeDram8(const uint32_t& addr, const uint8_t& data) {
	if (irqAddress == addr)
		irqSend();

	dram[addr] = data;
}

void spu_t::voices_t::decodeCompressedSample(voiceRegister_t& currentVoiceRegisters) {
	//should be the same as XA-ADPCM

	uint8_t valueShift = (currentVoiceRegisters.shiftFilter.data & 0x0f);
	if (valueShift > 12)
		valueShift = 9;

	uint8_t shift = 12 - valueShift;

	uint8_t filter = (currentVoiceRegisters.shiftFilter.data & 0x70) >> 4; //?? SPU-ADPCM has 5 filters
	if (filter > 5) {
		std::cout << "[SPU] EMULATION PASUED! ADPCM filter > 5 filter = " << std::dec << (uint16_t)filter << "\n";
		//g_emulationPaused = true;
	}

	int32_t fo = p_spu->pos_adpcm_table[filter];
	int32_t f1 = p_spu->neg_adpcm_table[filter];


	for (int i = 0; i < 28; i++) {
		int16_t t = int16_t(currentVoiceRegisters.compressedSamples[i] << 12) >> 12;
		int16_t s = (t << shift) + ((currentVoiceRegisters.old * fo + currentVoiceRegisters.older * f1 + 32) / 64);

		if (s > 0x7fff) s = 0x7fff;
		else if (s < -0x8000) s = -0x8000;

		currentVoiceRegisters.older = currentVoiceRegisters.old;
		currentVoiceRegisters.old = s;

		currentVoiceRegisters.samples[i] = (int16_t)s;
	}
}

int16_t spu_t::voices_t::output(const uint8_t& voice) {
	voiceRegister_t currentVoiceRegisters = voiceRegisters[voice];

	uint16_t currentSampleNumber = currentVoiceRegisters.counter >> 12;
	uint16_t i = (currentVoiceRegisters.counter >> 4) & 0xff;

	currentVoiceRegisters.step = currentVoiceRegisters.adpcmSampleRate.data;
	if (((channel_FM_Mode.data >> voice) & 0x1) && (voice > 0)) {
		uint16_t factor = (uint16_t)voiceRegisters[voice - 1].adsrLevel; //??
		factor = factor + 0x8000;
		uint32_t tempStep = int32_t((int16_t)currentVoiceRegisters.step);
		tempStep = (tempStep * (uint16_t)factor) >> 15;
		currentVoiceRegisters.step = tempStep & 0x0000ffff;
	}
	if (currentVoiceRegisters.step > 0x3fff)
		currentVoiceRegisters.step = 0x4000;

	currentVoiceRegisters.counter += currentVoiceRegisters.step;

	if (currentSampleNumber > 27) {
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

			currentVoiceRegisters.startEnvelopeRelease();
			currentVoiceRegisters.adsrLevel = 0;
			break;
		case 3:
			currentVoiceRegisters.currentVoiceAddress = currentVoiceRegisters.adpcmRepeatAddress.data * 8;

			p_spu->voices.channelOnOff.data |= (uint32_t)1 << voice;
			break;
		}

		for (int i = 0; i < 28; i++)
			currentVoiceRegisters.compressedSamples[i] = (currentVoiceRegisters.blockBytes[2 + i / 2] >> ((i % 2) * 4)) & 0xf;

		decodeCompressedSample(currentVoiceRegisters);
	}

	currentVoiceRegisters.sample = currentVoiceRegisters.samples[currentSampleNumber];
	currentVoiceRegisters.pushSample(currentVoiceRegisters.sample);

	currentVoiceRegisters.out = ((int32_t)p_spu->gauss[0x0ff - i] * currentVoiceRegisters.voiceOldest) >> 15;
	currentVoiceRegisters.out += ((int32_t)p_spu->gauss[0x1ff - i] * currentVoiceRegisters.voiceOlder) >> 15;
	currentVoiceRegisters.out += ((int32_t)p_spu->gauss[0x100 + i] * currentVoiceRegisters.voiceOld) >> 15;
	currentVoiceRegisters.out += ((int32_t)p_spu->gauss[0x000 + i] * currentVoiceRegisters.voiceNewest) >> 15;

	voiceRegisters[voice] = currentVoiceRegisters;

	return currentVoiceRegisters.sample;
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

void spu_t::voices_t::voiceRegister_t::startEnvelopeAttack() {
	adsrLevel = 0;

	bEnvelopeAttack = true;
	bEnvelopeDecay = false;
	bEnvelopeSustain = false;
	bEnvelopeRelease = false;

	uint32_t shiftValue = adsr.elem.attackShift;
	adsrCycles = 1 << max(0, (int32_t)shiftValue - 11);

	int32_t stepValue = 7 - adsr.elem.attackStep;
	adsrStep = stepValue << max(0, 11 - (int32_t)adsr.elem.attackShift);

	bool exponential = adsr.elem.attackMode;
	if (exponential && (adsrLevel > 0x6000))
		adsrCycles *= 4;

	adsrCyclesNumber = adsrCycles;
	adsrCycles = 0;
}

void spu_t::voices_t::voiceRegister_t::startEnvelopeDecay() {
	bEnvelopeAttack = false;
	bEnvelopeDecay = true;
	bEnvelopeSustain = false;
	bEnvelopeRelease = false;

	uint32_t shiftValue = adsr.elem.decayShift;
	adsrCycles = 1 << max(0, (int32_t)shiftValue - 11);

	int32_t stepValue = -8;
	adsrStep = stepValue << max(0, 11 - (int32_t)adsr.elem.decayShift);

	adsrStep = adsrStep * adsrLevel / 0x8000;

	adsrCyclesNumber = adsrCycles;
}

void spu_t::voices_t::voiceRegister_t::startEnvelopeSustain() {
	bEnvelopeAttack = false;
	bEnvelopeDecay = false;
	bEnvelopeSustain = true;
	bEnvelopeRelease = false;

	uint32_t shiftValue = adsr.elem.sustainShift;
	adsrCycles = 1 << max(0, shiftValue - 11);

	bool increase = !adsr.elem.sustainDirection;
	int32_t stepValue =
		increase ? 7 - adsr.elem.sustainStep : -8 + adsr.elem.sustainStep;
	adsrStep = stepValue << max(0, 11 - shiftValue);

	bool exponential = adsr.elem.sustainMode;
	if (exponential && increase && (adsrLevel > 0x6000))
		adsrCycles *= 4;
	else if (exponential && !increase)
		if (adsrStep * adsrLevel >= 0x8000) //??
			adsrStep = adsrStep * adsrLevel >> 15;

	adsrCyclesNumber = adsrCycles;
}

void spu_t::voices_t::voiceRegister_t::startEnvelopeRelease() {
	bEnvelopeAttack = false;
	bEnvelopeDecay = false;
	bEnvelopeSustain = false;
	bEnvelopeRelease = true;

	uint32_t shiftValue = adsr.elem.releaseShift;
	adsrCycles = 1 << max(0, (int32_t)shiftValue - 11);

	int32_t stepValue = -8;
	adsrStep = stepValue << max(0, 11 - (int32_t)shiftValue);

	bool exponential = adsr.elem.releaseMode;
	if (exponential)
		if (adsrStep * adsrLevel >= 0x8000) //??
			adsrStep = adsrStep * adsrLevel >> 15;

	adsrCyclesNumber = adsrCycles;
	adsrCycles = 0;
}

output_t spu_t::voices_t::voiceRegister_t::envelope(const int16_t& input) {

	if (bEnvelopeAttack || bEnvelopeDecay ||
		bEnvelopeSustain || bEnvelopeRelease)
		if (adsrCycles == 0) {
			adsrCycles = adsrCyclesNumber;

			adsrLevel = saturate(adsrLevel + adsrStep, 0, 0x7fff);

			if (bEnvelopeAttack) {
				if (adsrLevel == 0x7fff)
					startEnvelopeDecay();
			}
			else if (bEnvelopeDecay) {
				if (adsrLevel <= (adsr.elem.sustainLevel + 1) * 0x800) {
					adsrLevel = (adsr.elem.sustainLevel + 1) * 0x800;

					startEnvelopeSustain();
				}
			}
			else if (bEnvelopeSustain) {
			}
			else if (bEnvelopeRelease) {
				if (adsrLevel == 0)
					bEnvelopeRelease = false;
			}
		}
		else
			adsrCycles--;

	output_t output;
	output.channel0 = (float)input / 32767.0f * (float)adsrLevel / 32767.0f;
	output.channel1 = (float)input / 32767.0f * (float)adsrLevel / 32767.0f;

	return output;
}

template<typename T>
static T max(T a, T b) {
	return (a > b) ? a : b;
}

void spu_t::reverbClock(const output_t& input) {

#undef vLOUT
#undef vROUT
#undef mBASE
#undef dAPF1
#undef dAPF2
#undef vIIR
#undef vCOMB1
#undef vCOMB2
#undef vCOMB3
#undef vCOMB4
#undef vWALL
#undef vAPF1
#undef vAPF2
#undef mLSAME
#undef mRSAME
#undef mLCOMB1
#undef mRCOMB1
#undef mLCOMB2
#undef mRCOMB2
#undef dLSAME
#undef dRSAME
#undef mLDIFF
#undef mRDIFF
#undef mLCOMB3
#undef mRCOMB3
#undef mLCOMB4
#undef mRCOMB4
#undef dLDIFF
#undef dRDIFF
#undef mLAPF1
#undef mRAPF1
#undef mLAPF2
#undef mRAPF2
#undef vLIN
#undef vRIL

#define vLOUT (int32_t)reverbOutputVolumeLeftRight.hw.l
#define vROUT (int32_t)reverbOutputVolumeLeftRight.hw.h
#define mBASE dramReverbWorkAreaStartAddress.data
#define dAPF1 (int32_t)reverb[0x00]
#define dAPF2 (int32_t)reverb[0x01]
#define vIIR (int32_t)reverb[0x02]
#define vCOMB1 (int32_t)reverb[0x03]
#define vCOMB2 (int32_t)reverb[0x04]
#define vCOMB3 (int32_t)reverb[0x05]
#define vCOMB4 (int32_t)reverb[0x06]
#define vWALL (int32_t)reverb[0x07]
#define vAPF1 (int32_t)reverb[0x08]
#define vAPF2 (int32_t)reverb[0x09]
#define mLSAME reverb[0x0a]
#define mRSAME reverb[0x0b]
#define mLCOMB1 reverb[0x0c]
#define mRCOMB1 reverb[0x0d]
#define mLCOMB2 reverb[0x0e]
#define mRCOMB2 reverb[0x0f]
#define dLSAME reverb[0x10]
#define dRSAME reverb[0x11]
#define mLDIFF reverb[0x12]
#define mRDIFF reverb[0x13]
#define mLCOMB3 reverb[0x14]
#define mRCOMB3 reverb[0x15]
#define mLCOMB4 reverb[0x16]
#define mRCOMB4 reverb[0x17]
#define dLDIFF reverb[0x18]
#define dRDIFF reverb[0x19]
#define mLAPF1 reverb[0x1a]
#define mRAPF1 reverb[0x1b]
#define mLAPF2 reverb[0x1c]
#define mRAPF2 reverb[0x1d]
#define vLIN (int32_t)reverb[0x1e]
#define vRIN (int32_t)reverb[0x1f]

	auto sramRead = [&](int32_t addr) {

		addr = reverbCurrentBufferAddress + (addr << 3);

		if (addr > 0x7fffe)
			addr += -0x80000 + ((uint32_t)dramReverbWorkAreaStartAddress.data << 3);
		else if (addr < ((int32_t)dramReverbWorkAreaStartAddress.data << 3))
			addr += 0x80000 - ((uint32_t)dramReverbWorkAreaStartAddress.data << 3);

		return (int32_t)(int16_t)readDram16(addr);
		};

	auto sramWrite = [&](uint32_t addr, uint16_t data) {

		addr = reverbCurrentBufferAddress + (addr << 3);

		if (addr > 0x7fffe)
			addr += -0x80000 + ((uint32_t)dramReverbWorkAreaStartAddress.data << 3);
		else if (addr < ((int32_t)dramReverbWorkAreaStartAddress.data << 3))
			addr += 0x80000 - ((uint32_t)dramReverbWorkAreaStartAddress.data << 3);

		if (spuControlRegister.elem.reverbMasterEnable)
			writeDram16(addr, data);
		};

	if (bReverbCycle) { //left

		int32_t Lin = (vLIN * int16_t(input.channel0 * 32767.0f)) >> 15;

		sramWrite(mLSAME,
			saturate(((Lin + (sramRead(dLSAME) * vWALL >> 15) - sramRead(mLSAME - 2)) * vIIR >> 15) +
				sramRead(mLSAME - 2), -0x8000, 0x7fff));

		sramWrite(mLDIFF,
			saturate(((Lin + (sramRead(dRDIFF) * vWALL >> 15) - sramRead(mLDIFF - 2)) * vIIR >> 15) +
				sramRead(mLDIFF - 2), -0x8000, 0x7fff));

		int32_t Lout = (vCOMB1 * sramRead(mLCOMB1) >> 15) + (vCOMB2 * sramRead(mLCOMB2) >> 15) +
			(vCOMB3 * sramRead(mLCOMB3) >> 15) + (vCOMB4 * sramRead(mLCOMB4) >> 15);

		Lout = Lout - (vAPF1 * sramRead(mLAPF1 - dAPF1) >> 15);
		sramWrite(mLAPF1, saturate(Lout, -0x8000, 0x7fff));
		Lout = (vAPF1 * Lout >> 15) + sramRead(mLAPF1 - dAPF1);

		Lout = Lout - (vAPF2 * sramRead(mLAPF2 - dAPF2) >> 15);
		sramWrite(mLAPF2, saturate(Lout, -0x8000, 0x7fff));
		Lout = saturate((vAPF2 * Lout >> 15) + sramRead(mLAPF2 - dAPF2), -0x8000, 0x7fff);

		reverbOutput.channel0 = float(vLOUT * Lout >> 15) / 32767.0f;
	}
	else { //right

		int32_t Rin = (vRIN * int16_t(input.channel1 * 32767.0f)) >> 15;

		sramWrite(mRSAME,
			saturate(((Rin + (sramRead(dRSAME) * vWALL >> 15) - sramRead(mRSAME - 2)) * vIIR >> 15) +
				sramRead(mRSAME - 2), -0x8000, 0x7fff));

		sramWrite(mRDIFF,
			saturate(((Rin + (sramRead(dLDIFF) * vWALL >> 15) - sramRead(mRDIFF - 2)) * vIIR >> 15) +
				sramRead(mRDIFF - 2), -0x8000, 0x7fff));

		int32_t Rout = (vCOMB1 * sramRead(mRCOMB1) >> 15) + (vCOMB2 * sramRead(mRCOMB2) >> 15) +
			(vCOMB3 * sramRead(mRCOMB3) >> 15) + (vCOMB4 * sramRead(mRCOMB4) >> 15);

		Rout = Rout - (vAPF1 * sramRead(mRAPF1 - dAPF1) >> 15);
		sramWrite(mRAPF1, saturate(Rout, -0x8000, 0x7fff));
		Rout = (vAPF1 * Rout >> 15) + sramRead(mRAPF1 - dAPF1);

		Rout = Rout - (vAPF2 * sramRead(mRAPF2 - dAPF2) >> 15);
		sramWrite(mRAPF2, saturate(Rout, -0x8000, 0x7fff));
		Rout = saturate((vAPF2 * Rout >> 15) + sramRead(mRAPF2 - dAPF2), -0x8000, 0x7fff);

		reverbOutput.channel1 = float(vROUT * Rout >> 15) / 32767.0f;

		reverbCurrentBufferAddress = max(mBASE << 3, (reverbCurrentBufferAddress + 2) & 0x7fffe);
	}

	bReverbCycle != bReverbCycle;
}


output_t spu_t::clock() {

	output_t output = { 0.0f, 0.0f };
	output_t outputTempVoices = { 0.0f, 0.0f };
	output_t inputReverb = { 0.0f, 0.0f };


	//SPU Audio

	for (uint8_t voice = 0; voice < 24; voice++) {
		output_t outputVoice = voices.voiceRegisters[voice].envelope(voices.output(voice));

		if (!spuControlRegister.elem.unmuteSPU)
			outputVoice = { 0.0f, 0.0f };

		if (voice == 1) {
			writeDram16(0x800 + ((voice1MonoBufferCounter & 0x1ff) << 1), uint16_t(outputVoice.channel0 * 32767.0f));
			voice1MonoBufferCounter++;
		}
		else if (voice == 3) {
			writeDram16(0xc00 + ((voice3MonoBufferCounter & 0x1ff) << 1), uint16_t(outputVoice.channel0 * 32767.0f));
			voice3MonoBufferCounter++;
		}

		outputVoice.channel0 *= ((float)(int16_t(voices.voiceRegisters[voice].volumeLeftRight.hw.l << 1))) / 32767.0f;
		outputVoice.channel1 *= ((float)(int16_t(voices.voiceRegisters[voice].volumeLeftRight.hw.h << 1))) / 32767.0f;

		voices.voiceRegisters[voice].currentVolumeLeftRight.hw.l = uint16_t(outputVoice.channel0 * 32767.0f);//??
		voices.voiceRegisters[voice].currentVolumeLeftRight.hw.h = uint16_t(outputVoice.channel1 * 32767.0f);//??

		if (voices.channelReverbMode.data & (0x1 << voice)) {
			inputReverb = inputReverb + outputVoice;

			if (voices.channelOnOff.data & (0x1 << voice))
				voices.channelReverbMode.data &= ~(0x1 << voice);
		}

		outputTempVoices = outputTempVoices + outputVoice;
	}

	if (spuControlRegister.elem.spuEnable)
		output = output + outputTempVoices;


	//CD Drive Audio

		//nearest neighbor interpolation 
	counter37kHz += step44kHz;
	if (counter37kHz >= step37kHz) {
		counter37kHz -= step37kHz;

		outputCDXA = { 0.0f, 0.0f };
		if (pBus->pCdDrive->mode.elem.xa_adpcm && pBus->pCdDrive->bDataResponse)
			outputCDXA = clockXA_ADPCM();
	}

	output_t outputCDDA = { 0.0f, 0.0f };
	if (pBus->pCdDrive->bPlay)
		outputCDDA = clockCDDA();

	outputCD_DriveAudio = outputCDXA + outputCDDA;

	output_t outputCD_DriveAudioTemp;
	outputCD_DriveAudioTemp.channel0 = outputCD_DriveAudio.channel0 * pBus->pCdDrive->volumeLeftLeft / 128.0f
		+ outputCD_DriveAudio.channel1 * pBus->pCdDrive->volumeLeftRight / 128.0f;
	outputCD_DriveAudioTemp.channel1 = outputCD_DriveAudio.channel1 * pBus->pCdDrive->volumeRightRight / 128.0f
		+ outputCD_DriveAudio.channel0 * pBus->pCdDrive->volumeRightLeft / 128.0f;

	if (outputCD_DriveAudioTemp.channel0 > 1.0f)
		outputCD_DriveAudioTemp.channel0 = 1.0f;
	else if (outputCD_DriveAudioTemp.channel0 < -1.0f)
		outputCD_DriveAudioTemp.channel0 = -1.0f;

	if (outputCD_DriveAudioTemp.channel1 > 1.0f)
		outputCD_DriveAudioTemp.channel1 = 1.0f;
	else if (outputCD_DriveAudioTemp.channel1 < -1.0f)
		outputCD_DriveAudioTemp.channel1 = -1.0f;

	outputCD_DriveAudio = outputCD_DriveAudioTemp;

	if (pBus->pCdDrive->bMuted)
		outputCD_DriveAudio = { 0.0f, 0.0f };

	writeDram16(0 + ((cdAudioBufferCounter & 0x1ff) << 1), uint16_t(outputCD_DriveAudio.channel0 * 32767.0f));
	writeDram16(0x400 + ((cdAudioBufferCounter & 0x1ff) << 1), uint16_t(outputCD_DriveAudio.channel1 * 32767.0f));
	cdAudioBufferCounter++;

	outputCD_DriveAudio.channel0 *= (float)((int16_t)cdVolumeLeftRight.hw.l) / 32767.0f;
	outputCD_DriveAudio.channel1 *= (float)((int16_t)cdVolumeLeftRight.hw.h) / 32767.0f;

	if (spuControlRegister.elem.cdAudioEnable)
		output = output + outputCD_DriveAudio;

	if (spuControlRegister.elem.cdAudioReverb)
		inputReverb = inputReverb + outputCD_DriveAudio;


	//Reverb

	reverbClock(inputReverb);

	if (spuControlRegister.elem.reverbMasterEnable)
		output = output + reverbOutput;


	output.channel0 = output.channel0 * ((float)(int16_t(mainVolumeLeftRight.hw.l << 1))) / 32767.0f;
	output.channel1 = output.channel1 * ((float)(int16_t(mainVolumeLeftRight.hw.h << 1))) / 32767.0f;

	return output;
}

void spu_t::decode28NibblesXA_ADPCM(uint8_t* src, const uint8_t& block, const uint8_t& nibble,
	int16_t* dst, int16_t& old, int16_t& older, uint32_t& counter) {

	uint8_t valueShift = (src[4 + (block << 1) + nibble] & 0x0f);
	if (valueShift > 12)
		valueShift = 9;

	uint8_t shift = 12 - valueShift;

	int8_t filter = (src[4 + (block << 1) + nibble] & 0x30) >> 4;

	int32_t fo = pos_adpcm_table[filter];
	int32_t f1 = neg_adpcm_table[filter];

	for (uint8_t j = 0; j < 28; j++) {
		int16_t t = int16_t((((src[16 + block + j * 4]) >> (nibble * 4)) & 0x0f) << 12) >> 12;
		int16_t s = (t << shift) + ((old * fo + older * f1 + 32) / 64);

		if (s > 0x7fff) s = 0x7fff;
		else if (s < -0x8000) s = -0x8000;
		older = old;
		old = s;

		dst[counter] = s;
		counter++;
	}
}

void spu_t::decode28BytesXA_ADPCM(uint8_t* src, const uint8_t& block, const uint8_t& byte,
	int16_t* dst, int16_t& old, int16_t& older, uint32_t& counter) {

	uint8_t valueShift = (src[4 + (block << 1) + byte] & 0x0f);
	if (valueShift > 12)
		valueShift = 9;

	uint8_t shift = 12 - valueShift;

	uint8_t filter = (src[4 + (block << 1) + byte] & 0x30) >> 4;

	int32_t fo = pos_adpcm_table[filter];
	int32_t f1 = neg_adpcm_table[filter];

	for (uint8_t j = 0; j < 28; j++) {
		int32_t t = int16_t((int8_t)(src[16 + block + j * 4 + byte] & 0xff));
		int32_t s = (t << shift) + (((int32_t)old * fo + (int32_t)older * f1 + 32) / 64);

		if (s > 0x7fff) s = 0x7fff;
		else if (s < -0x8000) s = -0x8000;
		older = old;
		old = s;

		dst[counter] = (int16_t)s;
		counter++;
	}counter++;
}

void spu_t::decodeSectorXA_ADPCM(uint8_t*& src) {
	submode.data = src[0x10 + 0x02];
	codinginfo.data = src[0x10 + 0x03];
	bStereo = codinginfo.elem.monoStereo;
	bSampleRate18900Hz = codinginfo.elem.sampleRate;
	b8BitsPerSample = codinginfo.elem.bitsPerSample;

	src += 0x0c + 0x04 + 0x08;

	counterMono = 0;
	counterLeft = 0;
	counterRight = 0;
	if (!b8BitsPerSample)
		for (uint8_t i = 0; i < 0x12; i++) {
			for (uint8_t block = 0; block < 4; block++) {
				if (bStereo) {
					decode28NibblesXA_ADPCM(src, block, 0, samplesLeftXA, oldLeft, olderLeft, counterLeft);
					decode28NibblesXA_ADPCM(src, block, 1, samplesRightXA, oldRight, olderRight, counterRight);
				}
				else {
					decode28NibblesXA_ADPCM(src, block, 0, samplesMonoXA, oldMono, olderMono, counterMono);
					decode28NibblesXA_ADPCM(src, block, 1, samplesMonoXA, oldMono, olderMono, counterMono);
				}
			}
			src += 128;
		}
	else
		for (uint8_t i = 0; i < 0x12; i++) {
			for (uint8_t block = 0; block < 2; block++) {
				if (bStereo) {
					decode28BytesXA_ADPCM(src, block, 0, samplesLeftXA, oldLeft, olderLeft, counterLeft);
					decode28BytesXA_ADPCM(src, block, 1, samplesRightXA, oldRight, olderRight, counterRight);
				}
				else {
					decode28BytesXA_ADPCM(src, block, 0, samplesMonoXA, oldMono, olderMono, counterMono);
					decode28BytesXA_ADPCM(src, block, 1, samplesMonoXA, oldMono, olderMono, counterMono);
				}
			}
			src += 128;
		}
	src += 0x14 + 0x4;
}

output_t spu_t::clockXA_ADPCM() {
	output_t outputSample = OldOutputSampleXA;

	const uint16_t samplesPerSectorXA_ADPCM = 28 * 2 * 4 * 18;

	if (currentSample == samplesPerSectorXA_ADPCM / (b8BitsPerSample ? 2 : 1)) {
		////if (!submode.elem.endOfFile)//??
		////{
		//currentSample = 0;
		//pBus->pCdDrive->readSectorXA_ADPCM(ammXA, assXA, asectXA, pBufferXA_ADPCM);
		//decodeSectorXA_ADPCM(pBufferXA_ADPCM);
		////}
	}

	if (currentSample < samplesPerSectorXA_ADPCM / (b8BitsPerSample ? 2 : 1))
		if (bSampleRate18900Hz) {
			if (bCycle) {
				if (bStereo) {
					outputSample.channel0 = samplesLeftXA[currentSample >> 1] / 32767.0f;
					outputSample.channel1 = samplesRightXA[currentSample >> 1] / 32767.0f;
					currentSample += 2;
				}
				else {
					outputSample.channel0 = samplesMonoXA[currentSample] / 32767.0f;
					outputSample.channel1 = samplesMonoXA[currentSample] / 32767.0f;
					currentSample++;
				}
			}
		}
		else {
			if (bStereo) {
				outputSample.channel0 = samplesLeftXA[currentSample >> 1] / 32767.0f;
				outputSample.channel1 = samplesRightXA[currentSample >> 1] / 32767.0f;
				currentSample += 2;
			}
			else {
				outputSample.channel0 = samplesMonoXA[currentSample] / 32767.0f;
				outputSample.channel1 = samplesMonoXA[currentSample] / 32767.0f;
				currentSample++;
			}
		}

	bCycle = !bCycle;
	OldOutputSampleXA = outputSample;

	return outputSample;
}

output_t spu_t::clockCDDA() {

	output_t outputSample = { 0.0f, 0.0f };

	if (pBus->pCdDrive->bPlay) {
		if (counterCDDA == 0x930) {
			counterCDDA = 0;
			pBus->pCdDrive->readSectorCDDA(ammCDDA, assCDDA, asectCDDA, pBufferCDDA);
		}

		outputSample.channel0 = *((int16_t*)(pBufferCDDA + counterCDDA)) / 32767.0f;
		counterCDDA += 2;
		outputSample.channel1 = *((int16_t*)(pBufferCDDA + counterCDDA)) / 32767.0f;
		counterCDDA += 2;
	}

	return outputSample;
}