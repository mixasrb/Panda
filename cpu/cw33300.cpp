#include "cw33300.h"
#include "../bus/bus_interface.h"

#undef DEBUG
#define DEBUG 1

extern bool g_emulationPaused;
extern bool g_executedCpuInstructionsLogged;
extern bool g_biosCallbacksLogged;

extern const char* g_sideloadPath;
extern bool g_sideload;

cpu_t::cpu_t() {
	cp0.pCpu = this;
	cp2.pCpu = this;

	primLookup.resize(0x40);
	primLookup = {
		{"SPECIAL"}, {"BcondZ "}, {"J      "}, {"JAL    "}, {"BEQ    "}, {"BNE    "}, {"BLEZ   "}, {"BGTZ   "},
		{"ADDI   "}, {"ADDIU  "}, {"SLTI   "}, {"SLTIU  "}, {"ANDI   "}, {"ORI    "}, {"XORI   "}, {"LUI    "},
		{"COP0   "}, {"COP1   "}, {"COP2   "}, {"COP3   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"LB     "}, {"LH     "}, {"LWL    "}, {"LW     "}, {"LBU    "}, {"LHU    "}, {"LWR    "}, {"N/A    "},
		{"SB     "}, {"SH     "}, {"SWL    "}, {"SW     "}, {"N/A    "}, {"N/A    "}, {"SWR    "}, {"N/A    "},
		{"LWC0   "}, {"LWC1   "}, {"LWC2   "}, {"LWC3   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"SWC0   "}, {"SWC1   "}, {"SWC2   "}, {"SWC3   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}
	};

	secLookup.resize(0x40);
	secLookup = {
		{"SLL    "}, {"N/A    "}, {"SRL    "}, {"SRA    "}, {"SLLV   "}, {"N/A    "}, {"SRLV   "}, {"SRAV   "},
		{"JR     "}, {"JALR   "}, {"N/A    "}, {"N/A    "}, {"SYSCALL"}, {"BREAK  "}, {"N/A    "}, {"N/A    "},
		{"MFHI   "}, {"MTHI   "}, {"MFLO   "}, {"MTLO   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"MULT   "}, {"MULTU  "}, {"DIV    "}, {"DIVU   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"ADD    "}, {"ADDU   "}, {"SUB    "}, {"SUBU   "}, {"AND    "}, {"OR     "}, {"XOR    "}, {"NOR    "},
		{"N/A    "}, {"N/A    "}, {"SLT    "}, {"SLTU   "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "},
		{"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}, {"N/A    "}
	};

	shift_imm.resize(4);
	shift_imm = {
		{&cpu_t::SLL}, {&cpu_t::XXX}, {&cpu_t::SRL}, {&cpu_t::SRA}
	};

	shift_reg.resize(4);
	shift_reg = {
		{&cpu_t::SLLV}, {&cpu_t::XXX2}, {&cpu_t::SRLV}, {&cpu_t::SRAV}
	};

	sys_brk.resize(2);
	sys_brk = {
		{&cpu_t::SYSCALL}, { &cpu_t::BREAK }
	};

	alu_reg.resize(16);
	alu_reg = {
		{&cpu_t::ADD}, {&cpu_t::ADDU}, {&cpu_t::SUB}, {&cpu_t::SUBU}, {&cpu_t::AND}, {&cpu_t::OR}, {&cpu_t::XOR}, {&cpu_t::NOR},
		{&cpu_t::XXX2}, {&cpu_t::XXX2}, {&cpu_t::SLT}, {&cpu_t::SLTU}, {&cpu_t::XXX2}, {&cpu_t::XXX2}, {&cpu_t::XXX2}, {&cpu_t::XXX2}
	};

	mfhi_mflo.resize(2);
	mfhi_mflo = {
		{&cpu_t::MFHI}, {&cpu_t::MFLO}
	};

	mthi_mtlo.resize(2);
	mthi_mtlo = {
		{&cpu_t::MTHI}, {&cpu_t::MTLO}
	};

	mul_div.resize(4);
	mul_div = {
		{&cpu_t::MULT}, {&cpu_t::MULTU}, {&cpu_t::DIV}, {&cpu_t::DIVU}
	};

	b_cond_z.resize(4);
	b_cond_z = {
		{&cpu_t::BLTZ}, {&cpu_t::BGEZ}, {&cpu_t::BLTZAL}, {&cpu_t::BGEZAL}
	};

	j_jal.resize(2);
	j_jal = {
		{&cpu_t::J}, {&cpu_t::JAL}
	};

	beq_bne.resize(2);
	beq_bne = {
		{&cpu_t::BEQ}, {&cpu_t::BNE}
	};

	blez_bgtz.resize(2);
	blez_bgtz = {
		{&cpu_t::BLEZ}, {&cpu_t::BGTZ}
	};

	alu_imm.resize(7);
	alu_imm = {
		{&cpu_t::ADDI}, {&cpu_t::ADDIU}, {&cpu_t::SLTI}, {&cpu_t::SLTIU}, {&cpu_t::ANDI}, {&cpu_t::ORI}, {&cpu_t::XORI}
	};

	load_rt.resize(8);
	load_rt = {
		{&cpu_t::LB}, {&cpu_t::LH}, {&cpu_t::LWL}, {&cpu_t::LW}, {&cpu_t::LBU}, {&cpu_t::LHU}, {&cpu_t::LWR}, {&cpu_t::XXX2}
	};

	store_rt.resize(8);
	store_rt = {
		{&cpu_t::SB}, {&cpu_t::SH}, {&cpu_t::SWL}, {&cpu_t::SW}, {&cpu_t::XXX1}, {&cpu_t::XXX1}, {&cpu_t::SWR}, {&cpu_t::XXX1}
	};

	//debug
	executedOpcodes.resize(32);
}

void cpu_t::read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles, bool debug) {
	pBus->cpuRead32(addr, data, cycles, debug);
}

void cpu_t::write32(const uint32_t& addr, const uint32_t& data, uint8_t& wr_cycles) {
	pBus->cpuWrite32(addr, data, wr_cycles);

	/*if (cycle == 0) {
		cycle0.bWrite = true;
		cycle0.addr = addr;
		cycle0.data = data;
		cycle0.bitness = 32;
	}
	else if (cycle == 1) {
		cycle1.bWrite = true;
		cycle1.addr = addr;
		cycle1.data = data;
		cycle1.bitness = 32;
	}*/
}

void cpu_t::read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	pBus->cpuRead16(addr, data, cycles);
}

void cpu_t::write16(const uint32_t& addr, const uint16_t& data, uint8_t& wr_cycles) {
	pBus->cpuWrite16(addr, data, wr_cycles);

	/*if (cycle == 0) {
		cycle0.bWrite = true;
		cycle0.addr = addr;
		cycle0.data = data;
		cycle0.bitness = 16;
	}
	else if (cycle == 1) {
		cycle1.bWrite = true;
		cycle1.addr = addr;
		cycle1.data = data;
		cycle1.bitness = 16;
	}*/
}

void cpu_t::read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	pBus->cpuRead8(addr, data, cycles);
}

