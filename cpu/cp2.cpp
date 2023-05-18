#include "cp2.h"
#include "cw33300.h"

extern bool g_emulationPaused;
extern bool g_GteInstructionsLogged;

cp2::cp2() {
	commandLookup.resize(40);
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
}

void cp2::setDat(uint8_t reg, uint32_t value) {
	cop2dat[reg] = value;
}

void cp2::setCnt(uint8_t reg, uint32_t value) {
	cop2cnt[reg] = value;
}

void cp2::MFC2(uint8_t rt, uint8_t rd) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction MFC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

void cp2::CFC2(uint8_t rt, uint8_t rd) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction CFC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

void cp2::MTC2(uint8_t rt, uint8_t rd) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction MTC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

void cp2::CTC2(uint8_t rt, uint8_t rd) {
	setCnt(rd, pCpu->get(rt));
}

void cp2::BC2F(uint16_t imm) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2F 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

void cp2::BC2T(uint16_t imm) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2T 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

void cp2::COP2(uint32_t imm) {
	(this->*commandLookup[imm & 0x3f].command)(imm & 0x3f);
	pCpu->gteClocks = commandLookup[imm].clocks;
}

void cp2::LWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	uint32_t data;
	uint8_t cycles;
	pCpu->read32(pCpu->get(rs) + (int32_t)imm, data, cycles);
	setDat(rt, data);
}

void cp2::SWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	if (g_GteInstructionsLogged) {
		std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction SWC2 0x" << pCpu->opcode
			<< " pc 0x" << pCpu->pc - 4 << std::endl;
		g_emulationPaused = true;
	}
}

uint8_t cp2::XXXXX(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::RTPS(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCLIP(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::OP(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DPCS(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::INTPL(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::MVMVA(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCDS(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::CDP(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCDT(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCCS(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::CC(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCS(uint32_t command) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCT(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::SQR(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DCPL(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::DPCT(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::AVSZ3(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::AVSZ4(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::RTPT(uint32_t command) {

	///?!!!!
	cop2dat[25] = (cop2cnt[5] * 0x1000 +
		((cop2cnt[0] & 0x0000ffff) * (cop2dat[0] & 0x0000ffff) >> 12) +
		(((cop2cnt[0] & 0xffff0000) >> 16) * ((cop2dat[0] & 0xffff0000) >> 16) >> 12) +
		((cop2cnt[1] & 0x0000ffff) * (cop2dat[1] & 0x0000ffff) >> 12))
		>> (((commandEncoding_t)command).sf * 12);
	cop2dat[9] = (cop2dat[9] & 0xffff0000) | (cop2dat[25] & 0x0000ffff);

	cop2dat[26] = (cop2cnt[6] * 0x1000 +
		((cop2cnt[2] & 0x0000ffff) * (cop2dat[0] & 0x0000ffff) >> 12) +
		(((cop2cnt[2] & 0xffff0000) >> 16) * ((cop2dat[0] & 0xffff0000) >> 16) >> 12) +
		((cop2cnt[3] & 0x0000ffff) * (cop2dat[1] & 0x0000ffff) >> 12))
		>> (((commandEncoding_t)command).sf * 12);
	cop2dat[10] = (cop2dat[10] & 0xffff0000) | (cop2dat[26] & 0x0000ffff);

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::GPF(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::GPL(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t cp2::NCCT(uint32_t command) {

	std::cout << "[CP2] EMULATION PAUSED! unhandled command " << commandLookup[command & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
	return 0;
}