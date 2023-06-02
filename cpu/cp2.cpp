#include "cp2.h"
#include "cw33300.h"

extern bool g_emulationPaused;
extern bool g_GteInstructionsLogged;

cp2::cp2() {
	commandLookup.resize(0x40);
	commandLookup = {
		{"N/A  ",&cp2::XXXXX,0},{"RTPS ",&cp2::RTPS,15},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"NCLIP",&cp2::NCLIP,8},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"OP      ",&cp2::OP,6},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},

		{"DPCS  ",&cp2::DPCS,8},{"INTPL",&cp2::INTPL,8},{"MVMVA",&cp2::MVMVA,8},{"NCDS ",&cp2::NCDS,19},
		{"CDP   ",&cp2::CDP,13},{"N/A  ",&cp2::XXXXX,0},{"NCDT ",&cp2::NCDT,44},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"NCCS ",&cp2::NCCS,17},
		{"CC     ",&cp2::CC,11},{"N/A  ",&cp2::XXXXX,0},{"NCS   ",&cp2::NCS,14},{"N/A  ",&cp2::XXXXX,0},

		{"NCT   ",&cp2::NCT,30},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"SQR    ",&cp2::SQR,5},{"DCPL  ",&cp2::DCPL,8},{"DPCT ",&cp2::DPCT,17},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"AVSZ3",&cp2::AVSZ3,5},{"AVSZ4",&cp2::AVSZ4,6},{"N/A  ",&cp2::XXXXX,0},

		{"RTPT ",&cp2::RTPT,23},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},{"N/A  ",&cp2::XXXXX,0},
		{"N/A  ",&cp2::XXXXX,0},{"GPF    ",&cp2::GPF,5},{"GPF    ",&cp2::GPF,5},{"NCCT ",&cp2::NCCT,39},
	};

	unrTable.resize(101);
	unrTable = {
		0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,0xee,0xec,0xea,0xe8,0xe6,0xe4,0xe3,
		0xe1,0xdf,0xdd,0xdc,0xda,0xd8,0xd6,0xd5,0xd3,0xd1,0xd0,0xce,0xcd,0xcb,0xc9,0xc8,
		0xc6,0xc5,0xc3,0xc1,0xc0,0xbe,0xbd,0xbb,0xba,0xb8,0xb7,0xb5,0xb4,0xb2,0xb1,0xb0,
		0xae,0xad,0xab,0xaa,0xa9,0xa7,0xa6,0xa4,0xa3,0xa2,0xa0,0x9f,0x9e,0x9c,0x9b,0x9a,
		0x99,0x97,0x96,0x95,0x94,0x92,0x91,0x90,0x8f,0x8d,0x8c,0x8b,0x8a,0x89,0x87,0x86,
		0x85,0x84,0x83,0x82,0x81,0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x75,0x74,
		0x73,0x72,0x71,0x70,0x6f,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0x67,0x66,0x65,0x64,
		0x63,0x62,0x61,0x60,0x5f,0x5e,0x5d,0x5d,0x5c,0x5b,0x5a,0x59,0x58,0x57,0x56,0x55,
		0x54,0x53,0x53,0x52,0x51,0x50,0x4f,0x4e,0x4d,0x4d,0x4c,0x4b,0x4a,0x49,0x48,0x48,
		0x47,0x46,0x45,0x44,0x43,0x43,0x42,0x41,0x40,0x3f,0x3f,0x3e,0x3d,0x3c,0x3c,0x3b,
		0x3a,0x39,0x39,0x38,0x37,0x36,0x36,0x35,0x34,0x33,0x33,0x32,0x31,0x31,0x30,0x2f,
		0x2e,0x2e,0x2d,0x2c,0x2c,0x2b,0x2a,0x2a,0x29,0x28,0x28,0x27,0x26,0x26,0x25,0x24,
		0x24,0x23,0x22,0x22,0x21,0x20,0x20,0x1f,0x1e,0x1e,0x1d,0x1d,0x1c,0x1b,0x1b,0x1a,
		0x19,0x19,0x18,0x18,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x12,0x12,0x11,0x11,
		0x10,0x0f,0x0f,0x0e,0x0e,0x0d,0x0d,0x0c,0x0c,0x0b,0x0a,0x0a,0x09,0x09,0x08,0x08,
		0x07,0x07,0x06,0x06,0x05,0x05,0x04,0x04,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,
		0x00
	};
}
uint32_t cp2::getDat(uint8_t reg) {
	if ((reg == 1) || (reg == 3) || (reg == 5) ||
		(reg == 8) || (reg == 9) || (reg == 10) || (reg == 11))
		return (int32_t)(int16_t)cop2dat[reg];

	if (reg == 15)
		return cop2dat[14];

	return cop2dat[reg];
}