void cpu_t::write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	pBus->cpuWrite8(addr, data, writeClocks);

	/*if (cycle == 0) {
		cycle0.bWrite = true;
		cycle0.addr = addr;
		cycle0.data = data;
		cycle0.bitness = 8;
	}
	else if (cycle == 1) {
		cycle1.bWrite = true;
		cycle1.addr = addr;
		cycle1.data = data;
		cycle1.bitness = 8;
	}*/
}

uint32_t cpu_t::get(const uint8_t& reg) {
	//if (g_executedCpuInstructionsLogged) {
	//	std::stringstream ss;
	//	ss << "r" << (uint16_t)reg << " 0x" << std::hex << r[reg] << '\n';
	//	//p_debugger->log(ss.str());
	//	loggedOpcodes.push_back(ss.str());
	//}
	return r[reg];
}

void cpu_t::set(const uint8_t& reg, const uint32_t& value, const bool loadOpcode) {
	//if (!loadOpcode) {
	if (reg != 0)
		r[reg] = value;
	return;
	//}

	/*if (pipelineCycle == 0) {
		if (cycle0.bWrite == true)
			g_emulationPaused = true;
		cycle0.bWrite = true;
		cycle0.addr = reg;
		cycle0.data = value;
	}
	else if (pipelineCycle == 1) {
		if (cycle1.bWrite == true)
			g_emulationPaused = true;
		cycle1.bWrite = true;
		cycle1.addr = reg;
		cycle1.data = value;
	}*/
}

