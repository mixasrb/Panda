#include "cp0.h"
#include "cw33300.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

uint32_t cp0::get(const uint8_t& reg) {
	return cop0r[reg];
}

void cp0::set(const uint8_t& reg, const uint32_t& value) {
	if (reg == 13) {
		CAUSE &= 0xffffcff;
		CAUSE |= (value & !0xffffcff);
	}
	else
		cop0r[reg] = value;

	if (reg == 12)
		isolateDataCache = (SR & 0x10000) == 0x10000;

	if (SR & 0x400000) {
		std::cout << "[CP0] EMULATION PAUSED! unhandled BEV\n";
		g_emulationPaused = true;
	}
}

void cp0::MFC0(const uint8_t& rt, const uint8_t& rd) {
	pCpu->set(rt, get(rd));
}

void cp0::CFC0(const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0::MTC0(const uint8_t& rt, const uint8_t& rd) {
	set(rd, pCpu->get(rt));
}

void cp0::CTC0(const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0::BC0F(uint16_t imm) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0::COP0(uint32_t imm) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0::RFE() {
	SR = (SR & 0xfffffff0) | ((SR & 0x3c) >> 2);
}

void cp0::exceptionHandler(const excode_t& code, const uint32_t addrRef) {

	if ((code == _ADEL) || (code == _ADES)) {
		std::cout << "[CP0] EMULATION PAUSED! triggered exception code: 0x" << std::hex << (uint16_t)code << std::endl;
		//g_emulationPaused = true;
		return;
	}
	else if ((code == _BP) || (code == _OV)) {
		std::cout << "[CP0] EMULATION PAUSED! triggered exception code: 0x" << std::hex << (uint16_t)code << std::endl;
		//g_emulationPaused = true;
		return;
	}

	SR = (SR & 0xffffffc0) | ((SR & 0xf) << 2);
	//setting cause of exception
	CAUSE = CAUSE >> 7 << 7;
	CAUSE |= code << 2;

	if ((code == _ADEL) || (code == _ADES) || (code == _SYSCALL) || (code == _BP) || (code == _OV)) {
		if ((code == _ADEL) || (code == _ADES)) {
			BAD_V_ADDR = addrRef;
		}

		EPC = pCpu->pc - 8;
	}

	if (code == _INT)
		if (pCpu->isCurrentOpcodeBranch) {
			EPC = pCpu->opcode_pc;
			CAUSE |= 0x80000000;
		}
		else
			EPC = pCpu->pc - 4;

	pCpu->isPipelineFull = false;
	pCpu->pc = 0x80000080;
}

void cp0::interruptHandler(const uint32_t& irq) {
	pCpu->pBus->interruptStat |= irq;
}

void cp0::checkForInterrupts() {
	if (pCpu->pBus->interruptMask & pCpu->pBus->interruptStat)
		CAUSE |= 0x400;
	else
		CAUSE &= 0xfffffbff;

	const bool executeInterrupt = (SR & 0x1) && (SR & 0x400) && (CAUSE & 0x400);

	if (executeInterrupt)
		exceptionHandler(_INT);
}