uint32_t cp2::getCnt(uint8_t reg) {
	if ((reg == 4) || (reg == 12) ||
		(reg == 20) ||
		(reg == 26) || (reg == 27) ||
		(reg == 29) || (reg == 30))
		return (int32_t)(int16_t)cop2cnt[reg];

	if (reg == 31)
		if ((flag.data & 0x7f87e000) > 0)
			return flag.data | 0x80000000;
		else
			return flag.data;

	return cop2cnt[reg];
}

void cp2::setDat(uint8_t reg, uint32_t value) {
	if ((reg == 1) || (reg == 3) || (reg == 5) ||
		(reg == 7) || (reg == 8) ||
		(reg == 16) || (reg == 17) || (reg == 18) || (reg == 19)) {
		cop2dat[reg] = value & 0x0000ffff;
		return;
	}

	if (reg == 15) {
		cop2dat[12] = cop2dat[13];
		cop2dat[13] = cop2dat[14];
		cop2dat[14] = value;
		return;
	}

	if (reg == 28) {
		cop2dat[reg] = value & 0x00007fff;
		cop2dat[9] = (value & 0x0000001f) * 0x80;
		cop2dat[10] = ((value >> 5) & 0x0000001f) * 0x80;
		cop2dat[11] = ((value >> 10) & 0x0000001f) * 0x80;

		cop2dat[29] =
			(((cop2dat[9] / 0x80) > 0x1f) ? 0x1f : cop2dat[9] / 0x80) |
			((((cop2dat[10] / 0x80) > 0x1f) ? 0x1f : cop2dat[10] / 0x80) << 5) |
			((((cop2dat[11] / 0x80) > 0x1f) ? 0x1f : cop2dat[11] / 0x80) << 10);
		cop2dat[28] = cop2dat[29];
		return;
	}

	if ((reg == 9) || (reg == 10) || (reg == 11)) {
		cop2dat[reg] = value & 0x0000ffff;

		cop2dat[29] =
			(((cop2dat[9] / 0x80) > 0x1f) ? 0x1f : cop2dat[9] / 0x80) |
			((((cop2dat[10] / 0x80) > 0x1f) ? 0x1f : cop2dat[10] / 0x80) << 5) |
			((((cop2dat[11] / 0x80) > 0x1f) ? 0x1f : cop2dat[11] / 0x80) << 10);
		cop2dat[28] = cop2dat[29];
		return;
	}

	if (reg == 30) {
		cop2dat[reg] = value;
		uint32_t msb = 1 << 31;
		for (uint32_t i = 0; i <= 31; i++) {
			if (((value & 0x80000000) == 0) && (value & msb)) {
				cop2dat[31] = i;
				return;
			}

			if ((value & 0x80000000) && ((value & msb) == 0)) {
				cop2dat[31] = i;
				return;
			}

			msb >>= 1;
		}
		cop2dat[31] = 32;
		return;
	}

	if ((reg != 29) && (reg != 31))
		cop2dat[reg] = value;
}

void cp2::setCnt(uint8_t reg, uint32_t value) {
	if ((reg == 4) || (reg == 12) ||
		(reg == 20) ||
		(reg == 26) || (reg == 27) ||
		(reg == 29) || (reg == 30)) {
		cop2cnt[reg] = value & 0x0000ffff;
		return;
	}

	if (reg == 31) {
		flag.data = value & 0x7ffff000;
		return;
	}

	cop2cnt[reg] = value;
}

void cp2::MFC2(uint8_t rt, uint8_t rd) {
	pCpu->set(rt, getDat(rd));

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction MFC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "dat " << std::dec << (uint16_t)rd << std::hex << std::endl;
	}
}