void cpu_t::decodeExecuteOpcode(const uint32_t& opcode) {
	//debug
	//if (g_executedCpuInstructionsLogged) {
	//	//p_debugger->log(getInstructionStr(opcode, pc_old, false) + "\n");
	//	if (loggedOpcodes.size() >= 128)
	//		loggedOpcodes.clear();
	//	loggedOpcodes.push_back(getInstructionStr(opcode, pc_old, false) + "\n");
	//}

#if DEBUG
	if (g_executedCpuInstructionsLogged)
		std::cout << getInstructionStr(opcode, pc_old, false) + "\n";
#endif

	isCurrentOpcodeBranch = false;
	//if (opcode == 0)
	//	return;

	uint8_t prim_opcode = opcode >> 26;

	if (prim_opcode == 0) { // special
		uint8_t sec_opcode = opcode << 26 >> 26;

		if (sec_opcode <= 0x3) //shift-imm
			(this->*shift_imm[sec_opcode].instr)(
				opcode << 11 >> 27,
				opcode << 16 >> 27, opcode << 21 >> 27);

		else if (sec_opcode >= 0x4 && sec_opcode <= 0x7) //shift-reg
			(this->*shift_reg[sec_opcode ^ 0x04].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27, opcode << 16 >> 27);

		else if (sec_opcode == 0x08) { //jr
			isCurrentOpcodeBranch = true;
			JR(opcode << 6 >> 27);
		}

		else if (sec_opcode == 0x09) { //jalr
			isCurrentOpcodeBranch = true;
			JALR(opcode << 6 >> 27, opcode << 16 >> 27);
		}

		else if (sec_opcode >= 0xc && sec_opcode <= 0xd) //sys/brk
			(this->*sys_brk[sec_opcode ^ 0x0c].instr)(opcode >> 6);

		else if (sec_opcode == 0x10 || sec_opcode == 0x12) //mfhi/mflo
			(this->*mfhi_mflo[(sec_opcode >> 1) & 0x01].instr)(
				opcode << 16 >> 27);

		else if (sec_opcode == 0x11 || sec_opcode == 0x13) //mthi/mtlo
			(this->*mthi_mtlo[(sec_opcode >> 1) & 0x01].instr)(
				opcode << 6 >> 27);

		else if ((sec_opcode >> 2) == 0x06) //mul/div
			(this->*mul_div[sec_opcode ^ 0x18].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27);

		else if (sec_opcode >= 0x20 && sec_opcode <= 0x2f) //alu_reg
			(this->*alu_reg[sec_opcode ^ 0x20].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27, opcode << 16 >> 27);

		else {
			std::cout << "[CPU] EMULATION PAUSED! unhandled decode of opcode " << secLookup[opcode << 26 >> 26].name
				<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
			g_emulationPaused = true;
		}
	}

	else if (prim_opcode == 0x1) { //BcondZ
		isCurrentOpcodeBranch = true;
		switch (opcode << 11 >> 27) {
		case 0x10:
			(this->*b_cond_z[2].instr)(
				opcode << 6 >> 27, opcode); //bltzal
			break;
		case 0x11:
			(this->*b_cond_z[3].instr)(
				opcode << 6 >> 27, opcode); //bgezal
			break;
		default:
			if (((opcode << 11 >> 27) & 0x01) == 0)
				(this->*b_cond_z[0].instr)(
					opcode << 6 >> 27, opcode); //bltz
			else
				(this->*b_cond_z[1].instr)(
					opcode << 6 >> 27, opcode); //bgez
			break;
		}
	}

	else if (prim_opcode >= 0x02 && prim_opcode <= 0x03) { //j_jal
		isCurrentOpcodeBranch = true;
		(this->*j_jal[prim_opcode ^ 0x02].instr)(
			opcode << 6 >> 6);
	}

	else if (prim_opcode == 0x04 || prim_opcode == 0x05) { //beq/bne
		isCurrentOpcodeBranch = true;
		(this->*beq_bne[prim_opcode & 0x03].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode & 0x0000ffff);
	}

	else if (prim_opcode == 0x06 || prim_opcode == 0x07) { //blez/bgtz
		isCurrentOpcodeBranch = true;
		(this->*blez_bgtz[prim_opcode ^ 0x06].instr)(
			opcode << 6 >> 27, opcode & 0x0000ffff);
	}

	else if (prim_opcode >= 0x08 && prim_opcode < 0x0f) //alu-imm
		(this->*alu_imm[prim_opcode ^ 0x08].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode);

	else if (prim_opcode == 0x0f) //lui-imm
		LUI(opcode << 11 >> 27, opcode);

	else if (prim_opcode >= 0x20 && prim_opcode <= 0x27) //load rt,[rs+imm]
		(this->*load_rt[prim_opcode ^ 0x20].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode);

	else if (prim_opcode >= 0x28 && prim_opcode <= 0x2f) //store rt,[rs+imm]
		(this->*store_rt[prim_opcode ^ 0x28].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode);

	else if (prim_opcode & 0x10) { //coprocessors
		switch (prim_opcode & 0x03) {
		case 0: //cp0
			if ((prim_opcode & 0x20) == 0) {
				switch (opcode << 6 >> 27) {
				case 0x0:
					cp0.MFC0(opcode << 11 >> 27, opcode << 16 >> 27);
					break;
				case 0x2:
					cp0.CFC0(opcode << 11 >> 27, opcode << 16 >> 27);
					break;
				case 0x4:
					cp0.MTC0(opcode << 11 >> 27, opcode << 16 >> 27);
					break;
				case 0x6:
					cp0.CTC0(opcode << 11 >> 27, opcode << 16 >> 27);
					break;
				case 0x8:
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop0 opcode BCnF/BCnT " << primLookup[prim_opcode].name
						<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
					g_emulationPaused = true;
					break;
				case 0x10:
					cp0.RFE();
					break;
				default:
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop0 opcode " << primLookup[prim_opcode].name
						<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
					g_emulationPaused = true;
					break;
				}
			}
			else {  //LWC0/SWC0
				std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop0 opcode " << primLookup[prim_opcode].name
					<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
				g_emulationPaused = true;
			}
			break;
		case 2: //cp2
			if ((prim_opcode & 0x20) == 0)
				if (((opcode << 6 >> 27) & 0x10) == 0)
					switch (opcode << 7 >> 28)
					{
					case 0x0:
						cp2.MFC2(opcode << 11 >> 27, opcode << 16 >> 27);
						break;
					case 0x2:
						cp2.CFC2(opcode << 11 >> 27, opcode << 16 >> 27);
						break;
					case 0x4:
						cp2.MTC2(opcode << 11 >> 27, opcode << 16 >> 27);
						break;
					case 0x6:
						cp2.CTC2(opcode << 11 >> 27, opcode << 16 >> 27);
						break;
					case 0x8:
						if ((opcode << 11 >> 27) == 0)
							cp2.BC2F(opcode);
						else
							cp2.BC2T(opcode);
						break;
					default:
						std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop2 opcode " << primLookup[prim_opcode].name
							<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
						g_emulationPaused = true;
						break;
					}
				else
					cp2.COP2(opcode << 7 >> 7);
			else
				if ((prim_opcode & 0x08) == 0) //LWC2
					cp2.LWC2(opcode << 6 >> 27,
						opcode << 11 >> 27, opcode);
				else //SWC2
					cp2.SWC2(opcode << 6 >> 27,
						opcode << 11 >> 27, opcode);
			break;
		default:
			std::cout << "[CPU] EMULATION PAUSED! unhandled cop " << (uint16_t)(prim_opcode & 0x03) << " op 0x"
				<< std::hex << opcode << std::endl;
			g_emulationPaused = true;
			break;
		}
	}
	else {
		std::cout << "[CPU] EMULATION PAUSED! unhandled decode of opcode " << primLookup[prim_opcode].name << " op 0x"
			<< std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
		g_emulationPaused = true;
	}

}

void cpu_t::reset() {
	isPipelineFull = false;
	cycle = 0;
	pc = 0xbfc00000;
	cycle0.bWrite = false;
	cycle1.bWrite = false;
}

