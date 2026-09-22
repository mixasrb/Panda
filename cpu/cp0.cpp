#include "cp0.h"
#include "cw33300.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

uint32_t cp0_t::get(const uint8_t& reg) {
	return cop0r[reg];
}

void cp0_t::set(const uint8_t& reg, const uint32_t& value) {
	if (reg == 13) {
		CAUSE &= 0xffffcff;
		CAUSE |= (value & !0xffffcff);
	}
	else
		cop0r[reg] = value;

	if (reg == 12)
		isolateDataCache = ((SR & 0x10000) == 0x10000);

	if (SR & 0x400000) {
		std::cout << "[CP0] EMULATION PAUSED! unhandled BEV\n";
		g_emulationPaused = true;
	}

	executeInterrupt = (SR & 0x1) && (SR & 0x400) && (CAUSE & 0x400);

	//irqFlag = true;
	//uint8_t clocks;
	//uint32_t opcode;

	///*pCpu->pBus->cpuRead32(pCpu->pc - 1 * 4, pCpu->opcode, clocks, true);
	//irqFlag &= !pCpu->checkForCopOpcode(opcode);

	//pCpu->pBus->cpuRead32(pCpu->pc + 1 * 4, opcode, clocks, true);
	//irqFlag &= !pCpu->checkForCopOpcode(opcode);*/

	//if (irqFlag/* && !pCpu->isCurrentOpcodeBranch*/ && executeInterrupt) {
	//	executeInterrupt = false;
	//	exceptionHandler(_INT);
	//}
}

void cp0_t::MFC0(const uint8_t& rt, const uint8_t& rd) {
	pCpu->set(rt, get(rd));
}

void cp0_t::CFC0(const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0_t::MTC0(const uint8_t& rt, const uint8_t& rd) {
	set(rd, pCpu->get(rt));
}

void cp0_t::CTC0(const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0_t::BC0F(uint16_t imm) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0_t::COP0(uint32_t imm) {
	std::cout << "[CP0] EMULATION PAUSED! unhandled CP0 instruction 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << std::endl;
	g_emulationPaused = true;
}

void cp0_t::RFE() {
	SR = (SR & 0xfffffff0) | ((SR & 0x3c) >> 2);
	executeInterrupt = (SR & 0x1) && (SR & 0x400) && (CAUSE & 0x400);

	/*if (pCpu->cycle0.bWrite == true) {
		pCpu->cycle0.bWrite = false;
		if (pCpu->r[pCpu->cycle0.addr] == pCpu->cycle0.old)
			pCpu->r[pCpu->cycle0.addr] = pCpu->cycle0.data;
	}
	if (pCpu->cycle1.bWrite == true) {
		pCpu->cycle1.bWrite = false;
		if (pCpu->r[pCpu->cycle1.addr] == pCpu->cycle1.old)
			pCpu->r[pCpu->cycle1.addr] = pCpu->cycle1.data;;
	}*/
}

void cp0_t::exceptionHandler(const excode_t& code, const uint32_t addrRef) {

	/*if (pCpu->cycle0.bWrite == true) {
		pCpu->cycle0.bWrite = false;
		if (pCpu->r[pCpu->cycle0.addr] == pCpu->cycle0.old)
			pCpu->r[pCpu->cycle0.addr] = pCpu->cycle0.data;
	}
	if (pCpu->cycle1.bWrite == true) {
		pCpu->cycle1.bWrite = false;
		if (pCpu->r[pCpu->cycle1.addr] == pCpu->cycle1.old)
			pCpu->r[pCpu->cycle1.addr] = pCpu->cycle1.data;;
	}*/


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
			CAUSE |= 0x80000000; //BD
			//??
			//gte in branch delays!!
		}
		else
			//if (!pCpu->gteCommand)
			EPC = pCpu->pc - 4;


	pCpu->pc = 0x80000080;

	//empty pipeline 
	pCpu->opcodes[0] = 0;
	pCpu->opcodes[1] = 0;
}

void cp0_t::updateInterruptMaskStat() {
	if (pCpu->pBus->interruptMask & pCpu->pBus->interruptStat)
		CAUSE |= 0x400;
	else
		CAUSE &= 0xfffffbff;

	executeInterrupt = (SR & 0x1) && (SR & 0x400) && (CAUSE & 0x400);

	//irqFlag = true;
	//uint8_t clocks;
	//uint32_t opcode;

	///*pCpu->pBus->cpuRead32(pCpu->pc - 1 * 4, pCpu->opcode, clocks, true);
	//irqFlag &= !pCpu->checkForCopOpcode(opcode);

	//pCpu->pBus->cpuRead32(pCpu->pc + 1 * 4, opcode, clocks, true);
	//irqFlag &= !pCpu->checkForCopOpcode(opcode);*/

	//if (irqFlag /*&& !pCpu->isCurrentOpcodeBranch*/ && executeInterrupt) {
	//	executeInterrupt = false;
	//	exceptionHandler(_INT);
	//}
}

void cp0_t::interruptHandler(const uint32_t& irq) {
	pCpu->pBus->interruptStat |= irq;
	updateInterruptMaskStat();
}