void cp2::CFC2(uint8_t rt, uint8_t rd) {
	pCpu->set(rt, getCnt(rd));

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction CFC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "cnt " << std::dec << (uint16_t)rd << std::hex << std::endl;
	}
}

void cp2::MTC2(uint8_t rt, uint8_t rd) {
	setDat(rd, pCpu->get(rt));

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction MTC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "dat " << std::dec << (uint16_t)rd << std::hex << std::endl;
	}
}

void cp2::CTC2(uint8_t rt, uint8_t rd) {
	setCnt(rd, pCpu->get(rt));

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction CTC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "cnt " << std::dec << (uint16_t)rd << std::hex << std::endl;
	}
}

void cp2::BC2F(uint16_t imm) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2F 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp2::BC2T(uint16_t imm) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2T 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp2::COP2(uint32_t imm) {
	flag.data = 0;
	(this->*commandLookup[imm & 0x3f].command)(imm);
	pCpu->gteClocks = commandLookup[imm & 0x3f].clocks;

	if (g_GteInstructionsLogged)
		std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[imm & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
}

void cp2::LWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	uint32_t data;
	uint8_t cycles;
	pCpu->read32(pCpu->get(rs) + (int32_t)imm, data, cycles);
	setDat(rt, data);

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction LWC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "dat " << std::dec << (uint16_t)rt << std::hex << std::endl;
	}
}

void cp2::SWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	uint32_t data = getDat(rt);
	uint8_t cycles;
	pCpu->write32(pCpu->get(rs) + (int32_t)imm, data, cycles);

	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction SWC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		std::cout << "dat " << std::dec << (uint16_t)rt << std::hex << std::endl;
	}
}

#define getVX0 ((int32_t)(int16_t)(cop2dat[0] & 0x0000ffff))
#define getVY0 ((int32_t)(int16_t)(cop2dat[0] >> 16))
#define getVZ0 ((int32_t)(int16_t)(cop2dat[1] & 0x0000ffff))

#define getVX1 ((int32_t)(int16_t)(cop2dat[2] & 0x0000ffff))
#define getVY1 ((int32_t)(int16_t)(cop2dat[2] >> 16))
#define getVZ1 ((int32_t)(int16_t)(cop2dat[3] & 0x0000ffff))

#define getVX2 ((int32_t)(int16_t)(cop2dat[4] & 0x0000ffff))
#define getVY2 ((int32_t)(int16_t)(cop2dat[4] >> 16))
#define getVZ2 ((int32_t)(int16_t)(cop2dat[5] & 0x0000ffff))

#define setOTZ(value) cop2dat[7] = value

#define getIR0 ((int32_t)(int16_t)cop2dat[8])
#define getIR1 ((int32_t)(int16_t)cop2dat[9])
#define getIR2 ((int32_t)(int16_t)cop2dat[10])
#define getIR3 ((int32_t)(int16_t)cop2dat[11])

#define setIR0(value) cop2dat[8] = value
#define setIR1(value) setDat(9,value)
#define setIR2(value) setDat(10,value)
#define setIR3(value) setDat(11,value)

#define getSX0 ((int32_t)(int16_t)(cop2dat[12] & 0x0000ffff))
#define getSX1 ((int32_t)(int16_t)(cop2dat[13] & 0x0000ffff))
#define getSX2 ((int32_t)(int16_t)(cop2dat[14] & 0x0000ffff))

#define setSX2(sx) \
cop2dat[12] = (cop2dat[12] & 0xffff0000) | (cop2dat[13] & 0x0000ffff);\
cop2dat[13] = (cop2dat[13] & 0xffff0000) | (cop2dat[14] & 0x0000ffff);\
cop2dat[14] = (cop2dat[14] & 0xffff0000) | ((int16_t)sx)

#define getSY0 ((int32_t)(int16_t)((cop2dat[12] & 0xffff0000) >> 16))
#define getSY1 ((int32_t)(int16_t)((cop2dat[13] & 0xffff0000) >> 16))
#define getSY2 ((int32_t)(int16_t)((cop2dat[14] & 0xffff0000) >> 16))