void cpu_t::clock() {

	//SIDELOAD debug
#if DEBUG
	if ((pc == 0x80030000) && (g_sideload))
		sideLoad();
#endif

	//debug
	/*if (isInstructionExecuted && (pipelineCycle == 0)) {
		pipelineCycle = 1;
		isInstructionExecuted = false;
		if (cycle0.bWrite) {
			cycle0.bWrite = false;
			if (cycle0.addr != 0)
				r[cycle0.addr] = cycle0.data;
		}
	}
	if (isInstructionExecuted && (pipelineCycle == 1)) {
		pipelineCycle = 0;
		isInstructionExecuted = false;
		if (cycle1.bWrite) {
			cycle1.bWrite = false;
			if (cycle1.addr != 0)
				r[cycle1.addr] = cycle1.data;
		}
	}*/

	if (clocks == 0) {

		r[0] = 0;

		if (cp0.executeInterrupt) {
			irqFlag = true;
			pBus->cpuRead32(pc - 1 * 4, opcode, clocks, true);
			irqFlag &= !checkForCopOpcode(opcode);

			if (irqFlag) {
				cp0.executeInterrupt = false;
				cp0.exceptionHandler(_INT);
			}
		}

		//debug
#if DEBUG
		isInstructionExecuted = false;
#endif

		if (cycle == 0) {
		label0:
			cycle = 1;


			//debug
#if DEBUG
			setBiosCallFlag();
#endif

			//fetch
			pBus->cpuRead32(pc, opcodes[0], instructionFetchClocks);

			opcode_pc_0 = pc;
			opcode_pc = opcode_pc_1;

			opcode = opcodes[1];

			//debug
#if DEBUG
			pc_old = pc;
#endif

			pc += 4;

			if (!isPipelineFull)
				goto label1;

			memoryClocks = 0;
			gteClocks = 0;

			decodeExecuteOpcode(opcodes[1]);
			clocks = (instructionFetchClocks + memoryClocks + gteClocks - 1) / 2;
			//clocks = 1;

			//debug
#if DEBUG
			printBiosCall();
#endif

			//skullmonkeys
			/*if (cycle0.bWrite) {
				cycle0.bWrite = false;
				if (r[cycle0.addr] == cycle0.old)
					r[cycle0.addr] = cycle0.data;
			}*/

			//debug
#if DEBUG
			isInstructionExecuted = true;
#endif

		}

		else {
			if (!isPipelineFull)
				goto label0;

		label1:
			r[0] = 0;
			cycle = 0;
			isPipelineFull = true;



			//debug
#if DEBUG
			setBiosCallFlag();
#endif

			//fetch
			pBus->cpuRead32(pc, opcodes[1], instructionFetchClocks1);

			opcode_pc_1 = pc;
			opcode_pc = opcode_pc_0;

			opcode = opcodes[0];

			//debug
#if DEBUG
			pc_old = pc;
#endif

			pc += 4;
			memoryClocks = 0;
			gteClocks = 0;

			decodeExecuteOpcode(opcodes[0]);
			clocks = (instructionFetchClocks + memoryClocks + gteClocks - 1) / 2;
			//clocks = 1;

			//debug
#if DEBUG
			printBiosCall();
#endif

			//skullmonkeys
			/*if (cycle1.bWrite) {
				cycle1.bWrite = false;
				if (r[cycle1.addr] == cycle1.old)
					r[cycle1.addr] = cycle1.data;
			}*/

#if DEBUG
			isInstructionExecuted = true;
#endif

		}
	}


	/*if (clocks > 16)
		g_emulationPaused = true;
	else*/
	clocks--;
}

//instructions

void cpu_t::SLL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = r[rt] << imm;
}

void cpu_t::XXX(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name
		<< " op 0x" << std::hex << opcode << std::endl;
	g_emulationPaused = true;
}

void cpu_t::SRL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = r[rt] >> imm;
}

void cpu_t::SRA(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = (int32_t)r[rt] >> imm;
}

void cpu_t::SLLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rt] << (r[rs] & 0x1f);
}

void cpu_t::SRLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rt] >> (r[rs] & 0x1f);
}

void cpu_t::SRAV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = (int32_t)r[rt] >> (r[rs] & 0x1f);
}

void cpu_t::JR(const uint8_t& rs) {
	if (r[rs] & 0x3)
		cp0.exceptionHandler(_ADEL, r[rs]);
	else
		pc = r[rs];
}

void cpu_t::JALR(const uint8_t& rs, const uint8_t& rd) {
	uint32_t tempAddr = r[rs];
	r[rd] = pc;
	if (tempAddr & 0x3)
		cp0.exceptionHandler(_ADEL, tempAddr);
	else
		pc = tempAddr;
}

void cpu_t::SYSCALL(const uint32_t& imm) {
	cp0.exceptionHandler(_SYSCALL);
}

void cpu_t::BREAK(const uint32_t& imm) {
	/*std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name
		<< "op 0x" << std::hex << opcode << std::endl;
	g_emulationPaused = true;*/
	cp0.exceptionHandler(_BP);
}

void cpu_t::MULT(const uint8_t& rs, const uint8_t& rt) {
	int64_t operandA = (int32_t)r[rs];
	int64_t operandB = (int32_t)r[rt];
	int64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
}

void cpu_t::MULTU(const uint8_t& rs, const uint8_t& rt) {
	uint64_t operandA = (uint32_t)r[rs];
	uint64_t operandB = (uint32_t)r[rt];
	uint64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
}

void cpu_t::DIV(const uint8_t& rs, const uint8_t& rt) {
	int32_t operandA = r[rs];
	int32_t operandB = r[rt];

	if ((operandA == 0x80000000) && (operandB == 0xffffffff)) {
		lo = 0x80000000;
		hi = 0x00000000;
		return;
	}

	else if (operandB == 0) {
		if (operandA < 0) {
			lo = 0x00000001;
			hi = operandA;
		}
		else {
			lo = 0xffffffff;
			hi = operandA;
		}
		return;
	}

	else if (operandB) {
		lo = operandA / operandB;
		hi = operandA % operandB;
	}
	else
		std::cout << "~[CPU] DIV divider = 0, potential problem \n";
}

void cpu_t::DIVU(const uint8_t& rs, const uint8_t& rt) {
	uint32_t operand_a = r[rs];
	uint32_t operand_b = r[rt];

	if (operand_b == 0) {
		lo = 0xffffffff;
		hi = operand_a;
		return;
	}

	else if (operand_b) {
		lo = operand_a / operand_b;
		hi = operand_a % operand_b;
	}
	else
		std::cout << "~[CPU] DIVU divider = 0, potential problem \n";
}

void cpu_t::MFHI(const uint8_t& rd) {
	r[rd] = hi;
}

void cpu_t::MFLO(const uint8_t& rd) {
	r[rd] = lo;
}

void cpu_t::MTHI(const uint8_t& rs) {
	hi = r[rs];
}

void cpu_t::MTLO(const uint8_t& rs) {
	lo = r[rs];
}

void cpu_t::ADD(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = r[rs];
	uint32_t b = r[rt];
	uint32_t res = a + b;

	bool overflow = ((a ^ res) & (b ^ res)) >> 31;
	if (overflow)
		cp0.exceptionHandler(_OV);
	else
		r[rd] = res;
}

void cpu_t::ADDU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] + r[rt];
}

void cpu_t::SUB(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = r[rs];
	uint32_t b = r[rt];
	uint32_t res = a - b;

	if (((int32_t)b > 0 && (int32_t)a < INT32_MIN + (int32_t)b) ||
		((int32_t)b < 0 && (int32_t)a > INT32_MAX + (int32_t)b))
		cp0.exceptionHandler(_OV); // ???
	else
		r[rd] = res;
}

void cpu_t::SUBU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] - r[rt];
}

void cpu_t::AND(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] & r[rt];
}

void cpu_t::OR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] | r[rt];
}

void cpu_t::XOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] ^ r[rt];
}

void cpu_t::NOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = ~(r[rs] | r[rt]);
}

void cpu_t::XXX2(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name
		<< " op 0x" << std::hex << opcode << std::endl;
	g_emulationPaused = true;
}

void cpu_t::SLT(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = ((int32_t)r[rs] < (int32_t)r[rt]) ? 1 : 0;
}

void cpu_t::SLTU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = (r[rs] < r[rt]) ? 1 : 0;
}

void cpu_t::BLTZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BGEZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BLTZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = r[rs];
	r[31] = pc;
	if (temp < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BGEZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = r[rs];
	r[31] = pc;
	if (temp >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::J(const uint32_t& imm) {
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void cpu_t::JAL(const uint32_t& imm) {
	r[31] = pc;
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void cpu_t::BEQ(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (r[rs] == r[rt])
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BNE(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (r[rs] != r[rt])
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BLEZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] <= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::BGTZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] > 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cpu_t::ADDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	uint32_t a = r[rs];
	uint16_t b = imm;
	uint32_t temp = a + (int16_t)b;

	if (((int32_t)a > 0 && (int16_t)b > 0 && (int32_t)temp < 0) ||
		((int32_t)a < 0 && (int16_t)b < 0 && (int32_t)temp > 0))
		cp0.exceptionHandler(_OV);
	else
		r[rt] = temp;
}

void cpu_t::ADDIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] + (int16_t)imm;
}

void cpu_t::SLTI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = ((int32_t)r[rs] < (int32_t)((int16_t)imm)) ? 1 : 0;
}

void cpu_t::SLTIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = (r[rs] < (uint32_t)(int32_t)((int16_t)imm)) ? 1 : 0;
}

void cpu_t::ANDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] & imm;
}

void cpu_t::ORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] | imm;
}

void cpu_t::XORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] ^ imm;
}

void cpu_t::LUI(const uint8_t& rt, const uint16_t& imm) {
	r[rt] = imm << 16;
}

void cpu_t::LB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;
	uint8_t temp;
	pBus->cpuRead8(temp_addr, temp, memoryClocks);

	r[rt] = (int32_t)((int8_t)temp);

	/*uint32_t value = (int32_t)((int8_t)temp);
	if (cycle == 0) {
		if (cycle0.bWrite == true)
			g_emulationPaused = true;
		cycle0.old = r[rt];
		cycle0.bWrite = true;
		cycle0.addr = rt;
		cycle0.data = value;
	}
	else if (cycle == 1) {
		if (cycle1.bWrite == true)
			g_emulationPaused = true;
		cycle1.old = r[rt];
		cycle1.bWrite = true;
		cycle1.addr = rt;
		cycle1.data = value;
	}*/
}

void cpu_t::LH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else {
		uint16_t temp;
		pBus->cpuRead16(temp_addr, temp, memoryClocks);

		r[rt] = (int32_t)((int16_t)temp);

		/*uint32_t value = (int32_t)((int16_t)temp);
		if (cycle == 0) {
			if (cycle0.bWrite == true)
				g_emulationPaused = true;
			cycle0.old = r[rt];
			cycle0.bWrite = true;
			cycle0.addr = rt;
			cycle0.data = value;
		}
		else if (cycle == 1) {
			if (cycle1.bWrite == true)
				g_emulationPaused = true;
			cycle1.old = r[rt];
			cycle1.bWrite = true;
			cycle1.addr = rt;
			cycle1.data = value;
		}*/
	}
}

void cpu_t::LWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->cpuRead32(temp_addr, data, memoryClocks);

	switch (offset) {
	case 0:
		r[rt] = (r[rt] << 8 >> 8) | (data << 24);
		break;
	case 1:
		r[rt] = (r[rt] << 16 >> 16) | (data << 16);
		break;
	case 2:
		r[rt] = (r[rt] << 24 >> 24) | (data << 8);
		break;
	case 3:
		r[rt] = data;
		break;
	}
}

void cpu_t::LW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x3)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else {
		pBus->cpuRead32(temp_addr, r[rt], memoryClocks);

		/*uint32_t value = temp;
		if (cycle == 0) {
			if (cycle0.bWrite == true)
				g_emulationPaused = true;
			cycle0.old = r[rt];
			cycle0.bWrite = true;
			cycle0.addr = rt;
			cycle0.data = value;
		}
		else if (cycle == 1) {
			if (cycle1.bWrite == true)
				g_emulationPaused = true;
			cycle1.old = r[rt];
			cycle1.bWrite = true;
			cycle1.addr = rt;
			cycle1.data = value;
		}*/
	}
}

void cpu_t::LBU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint8_t temp;
	pBus->cpuRead8(r[rs] + (int32_t)imm, temp, memoryClocks);

	r[rt] = temp;

	//skullmonkeys
	//uint32_t value = temp;
	//if (cycle == 0) {
	//	if (cycle0.bWrite == true)
	//		g_emulationPaused = true;
	//	cycle0.old = r[rt];
	//	cycle0.bWrite = true;
	//	cycle0.addr = rt;
	//	cycle0.data = value;
	//}
	//else if (cycle == 1) {
	//	if (cycle1.bWrite == true)
	//		g_emulationPaused = true;
	//	cycle1.old = r[rt];
	//	cycle1.bWrite = true;
	//	cycle1.addr = rt;
	//	cycle1.data = value;
	//}
}