#define setSY2(sy) \
cop2dat[12] = (cop2dat[12] & 0x0000ffff) | (cop2dat[13] & 0xffff0000);\
cop2dat[13] = (cop2dat[13] & 0x0000ffff) | (cop2dat[14] & 0xffff0000);\
cop2dat[14] = (cop2dat[14] & 0x0000ffff) | (((int16_t)sy) << 16)

#define setSZ3(sz) \
cop2dat[16] = cop2dat[17];\
cop2dat[17] = cop2dat[18];\
cop2dat[18] = cop2dat[19];\
cop2dat[19] = (uint16_t)(sz)

#define getSZ0 cop2dat[16]
#define getSZ1 cop2dat[17]
#define getSZ2 cop2dat[18]
#define getSZ3 cop2dat[19]

#define getMAC0 ((int32_t)(cop2dat[24]))
#define getMAC1 ((int32_t)(cop2dat[25]))
#define getMAC2 ((int32_t)(cop2dat[26]))
#define getMAC3 ((int32_t)(cop2dat[27]))

#define setMAC0(x) cop2dat[24] = x
#define setMAC1(x) cop2dat[25] = x
#define setMAC2(x) cop2dat[26] = x
#define setMAC3(x) cop2dat[27] = x

#define getRT11 ((int32_t)(int16_t)(cop2cnt[0] & 0x0000ffff))
#define getRT12 ((int32_t)(int16_t)(cop2cnt[0] >> 16))
#define getRT13 ((int32_t)(int16_t)(cop2cnt[1] & 0x0000ffff))
#define getRT21 ((int32_t)(int16_t)(cop2cnt[1] >> 16))
#define getRT22 ((int32_t)(int16_t)(cop2cnt[2] & 0x0000ffff))
#define getRT23 ((int32_t)(int16_t)(cop2cnt[2] >> 16))
#define getRT31 ((int32_t)(int16_t)(cop2cnt[3] & 0x0000ffff))
#define getRT32 ((int32_t)(int16_t)(cop2cnt[3] >> 16))
#define getRT33 ((int32_t)(int16_t)(cop2cnt[4] & 0x0000ffff))

#define getTRX ((int32_t)(cop2cnt[5]))
#define getTRY ((int32_t)(cop2cnt[6]))
#define getTRZ ((int32_t)(cop2cnt[7]))

#define getOFX ((int32_t)(cop2cnt[24]))
#define getOFY ((int32_t)(cop2cnt[25]))
#define getH ((uint16_t)cop2cnt[26])

#define getDQA ((int32_t)(int16_t)cop2cnt[27])
#define getDQB ((int32_t)cop2cnt[28])

#define getZSF3 ((int32_t)(int16_t)(cop2cnt[29]))
#define getZSF4 ((int32_t)(int16_t)(cop2cnt[30]))

int32_t cp2::devide() {
	int32_t n;
	if (getSZ3 == 0) {
		flag.elem.divOV = 1;
		return 0x1ffff;
	}

	if (getH < getSZ3 * 2) {
		uint32_t z = 32;
		uint32_t msb = 1 << 31;
		for (uint32_t i = 0; i <= 31; i++) {
			if ((getSZ3 & 0x80000000) && ((getSZ3 & msb) == 0)) {
				z = i;
				break;
			}
			msb >>= 1;
		}

		n = getH << z;
		uint32_t d = getSZ3 << z;
		uint32_t u = 0;
		if (((d - 0x7fc0) >> 7) <= 0x256)
			u = unrTable[(d - 0x7fc0) >> 7] + 0x101;
		else
			u = 0x10;
		d = ((0x2000080 - (d * u)) >> 8);
		d = ((0x0000080 + (d * u)) >> 8);
		n = std::min(0x1ffff, (int32_t)(((n * d) + 0x8000) >> 16));
	}
	else {
		n = 0x1ffff;
	}

	if (n > 0x1ffff) {
		flag.elem.divOV = 1;
		return 0x1ffff;
	}

	return n;
}