void cpu_t::LHU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else {
		uint16_t temp;
		pBus->cpuRead16(temp_addr, temp, memoryClocks);

		r[rt] = temp;

		/*uint32_t value = temp;
		if (cycle == 0) {
			if (cycle0.bWrite == true)
				g_emulationPaused = true;
			cycle0.old = r[rt];
			cycle0.bWrite = true;
			cycle0.addr = rt;
			cycle0.data = value;
		}
		else if (cycle == 1) {
			if (cycle1.bWrite == true)
				g_emulationPaused = true;
			cycle1.old = r[rt];
			cycle1.bWrite = true;
			cycle1.addr = rt;
			cycle1.data = value;
		}*/
	}
}

void cpu_t::LWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->cpuRead32(temp_addr, data, memoryClocks);

	switch (offset) {
	case 0:
		r[rt] = data;
		break;
	case 1:
		r[rt] = (r[rt] >> 24 << 24) | (data >> 8);
		break;
	case 2:
		r[rt] = (r[rt] >> 16 << 16) | (data >> 16);
		break;
	case 3:
		r[rt] = (r[rt] >> 8 << 8) | (data >> 24);
		break;
	}
}

void cpu_t::XXX2(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << primLookup[opcode >> 26].name
		<< " op 0x" << std::hex << opcode << std::endl;
	g_emulationPaused = true;
}

void cpu_t::SB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;
	write8(temp_addr, r[rt], writeClocks);
}

void cpu_t::SH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1) {
		cp0.exceptionHandler(_ADES, temp_addr);
	}
	else
		pBus->cpuWrite16(temp_addr, r[rt], writeClocks);
}

void cpu_t::SWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->cpuRead32(temp_addr, data, memoryClocks);

	switch (offset) {
	case 0:
		pBus->cpuWrite32(temp_addr, (r[rt] >> 24) | (data >> 8 << 8), writeClocks);
		break;
	case 1:
		pBus->cpuWrite32(temp_addr, (r[rt] >> 16) | (data >> 16 << 16), writeClocks);
		break;
	case 2:
		pBus->cpuWrite32(temp_addr, (r[rt] >> 8) | (data >> 24 << 24), writeClocks);
		break;
	case 3:
		pBus->cpuWrite32(temp_addr, r[rt], writeClocks);
		break;
	}
}

void cpu_t::SW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x3)
		cp0.exceptionHandler(_ADES, temp_addr);
	else
		pBus->cpuWrite32(temp_addr, r[rt], writeClocks);
}

void cpu_t::XXX1(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << primLookup[opcode >> 26].name
		<< " op " << std::hex << opcode << std::endl;
	g_emulationPaused = true;
}

void cpu_t::SWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->cpuRead32(temp_addr, data, memoryClocks);

	switch (offset)
	{
	case 0:
		pBus->cpuWrite32(temp_addr, r[rt], memoryClocks);
		break;
	case 1:
		pBus->cpuWrite32(temp_addr, (r[rt] << 8) | (data << 24 >> 24), writeClocks);
		break;
	case 2:
		pBus->cpuWrite32(temp_addr, (r[rt] << 16) | (data << 16 >> 16), writeClocks);
		break;
	case 3:
		pBus->cpuWrite32(temp_addr, (r[rt] << 24) | (data << 8 >> 8), writeClocks);
		break;
	}
}

//debug

void cpu_t::setBiosCallFlag() {
	if (pc == 0xa0) {
		biosCallFlag = 'a';
		return;
	}
	if (pc == 0xb0) {
		biosCallFlag = 'b';
		return;
	}
	if (pc == 0xc0) {
		biosCallFlag = 'c';
		return;
	}
}

void cpu_t::printBiosCall() {
	uint8_t call = get(9);

	if (biosCallFlag.has_value()) {
		switch (biosCallFlag.value()) {
		case 'a':
			if (g_biosCallbacksLogged) {
				if (call > 0xb5)
					call = 0xb5;
				std::cout << "[KERNEL] " << pBus->biosChip.A[call].info << std::endl;

				if (call == 0x2a) {
					std::cout << "~[KERNEL] dst 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] src 0x" << std::hex << get(5) << std::endl;
					std::cout << "~[KERNEL] len 0x" << std::hex << get(6) << std::endl;
				}

				if (call == 0xab) {
					std::cout << "~[KERNEL] port 0x" << std::hex << get(4) << std::endl;
				}

				if (call == 0xaf) {
					std::cout << "~[KERNEL] port 0x" << std::hex << get(4) << std::endl;
				}
			}

			if (call == 0x3c) {
				std::stringstream ss;
				ss << (char)get(4);
				p_debugger->log(ss.str());
			}

			break;
		case 'b':
			if (g_biosCallbacksLogged) {
				if (call >= 0x5e && call <= 0xff)
					call = 0x5e;
				if (call > 0xff)
					call = 0x5f;

				if (oldCall != 0x0b) //(/*&& (call != 0x17)*//* && (call != 0x3d)*/)
					std::cout << "[KERNEL] " << pBus->biosChip.B[call].info << std::endl;

				oldCall = call;

				if (call == 0x4a) {
					std::cout << "~[KERNEL] pad_enable 0x" << std::hex << get(4) << std::endl;
				}

				if (call == 0x4d) {
					std::cout << "~[KERNEL] port 0x" << std::hex << get(4) << std::endl;
				}

				if (call == 0x02) {
					std::cout << "~[KERNEL] t 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] reload 0x" << std::hex << get(5) << std::endl;
					std::cout << "~[KERNEL] flags 0x" << std::hex << get(6) << std::endl;
				}

				if (call == 0x08) {
					std::cout << "~[KERNEL] class 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] spec 0x" << std::hex << get(5) << std::endl;
					std::cout << "~[KERNEL] mode 0x" << std::hex << get(6) << std::endl;
					std::cout << "~[KERNEL] func 0x" << std::hex << get(7) << std::endl;
				}

				if ((call == 0x07) || (call == 0x20)) {
					std::cout << "~[KERNEL] event 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] specs 0x" << std::hex << get(5) << std::endl;
				}

				if ((call == 0x0b) || (call == 0x0c) || (call == 0x09)) {
					uint8_t cycles;
					uint32_t ptr;
					uint32_t data;
					read32(0x120, ptr, cycles);
					ptr += 7 * 4 * (get(4) & 0xffff);
					read32(ptr, data, cycles);
					std::cout << "~[KERNEL] event 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] class 0x" << std::hex << data << std::endl;
					read32(ptr + 0x4, data, cycles);
					std::cout << "~[KERNEL] flag 0x" << std::hex << data << std::endl;
					read32(ptr + 0x8, data, cycles);
					std::cout << "~[KERNEL] specs 0x" << std::hex << data << std::endl;
					read32(ptr + 0xc, data, cycles);
					std::cout << "~[KERNEL] mode 0x" << std::hex << data << std::endl;
				}
			}

			/*if (call == 0x0b) {
				uint8_t cycles;
				uint32_t ptr;
				uint32_t data;
				read32(0x120, ptr, cycles);
				ptr += 7 * 4 * (get(4) & 0xffff);
				read32(ptr, data, cycles);
				if (data != 0xf0000009) {
					std::cout << "event 0x" << std::hex << get(4) << std::endl;
					std::cout << "class 0x" << std::hex << data << std::endl;
					read32(ptr + 0x4, data, cycles);
					std::cout << "flag 0x" << std::hex << data << std::endl;
					read32(ptr + 0x8, data, cycles);
					std::cout << "specs 0x" << std::hex << data << std::endl;
					read32(ptr + 0xc, data, cycles);
					std::cout << "mode 0x" << std::hex << data << std::endl;
				}
			}*/

			if (call == 0x3d) {
				std::stringstream ss;
				ss << (char)get(4);
				p_debugger->log(ss.str());
			}

			if (g_biosCallbacksLogged) {
				if ((call == 0x4e) || (call == 0x4f)) {
					std::cout << "~[KERNEL] port 0x" << std::hex << get(4) << std::endl;
					std::cout << "~[KERNEL] sector 0x" << std::hex << get(5) << std::endl;
					std::cout << "~[KERNEL] dst/src 0x" << std::hex << get(6) << std::endl;
					g_emulationPaused = true;
				}

				if ((call == 0x5c) || (call == 0x5d)) {
					std::cout << "~[KERNEL] slot 0x" << std::hex << get(4) << std::endl;
				}
			}

			break;
		case 'c':
			if (g_biosCallbacksLogged) {
				if (call > 0x20)
					call = 0x20;
				std::cout << "[KERNEL] " << pBus->biosChip.C[call].info << std::endl;
			}

			break;
		}

		biosCallFlag.reset();
	}
}

std::string cpu_t::getInstructionStr(const uint32_t& opcode, const uint32_t& pc, const bool& b) {
	std::string s;
	std::stringstream ss;
	ss << "0x" << std::hex << pc << ": ";
	s = ss.str();

	uint8_t prim_opcode = opcode >> 26;
	if (prim_opcode != 0x00) {
		//if (b) {
		//	//std::cout << prim_lookup[prim_opcode].name << "pc 0x" << std::hex << pc_old << "\n";
		//	std::stringstream ss;
		//	ss << primLookup[prim_opcode].name << "pc 0x" << std::hex << pc_old
		//		<< "opcode " << std::hex << opcode << "\n";
		//	printf(ss.str().c_str());
		//	/*if (pc > 0x8005000)
		//		ss << prim_lookup[prim_opcode].name << "pc 0x" << std::hex << pc_old << "\n";
		//	p_debugger->Log(ss.str());*/
		//}
		s += primLookup[prim_opcode].name;
	}
	else {
		uint8_t sec_opcode = opcode << 26 >> 26;
		//if (b) {
		//	//std::cout << sec_lookup[sec_opcode].name << "pc 0x" << std::hex << pc_old << "\n";
		//	std::stringstream ss;
		//	ss << secLookup[sec_opcode].name << "pc 0x" << std::hex << pc_old
		//		<< "opcode " << std::hex << opcode << "\n";
		//	printf(ss.str().c_str());
		//	/*ss << sec_lookup[sec_opcode].name << "pc 0x" << std::hex << pc_old << "\n";
		//	p_debugger->Log(ss.str());*/
		//}
		s += secLookup[sec_opcode].name;
	}
	s += getDecodedInstructionStr(opcode, pc);

	//std::cout << "____________________________________________________\n";
	return s;
}