uint8_t cp2::XXXXX(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::RTPS(uint32_t commandData) {
	commandEncoding_t command;
	command.data = commandData;

	int32_t sf = command.sf;
	int32_t lm = command.lm;

	setMAC1((getTRX * 0x1000 + getRT11 * getVX0
		+ getRT12 * getVY0 + getRT13 * getVZ0) >> (sf * 12));

	setMAC2((getTRY * 0x1000 + getRT21 * getVX0
		+ getRT22 * getVY0 + getRT23 * getVZ0) >> (sf * 12));

	setMAC3((getTRZ * 0x1000 + getRT31 * getVX0
		+ getRT32 * getVY0 + getRT33 * getVZ0) >> (sf * 12));

	if (lm) {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < 0) {
			setIR1(0);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < 0) {
			setIR2(0);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < 0) {
			setIR3(0);
		}
		else
			setIR3(getMAC3);
	}
	else {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < -0x8000) {
			setIR1(-0x8000);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < -0x8000) {
			setIR2(-0x8000);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < -0x8000) {
			setIR3(-0x8000);
		}
		else
			setIR3(getMAC3);
	}

	if ((getMAC3 >> ((1 - sf) * 12)) > 0xffff) {
		setSZ3(0xffff);
		flag.elem.sz3_otcSat = 1;
	}
	else if ((getMAC3 >> ((1 - sf) * 12)) < 0) {
		setSZ3(0);
		flag.elem.sz3_otcSat = 1;
	}
	else {
		setSZ3((getMAC3 >> ((1 - sf) * 12)));
	}


	setMAC0(devide() * getIR1 + getOFX);//
	if ((int16_t)(getMAC0 / 0x10000) > 0x3ff) {
		setSX2(0x3ff);
		flag.elem.sx2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < -0x400) {
		setSX2(0xdead);
		flag.elem.sx2Sat = 1;
	}
	else {
		setSX2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getIR2 + getOFY);//
	if ((getMAC0 / 0x10000) > 0x3ff) {
		setSY2(0x3ff);
		flag.elem.sy2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < (int16_t)-0x400) {
		setSY2(0xdead);
		flag.elem.sy2Sat = 1;
	}
	else {
		setSY2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getDQA + getDQB);
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setIR0(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setIR0(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setIR0(getMAC0 / 0x1000);
	}

	//flag.elem.mac0ResultNegOV = 1; //
	//flag.elem.mac0ResultPosOV = 1; //

	return 0;
}

uint8_t cp2::NCLIP(uint32_t command) {
	setMAC0(getSX0 * (getSY1 - getSY2) +
		getSX1 * (getSY2 - getSY0) +
		getSX2 * (getSY0 - getSY1));
	return 0;
}

uint8_t cp2::OP(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DPCS(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::INTPL(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::MVMVA(uint32_t commandData) {
	commandEncoding_t command;
	command.data = commandData;

	int32_t sf = command.sf;
	int32_t lm = command.lm;

	setMAC1((getRT11 * getVX0 + getRT12 * getVY0 + getRT13 * getVZ0) >> (sf * 12));
	setMAC2((getRT21 * getVX0 + getRT22 * getVY0 + getRT23 * getVZ0) >> (sf * 12));
	setMAC3((getRT31 * getVX0 + getRT32 * getVY0 + getRT33 * getVZ0) >> (sf * 12));

	if (lm) {
		if (getMAC1 > 0x7fff)
			setIR1(0x7fff);
		else if (getMAC1 < 0)
			setIR1(0);
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff)
			setIR2(0x7fff);
		else if (getMAC2 < 0)
			setIR2(0);
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff)
			setIR3(0x7fff);
		else if (getMAC3 < 0)
			setIR3(0);
		else
			setIR3(getMAC3);
	}
	else {
		if (getMAC1 > 0x7fff)
			setIR1(0x7fff);
		else if (getMAC1 < -0x8000)
			setIR1(-0x8000);
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff)
			setIR2(0x7fff);
		else if (getMAC2 < -0x8000)
			setIR2(-0x8000);
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff)
			setIR3(0x7fff);
		else if (getMAC3 < -0x8000)
			setIR3(-0x8000);
		else
			setIR3(getMAC3);
	}

	return 0;
}