std::string cpu_t::getDecodedInstructionStr(const uint32_t& opcode, const uint32_t& pc) {
	uint8_t prim_opcode = opcode >> 26;

	std::stringstream ss;

	if (prim_opcode == 0) { // special

		uint8_t sec_opcode = opcode << 26 >> 26;

		if (sec_opcode >= 0 && sec_opcode <= 0x3) // shift-imm
			ss << "rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27) << " imm5 0x" << std::hex << (opcode << 21 >> 27);

		else if (sec_opcode >= 0x4 && sec_opcode < 0x8) //shift-reg
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27);

		else if (sec_opcode == 0x08) // jr
			ss << "rs" << (opcode << 6 >> 27);

		else if (sec_opcode == 0x09) // jalr
			ss << "rs" << (opcode << 6 >> 27) << " rd" << (opcode << 16 >> 27);

		else if (sec_opcode >= 0xc && sec_opcode <= 0xd) //sys/brk
			ss << "comment" << (opcode << 6 >> 12);

		else if (sec_opcode == 0x10 || sec_opcode == 0x12) //mfhi/mflo
			ss << "rd" << (opcode << 16 >> 27);

		else if (sec_opcode == 0x11 || sec_opcode == 0x13) //mthi/mtlo
			ss << "rs" << (opcode << 6 >> 27);

		else if ((sec_opcode >> 2) == 0x06) //mul/div
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27);

		else if (sec_opcode >= 0x20 && sec_opcode <= 0x2f) // alu_reg
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27);
	}

	else if (prim_opcode == 0x1) { //BcondZ
		switch (opcode << 11 >> 27) {
		case 0x10: //bltzal
			ss << "BLTZAL rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
			break;
		case 0x11: //bgezal
			ss << "BGEZAL rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
			break;
		default:
			if (!((opcode << 11 >> 27) & 0x1)) //bltz
				ss << "BLTZ rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
			else //bgez
				ss << "BGEZ rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
			break;
		}
	}

	else if (prim_opcode >= 0x2 && prim_opcode <= 0x3) // j_jal
		ss << "imm26 0x" << std::hex << (opcode << 6 >> 6);

	else if (prim_opcode == 0x04 || prim_opcode == 0x05) // beq/bne
		ss << "{0x" << std::hex << pc + (int16_t)(opcode & 0x0000ffff) * 4 - 4 << "} "
		<< std::dec << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode == 0x06 || prim_opcode == 0x07) // blez/bgtz
		ss << "rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode >= 0x08 && prim_opcode < 0x0f) // alu-imm
		ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode == 0x0f) // lui-imm
		ss << "rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode >= 0x20 && prim_opcode <= 0x27) // load rt,[rs+imm]
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << (int16_t)(opcode & 0x0000ffff) + get(opcode << 6 >> 27) << "]"
		<< std::dec << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode >= 0x28 && prim_opcode <= 0x2f) // store rt,[rs+imm]
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << (int16_t)(opcode & 0x0000ffff) + get(opcode << 6 >> 27) << "]"
		<< std::dec << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode & 0x10) { //coprocessors
		switch (prim_opcode & 0x03) {
		case 0: //cp0
			if ((prim_opcode & 0x20) == 0) {
				switch (opcode << 6 >> 27)
				{
				case 0x0:
					ss << "MFC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					break;
				case 0x2:
					ss << "CFC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					break;
				case 0x4:
					ss << "MTC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					break;
				case 0x6:
					ss << "CTC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					break;
				case 0x8:
					//
					break;
				case 0x10:
					ss << "RFE";
					break;
				}
			}
			else {  //LWC0/SWC0
				//
			}
			break;
		case 2: //cop2
			if ((prim_opcode & 0x20) == 0)
				if (((opcode << 6 >> 27) & 0x10) == 0)
					switch (opcode << 7 >> 28)
					{
					case 0x0:
						ss << "MFC2 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
						break;
					case 0x2:
						ss << "CFC2 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
						break;
					case 0x4:
						ss << "MTC2 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
						break;
					case 0x6:
						ss << "CTC2 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
						break;
					case 0x8:
						if ((opcode << 11 >> 27) == 0)
							ss << "BC2F imm16 0x" << (opcode & 0x0000ffff);
						else
							ss << "BC2T imm16 0x" << (opcode & 0x0000ffff);
						break;
					}
				else
					ss << "COP2 imm25 0x" << (opcode << 7 >> 7);
			else
				if ((prim_opcode & 0x08) == 0) //LWC2
					ss << "LWC2 rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
				else //SWC2
					ss << "SWC2 rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
			break;
		}
	}

	else
		ss << "Unknown ???";

	return ss.str();
}

void cpu_t::sideLoad() {
	g_sideload = false;
	std::ifstream file(g_sideloadPath, std::ifstream::binary | std::ifstream::ate);
	if (!file.is_open()) {
		std::cout << "failed to open sideload!\n";
		g_emulationPaused = true;
	}

	size_t lenght = (size_t)file.tellg();
	std::vector<uint8_t> buffer(lenght);
	file.seekg(0);
	file.read((char*)buffer.data(), lenght);
	file.close();

	pc = (((uint32_t)buffer[0x13]) << 24) | (((uint32_t)buffer[0x12]) << 16)
		| (((uint32_t)buffer[0x11]) << 8) | (uint32_t)buffer[0x10];

	set(28, (((uint32_t)buffer[0x17]) << 24) | (((uint32_t)buffer[0x16]) << 16)
		| (((uint32_t)buffer[0x15]) << 8) | (uint32_t)buffer[0x14]);

	uint32_t ramAddr = (((uint32_t)buffer[0x1b]) << 24) | (((uint32_t)buffer[0x1a]) << 16)
		| (((uint32_t)buffer[0x19]) << 8) | (uint32_t)buffer[0x18];

	uint32_t size = (((uint32_t)buffer[0x1f]) << 24) | (((uint32_t)buffer[0x1e]) << 16)
		| (((uint32_t)buffer[0x1d]) << 8) | (uint32_t)buffer[0x1c];

	uint32_t initialSpFp = (((uint32_t)buffer[0x33]) << 24) | (((uint32_t)buffer[0x32]) << 16)
		| (((uint32_t)buffer[0x31]) << 8) | (uint32_t)buffer[0x30];
	if (initialSpFp) {
		set(29, initialSpFp);
		set(30, initialSpFp);
	}

	uint32_t initialSpFpOffset = (((uint32_t)buffer[0x37]) << 24) | (((uint32_t)buffer[0x36]) << 16)
		| (((uint32_t)buffer[0x35]) << 8) | (uint32_t)buffer[0x34];
	if (initialSpFpOffset) {
		std::cout << "Unhandled Initial SP/R29 & FP/R30 Offs in PSX.EXE (sideload)\n";
		g_emulationPaused = true;
	}

	uint8_t cycles;
	for (uint32_t i = 0; i < size; i++) {
		write8(ramAddr + i, buffer[i + 0x800], cycles);
	}
}

bool cpu_t::checkForCopOpcode(const uint32_t& opcode) {
	uint8_t prim_opcode = opcode >> 26;

	if (prim_opcode & 0x10) { //coprocessors
		switch (prim_opcode & 0x03) {
		case 0: //cp0
			return true;
			break;
		case 2: //cp2
			return true;
			break;
		}
	}
	return false;
}