uint8_t cp2::NCDS(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::CDP(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCDT(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCCS(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::CC(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCS(uint32_t command) {
	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCT(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::SQR(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DCPL(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DPCT(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::AVSZ3(uint32_t command) {
	setMAC0(getZSF3 * (getSZ1 + getSZ2 + getSZ3));
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setOTZ(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setOTZ(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setOTZ(getMAC0 / 0x1000);
	}
	return 0;
}

uint8_t cp2::AVSZ4(uint32_t command) {
	setMAC0(getZSF4 * (getSZ0 + getSZ1 + getSZ2 + getSZ3));
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setOTZ(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setOTZ(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setOTZ(getMAC0 / 0x1000);
	}
	return 0;
}

uint8_t cp2::RTPT(uint32_t commandData) {
	commandEncoding_t command;
	command.data = commandData;

	int32_t sf = command.sf;
	int32_t lm = command.lm;

	setMAC1((getTRX * 0x1000 + getRT11 * getVX0
		+ getRT12 * getVY0 + getRT13 * getVZ0) >> (sf * 12));

	setMAC2((getTRY * 0x1000 + getRT21 * getVX0
		+ getRT22 * getVY0 + getRT23 * getVZ0) >> (sf * 12));

	setMAC3((getTRZ * 0x1000 + getRT31 * getVX0
		+ getRT32 * getVY0 + getRT33 * getVZ0) >> (sf * 12));

	if (lm) {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < 0) {
			setIR1(0);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < 0) {
			setIR2(0);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < 0) {
			setIR3(0);
		}
		else
			setIR3(getMAC3);
	}
	else {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < -0x8000) {
			setIR1(-0x8000);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < -0x8000) {
			setIR2(-0x8000);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < -0x8000) {
			setIR3(-0x8000);
		}
		else
			setIR3(getMAC3);
	}

	if ((getMAC3 >> ((1 - sf) * 12)) > 0xffff) {
		setSZ3(0xffff);
		flag.elem.sz3_otcSat = 1;
	}
	else if ((getMAC3 >> ((1 - sf) * 12)) < 0) {
		setSZ3(0);
		flag.elem.sz3_otcSat = 1;
	}
	else {
		setSZ3((getMAC3 >> ((1 - sf) * 12)));
	}


	setMAC0(devide() * getIR1 + getOFX);//
	if ((int16_t)(getMAC0 / 0x10000) > 0x3ff) {
		setSX2(0x3ff);
		flag.elem.sx2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < -0x400) {
		setSX2(0xdead);
		flag.elem.sx2Sat = 1;
	}
	else {
		setSX2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getIR2 + getOFY);//
	if ((getMAC0 / 0x10000) > 0x3ff) {
		setSY2(0x3ff);
		flag.elem.sy2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < (int16_t)-0x400) {
		setSY2(0xdead);
		flag.elem.sy2Sat = 1;
	}
	else {
		setSY2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getDQA + getDQB);
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setIR0(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setIR0(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setIR0(getMAC0 / 0x1000);
	}

	setMAC1((getTRX * 0x1000 + getRT11 * getVX1
		+ getRT12 * getVY0 + getRT13 * getVZ0) >> (sf * 12));

	setMAC2((getTRY * 0x1000 + getRT21 * getVX1
		+ getRT22 * getVY0 + getRT23 * getVZ0) >> (sf * 12));

	setMAC3((getTRZ * 0x1000 + getRT31 * getVX1
		+ getRT32 * getVY0 + getRT33 * getVZ0) >> (sf * 12));

	if (lm) {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < 0) {
			setIR1(0);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < 0) {
			setIR2(0);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < 0) {
			setIR3(0);
		}
		else
			setIR3(getMAC3);
	}
	else {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < -0x8000) {
			setIR1(-0x8000);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < -0x8000) {
			setIR2(-0x8000);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < -0x8000) {
			setIR3(-0x8000);
		}
		else
			setIR3(getMAC3);
	}

	if ((getMAC3 >> ((1 - sf) * 12)) > 0xffff) {
		setSZ3(0xffff);
		flag.elem.sz3_otcSat = 1;
	}
	else if ((getMAC3 >> ((1 - sf) * 12)) < 0) {
		setSZ3(0);
		flag.elem.sz3_otcSat = 1;
	}
	else {
		setSZ3((getMAC3 >> ((1 - sf) * 12)));
	}


	setMAC0(devide() * getIR1 + getOFX);//
	if ((int16_t)(getMAC0 / 0x10000) > 0x3ff) {
		setSX2(0x3ff);
		flag.elem.sx2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < -0x400) {
		setSX2(0xdead);
		flag.elem.sx2Sat = 1;
	}
	else {
		setSX2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getIR2 + getOFY);//
	if ((getMAC0 / 0x10000) > 0x3ff) {
		setSY2(0x3ff);
		flag.elem.sy2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < (int16_t)-0x400) {
		setSY2(0xdead);
		flag.elem.sy2Sat = 1;
	}
	else {
		setSY2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getDQA + getDQB);
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setIR0(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setIR0(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setIR0(getMAC0 / 0x1000);
	}

	setMAC1((getTRX * 0x1000 + getRT11 * getVX2
		+ getRT12 * getVY0 + getRT13 * getVZ0) >> (sf * 12));

	setMAC2((getTRY * 0x1000 + getRT21 * getVX2
		+ getRT22 * getVY0 + getRT23 * getVZ0) >> (sf * 12));

	setMAC3((getTRZ * 0x1000 + getRT31 * getVX2
		+ getRT32 * getVY0 + getRT33 * getVZ0) >> (sf * 12));

	if (lm) {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < 0) {
			setIR1(0);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < 0) {
			setIR2(0);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < 0) {
			setIR3(0);
		}
		else
			setIR3(getMAC3);
	}
	else {
		if (getMAC1 > 0x7fff) {
			setIR1(0x7fff);
			flag.elem.ir1Sat = 1;
		}
		else if (getMAC1 < -0x8000) {
			setIR1(-0x8000);
			flag.elem.ir1Sat = 1;
		}
		else
			setIR1(getMAC1);

		if (getMAC2 > 0x7fff) {
			setIR2(0x7fff);
			flag.elem.ir2Sat = 1;
		}
		else if (getMAC2 < -0x8000) {
			setIR2(-0x8000);
			flag.elem.ir2Sat = 1;
		}
		else
			setIR2(getMAC2);

		if (getMAC3 > 0x7fff) {
			setIR3(0x7fff);
		}
		else if (getMAC3 < -0x8000) {
			setIR3(-0x8000);
		}
		else
			setIR3(getMAC3);
	}

	if ((getMAC3 >> ((1 - sf) * 12)) > 0xffff) {
		setSZ3(0xffff);
		flag.elem.sz3_otcSat = 1;
	}
	else if ((getMAC3 >> ((1 - sf) * 12)) < 0) {
		setSZ3(0);
		flag.elem.sz3_otcSat = 1;
	}
	else {
		setSZ3((getMAC3 >> ((1 - sf) * 12)));
	}


	setMAC0(devide() * getIR1 + getOFX);//
	if ((int16_t)(getMAC0 / 0x10000) > 0x3ff) {
		setSX2(0x3ff);
		flag.elem.sx2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < -0x400) {
		setSX2(0xdead);
		flag.elem.sx2Sat = 1;
	}
	else {
		setSX2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getIR2 + getOFY);//
	if ((getMAC0 / 0x10000) > 0x3ff) {
		setSY2(0x3ff);
		flag.elem.sy2Sat = 1;
	}
	else if ((int16_t)(getMAC0 / 0x10000) < (int16_t)-0x400) {
		setSY2(0xdead);
		flag.elem.sy2Sat = 1;
	}
	else {
		setSY2(getMAC0 / 0x10000);
	}

	setMAC0(devide() * getDQA + getDQB);
	if ((int16_t)(getMAC0 / 0x1000) < 0) {
		setIR0(0);
		flag.elem.ir0Sat = 1;
	}
	else if ((getMAC0 / 0x1000) > 0x1000) {
		setIR0(0x1000);
		flag.elem.ir0Sat = 1;
	}
	else {
		setIR0(getMAC0 / 0x1000);
	}

	return 0;
}

uint8_t cp2::GPF(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::GPL(uint32_t command) {

	//std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
	//	<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	//g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCCT(uint32_t command) {

	/*std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;*/
		//g_emulationPaused = true;
	return 0;
}