#include "cw33300.h"
#include "../bus/bus_interface.h"

#undef CPU_DEBUG
#define CPU_DEBUG 1

extern bool g_emulationPaused;
extern bool g_executedCpuInstructionsLogged;
extern bool g_biosCallbacksLogged;

extern const char* g_sideloadPath;
extern bool g_sideload;

Cpu::Cpu() {
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

	shift_imm[0] = {&Cpu::SLL};
	shift_imm[1] = {&Cpu::XXX};
	shift_imm[2] = {&Cpu::SRL};
	shift_imm[3] = {&Cpu::SRA};

	shift_reg[0] = {&Cpu::SLLV};
	shift_reg[1] = {&Cpu::XXX2};
	shift_reg[2] = {&Cpu::SRLV};
	shift_reg[3] = {&Cpu::SRAV};

	sys_brk[0] = {&Cpu::SYSCALL};
	sys_brk[1] = {&Cpu::BREAK};

	mul_div[0] = { &Cpu::MULT };
	mul_div[1] = { &Cpu::MULTU };
	mul_div[2] = { &Cpu::DIV };
	mul_div[3] = { &Cpu::DIVU };

	mfhi_mflo[0] = { &Cpu::MFHI };
	mfhi_mflo[1] = { &Cpu::MFLO };

	mthi_mtlo[0] = { &Cpu::MTHI };
	mthi_mtlo[1] = { &Cpu::MTLO };

	alu_reg[0] = { &Cpu::ADD };
	alu_reg[1] = { &Cpu::ADDU };
	alu_reg[2] = { &Cpu::SUB };
	alu_reg[3] = { &Cpu::SUBU };
	alu_reg[4] = { &Cpu::AND };
	alu_reg[5] = { &Cpu::OR };
	alu_reg[6] = { &Cpu::XOR };
	alu_reg[7] = { &Cpu::NOR };
	alu_reg[8] = { &Cpu::XXX2 };
	alu_reg[9] = { &Cpu::XXX2 };
	alu_reg[10] = { &Cpu::SLT };
	alu_reg[11] = { &Cpu::SLTU };
	alu_reg[12] = { &Cpu::XXX2 };
	alu_reg[13] = { &Cpu::XXX2 };
	alu_reg[14] = { &Cpu::XXX2 };
	alu_reg[15] = { &Cpu::XXX2 };

	b_cond_z[0] = {&Cpu::BLTZ};
	b_cond_z[1] = {&Cpu::BGEZ};
	b_cond_z[2] = {&Cpu::BLTZAL};
	b_cond_z[3] = {&Cpu::BGEZAL};

	j_jal[0] = {&Cpu::J};
	j_jal[1] = {&Cpu::JAL};

	beq_bne[0] = {&Cpu::BEQ};
	beq_bne[1] = {&Cpu::BNE};

	blez_bgtz[0] = {&Cpu::BLEZ};
	blez_bgtz[1] = {&Cpu::BGTZ};

	alu_imm[0] = {&Cpu::ADDI};
	alu_imm[1] = {&Cpu::ADDIU};
	alu_imm[2] = {&Cpu::SLTI};
	alu_imm[3] = {&Cpu::SLTIU};
	alu_imm[4] = {&Cpu::ANDI};
	alu_imm[5] = {&Cpu::ORI};
	alu_imm[6] = {&Cpu::XORI};

	load_rt[0] = {&Cpu::LB};
	load_rt[1] = {&Cpu::LH};
	load_rt[2] = {&Cpu::LWL};
	load_rt[3] = {&Cpu::LW};
	load_rt[4] = {&Cpu::LBU};
	load_rt[5] = {&Cpu::LHU};
	load_rt[6] = {&Cpu::LWR};
	load_rt[7] = {&Cpu::XXX2};

	store_rt[0] = {&Cpu::SB};
	store_rt[1] = {&Cpu::SH};
	store_rt[2] = {&Cpu::SWL};
	store_rt[3] = {&Cpu::SW};
	store_rt[4] = {&Cpu::XXX1};
	store_rt[5] = {&Cpu::XXX1};
	store_rt[6] = {&Cpu::SWR};
	store_rt[7] = {&Cpu::XXX1};

	//debug
	executedOpcodes.resize(32);
}

void Cpu::Read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles,const bool& debug) {
	pBus->Read32(addr, data, cycles, debug);
}

void Cpu::Write32(const uint32_t& addr, const uint32_t& data, uint8_t& wr_cycles) {
	pBus->Write32(addr, data, wr_cycles);
}

void Cpu::Read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	pBus->Read16(addr, data, cycles);
}

void Cpu::Write16(const uint32_t& addr, const uint16_t& data, uint8_t& wr_cycles) {
	pBus->Write16(addr, data, wr_cycles);
}

void Cpu::Read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	pBus->Read8(addr, data, cycles);
}

void Cpu::Write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	pBus->Write8(addr, data, writeClocks);
}

uint32_t Cpu::GetRegister(const uint8_t& reg) {
	return r[reg];
}

void Cpu::SetRegister(const uint8_t& reg, const uint32_t& value, const bool& loadOpcode) {
	if (reg)
		r[reg] = value;
}

void Cpu::DecodeExecuteOpcode(const uint32_t& opcode) {
#if CPU_DEBUG
	if (g_executedCpuInstructionsLogged)
		std::cout << getInstructionStr(opcode, pc_old, false) + "\n";
#endif

	isCurrentOpcodeBranch = false;

	uint8_t prim_opcode = opcode >> 26;

	if (prim_opcode == 0) { // special
		uint8_t sec_opcode = opcode << 26 >> 26;

		/*switch (sec_opcode) {
		case 0b000000:
		case 0b000001:
		case 0b000010:
		case 0b000011:
			(this->*shift_imm[sec_opcode].instr)(
				opcode << 11 >> 27,
				opcode << 16 >> 27, opcode << 21 >> 27);
			break;
		case 0b000100:
		case 0b000101:
		case 0b000110:
		case 0b000111:
			(this->*shift_reg[sec_opcode ^ 0x04].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27, opcode << 16 >> 27);
			break;
		case 0b001000:
			isCurrentOpcodeBranch = true;
			JR(opcode << 6 >> 27);
			break;
		case 0b001001:
			isCurrentOpcodeBranch = true;
			JALR(opcode << 6 >> 27, opcode << 16 >> 27);
			break;
		case 0b001100:
		case 0b001101:
			(this->*sys_brk[sec_opcode ^ 0x0c].instr)(opcode >> 6);
			break;
		case 0b010000:
		case 0b010010:
			(this->*mfhi_mflo[(sec_opcode >> 1) & 0x01].instr)(
				opcode << 16 >> 27);
			break;
		case 0b010001:
		case 0b010011:
			(this->*mthi_mtlo[(sec_opcode >> 1) & 0x01].instr)(
				opcode << 6 >> 27);
			break;
		case 0b011000:
		case 0b011001:
		case 0b011010:
		case 0b011011:
			(this->*mul_div[sec_opcode ^ 0x18].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27);
			break;
		case 0b100000:
		case 0b100001:
		case 0b100010:
		case 0b100011:
		case 0b100100:
		case 0b100101:
		case 0b100110:
		case 0b100111:
		case 0b101000:
		case 0b101001:
		case 0b101010:
		case 0b101011:
		case 0b101100:
		case 0b101101:
		case 0b101110:
		case 0b101111:
			(this->*alu_reg[sec_opcode ^ 0x20].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27, opcode << 16 >> 27);
			break;

		}*/


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
				<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
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
						<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
					g_emulationPaused = true;
					break;
				case 0x10:
					cp0.RFE();
					break;
				default:
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop0 opcode " << primLookup[prim_opcode].name
						<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
					g_emulationPaused = true;
					break;
				}
			}
			else {  //LWC0/SWC0
				std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop0 opcode " << primLookup[prim_opcode].name
					<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
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
							<< " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
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
				<< std::hex << opcode << '\n';
			g_emulationPaused = true;
			break;
		}
	}
	else {
		std::cout << "[CPU] EMULATION PAUSED! unhandled decode of opcode " << primLookup[prim_opcode].name << " op 0x"
			<< std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << '\n';
		g_emulationPaused = true;
	}

}

void Cpu::Reset() {
	cycle = 0;
	pc = 0xbfc00000;

	cycle0.bWrite = false;
	cycle1.bWrite = false;
}

void Cpu::Clock() {

	//SIDELOAD debug
#if CPU_DEBUG
	if ((pc == 0x80030000) && (g_sideload))
		sideLoad();
#endif

	if (clocks == 0) {

		if (cp0.executeInterrupt) {
			bInterrupt = true;
			pBus->Read32(pc - 1 * 4, opcode, clocks, true);
			//Maybe can cause problems but possibilty should be small
			bInterrupt &= !checkForCopOpcode(opcode) & !cycle0.bWrite & !cycle1.bWrite;

			if (bInterrupt) {
				cp0.executeInterrupt = false;
				cp0.exceptionHandler(_INT);
			}
		}

		//debug
#if CPU_DEBUG
		setBiosCallFlag();
		pc_old = pc;
#endif

		if (!cycle) { //First cycle
			cycle = 1;

			//fetch
			pBus->Read32(pc, opcodes[0], instructionFetchClocks);
			if (pc >= 0xa0000000)
				clocks += instructionFetchClocks;

			//	if ((pc >= 0xa0000000) || (!iCacheEnabled)) {
			//		//fetch
			//		pBus->cpuRead32(pc, opcodes[0], instructionFetchClocks);
			//		clocks += instructionFetchClocks;
			//	}
			//	else {
			//		for (uint16_t i = 0; i < 16; i++) {
			//			if (iCache[(iCacheCounter + i) & 0xf].pc == pc) {
			//				opcodes[0] = iCache[(iCacheCounter + i) & 0xf].opcode;
			//				goto skipFetch0;
			//			}
			//		}
			//		iCache[iCacheCounter].pc = pc;
			//		//fetch
			//		pBus->cpuRead32(pc, iCache[iCacheCounter].opcode, instructionFetchClocks);
			//		opcodes[0] = iCache[iCacheCounter].opcode;
			//		iCacheCounter = ++iCacheCounter & 0xf;
			//		clocks += instructionFetchClocks;
			//	}
			//skipFetch0:

			opcode_pc_0 = pc;
			opcode_pc = opcode_pc_1;
			opcode = opcodes[1];
		}
		else { //Second cycle
			cycle = 0;

			//fetch
			pBus->Read32(pc, opcodes[1], instructionFetchClocks);
			if (pc >= 0xa0000000)
				clocks += instructionFetchClocks;

			//	if ((pc >= 0xa0000000) || (!iCacheEnabled)) {
			//		//fetch
			//		pBus->cpuRead32(pc, opcodes[1], instructionFetchClocks);
			//		clocks += instructionFetchClocks;
			//	}
			//	else {
			//		for (uint16_t i = 0; i < 16; i++) {
			//			if (iCache[(iCacheCounter + i) & 0xf].pc == pc) {
			//				opcodes[1] = iCache[(iCacheCounter + i) & 0xf].opcode;
			//				goto skipFetch1;
			//			}
			//		}
			//		iCache[iCacheCounter].pc = pc;
			//		//fetch
			//		pBus->cpuRead32(pc, iCache[iCacheCounter].opcode, instructionFetchClocks);
			//		opcodes[1] = iCache[iCacheCounter].opcode;
			//		iCacheCounter = ++iCacheCounter & 0xf;
			//		clocks += instructionFetchClocks;
			//	}
			//skipFetch1:

			opcode_pc_1 = pc;
			opcode_pc = opcode_pc_0;

			opcode = opcodes[0];
		}


		//Hack Polaris SnoCross (USA)
		/*if (pc == 0x800133fc)
			r[2] = 1; */

		//Hack Hot Wheels - Turbo Racing (USA)
		/*if (pc == 0x80101310)
			r[2] = 1;
		if (pc == 0x800b42e0)
			r[2] = 0x200;*/


		pc += 4;

		memoryClocks = 0;
		gteClocks = 0;

		if (cycle == 0) {
			if (cycle0.bWrite) {
				cycle0.bWrite = false;
				if (r[cycle0.addr] == cycle0.old)
					r[cycle0.addr] = cycle0.data;
			}
		}
		else {
			if (cycle1.bWrite) {
				cycle1.bWrite = false;
				if (r[cycle1.addr] == cycle1.old)
					r[cycle1.addr] = cycle1.data;
			}
		}

		r[0] = 0;

		DecodeExecuteOpcode(opcode);

		//assumes that all instructions from cached regions are cached
		clocks += memoryClocks + gteClocks;

		//debug
#if CPU_DEBUG
		printBiosCall();
		isInstructionExecuted = true;
#endif

	}

	if (clocks)
		clocks--;

	cp2.clock();
}

//instructions

void Cpu::SLL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = r[rt] << imm;
}

void Cpu::XXX(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name
		<< " op 0x" << std::hex << opcode << '\n';
	g_emulationPaused = true;
}

void Cpu::SRL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = r[rt] >> imm;
}

void Cpu::SRA(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	r[rd] = (int32_t)r[rt] >> imm;
}

void Cpu::SLLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rt] << (r[rs] & 0x1f);
}

void Cpu::SRLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rt] >> (r[rs] & 0x1f);
}

void Cpu::SRAV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = (int32_t)r[rt] >> (r[rs] & 0x1f);
}

void Cpu::JR(const uint8_t& rs) {
	if (r[rs] & 0x3)
		cp0.exceptionHandler(_ADEL, r[rs]);
	else
		pc = r[rs];
}

void Cpu::JALR(const uint8_t& rs, const uint8_t& rd) {
	uint32_t tempAddr = r[rs];
	r[rd] = pc;
	if (tempAddr & 0x3)
		cp0.exceptionHandler(_ADEL, tempAddr);
	else
		pc = tempAddr;
}

void Cpu::SYSCALL(const uint32_t& imm) {
	cp0.exceptionHandler(_SYSCALL);
}

void Cpu::BREAK(const uint32_t& imm) {
	cp0.exceptionHandler(_BP);
}

void Cpu::MULT(const uint8_t& rs, const uint8_t& rt) {
	int64_t operandA = (int32_t)r[rs];
	int64_t operandB = (int32_t)r[rt];
	int64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
}

void Cpu::MULTU(const uint8_t& rs, const uint8_t& rt) {
	uint64_t operandA = (uint32_t)r[rs];
	uint64_t operandB = (uint32_t)r[rt];
	uint64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
}

void Cpu::DIV(const uint8_t& rs, const uint8_t& rt) {
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
	else {
		std::cout << "~[CPU] DIV divider = 0, potential problem \n";
		g_emulationPaused = true;
	}
}

void Cpu::DIVU(const uint8_t& rs, const uint8_t& rt) {
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
	else {
		std::cout << "~[CPU] DIVU divider = 0, potential problem \n";
		g_emulationPaused = true;
	}
}

void Cpu::MFHI(const uint8_t& rd) {
	r[rd] = hi;
}

void Cpu::MFLO(const uint8_t& rd) {
	r[rd] = lo;
}

void Cpu::MTHI(const uint8_t& rs) {
	hi = r[rs];
}

void Cpu::MTLO(const uint8_t& rs) {
	lo = r[rs];
}

void Cpu::ADD(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = r[rs];
	uint32_t b = r[rt];
	uint32_t res = a + b;

	bool overflow = ((a ^ res) & (b ^ res)) >> 31;
	if (overflow)
		cp0.exceptionHandler(_OV);
	else
		r[rd] = res;
}

void Cpu::ADDU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] + r[rt];
}

void Cpu::SUB(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = r[rs];
	uint32_t b = r[rt];
	uint32_t res = a - b;

	if (((int32_t)b > 0 && (int32_t)a < INT32_MIN + (int32_t)b) ||
		((int32_t)b < 0 && (int32_t)a > INT32_MAX + (int32_t)b))
		cp0.exceptionHandler(_OV); // ???
	else
		r[rd] = res;
}

void Cpu::SUBU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] - r[rt];
}

void Cpu::AND(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] & r[rt];
}

void Cpu::OR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] | r[rt];
}

void Cpu::XOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = r[rs] ^ r[rt];
}

void Cpu::NOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = ~(r[rs] | r[rt]);
}

void Cpu::XXX2(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name
		<< " op 0x" << std::hex << opcode << '\n';
	g_emulationPaused = true;
}

void Cpu::SLT(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = ((int32_t)r[rs] < (int32_t)r[rt]) ? 1 : 0;
}

void Cpu::SLTU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	r[rd] = (r[rs] < r[rt]) ? 1 : 0;
}

void Cpu::BLTZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BGEZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BLTZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = r[rs];
	r[31] = pc;
	if (temp < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BGEZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = r[rs];
	r[31] = pc;
	if (temp >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::J(const uint32_t& imm) {
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void Cpu::JAL(const uint32_t& imm) {
	r[31] = pc;
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void Cpu::BEQ(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (r[rs] == r[rt])
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BNE(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (r[rs] != r[rt])
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BLEZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] <= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::BGTZ(const uint8_t& rs, const uint16_t& imm) {
	if ((int32_t)r[rs] > 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void Cpu::ADDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	uint32_t a = r[rs];
	uint16_t b = imm;
	uint32_t temp = a + (int16_t)b;

	if (((int32_t)a > 0 && (int16_t)b > 0 && (int32_t)temp < 0) ||
		((int32_t)a < 0 && (int16_t)b < 0 && (int32_t)temp > 0))
		cp0.exceptionHandler(_OV);
	else
		r[rt] = temp;
}

void Cpu::ADDIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] + (int16_t)imm;
}

void Cpu::SLTI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = ((int32_t)r[rs] < (int32_t)((int16_t)imm)) ? 1 : 0;
}

void Cpu::SLTIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = (r[rs] < (uint32_t)(int32_t)((int16_t)imm)) ? 1 : 0;
}

void Cpu::ANDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] & imm;
}

void Cpu::ORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] | imm;
}

void Cpu::XORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	r[rt] = r[rs] ^ imm;
}

void Cpu::LUI(const uint8_t& rt, const uint16_t& imm) {
	r[rt] = imm << 16;
}

void Cpu::LB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;
	uint8_t temp;
	pBus->Read8(temp_addr, temp, memoryClocks);

	r[rt] = (int32_t)((int8_t)temp);
}

void Cpu::LH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else {
		uint16_t temp;
		pBus->Read16(temp_addr, temp, memoryClocks);

		r[rt] = (int32_t)((int16_t)temp);
	}
}

void Cpu::LWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->Read32(temp_addr, data, memoryClocks);

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

void Cpu::LW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x3)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else
		pBus->Read32(temp_addr, r[rt], memoryClocks);
}

void Cpu::LBU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint8_t temp;
	pBus->Read8(r[rs] + (int32_t)imm, temp, memoryClocks);

	r[rt] = temp;

	//skullmonkeys
}

void Cpu::LHU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1)
		cp0.exceptionHandler(_ADEL, temp_addr);
	else {
		uint16_t temp;
		pBus->Read16(temp_addr, temp, memoryClocks);

		r[rt] = temp;
	}
}

void Cpu::LWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->Read32(temp_addr, data, memoryClocks);

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

void Cpu::XXX2(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << primLookup[opcode >> 26].name
		<< " op 0x" << std::hex << opcode << '\n';
	g_emulationPaused = true;
}

void Cpu::SB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;
	Write8(temp_addr, r[rt], writeClocks);
}

void Cpu::SH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x1) {
		cp0.exceptionHandler(_ADES, temp_addr);
	}
	else
		pBus->Write16(temp_addr, r[rt], writeClocks);
}

void Cpu::SWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->Read32(temp_addr, data, memoryClocks);

	switch (offset) {
	case 0:
		pBus->Write32(temp_addr, (r[rt] >> 24) | (data >> 8 << 8), writeClocks);
		break;
	case 1:
		pBus->Write32(temp_addr, (r[rt] >> 16) | (data >> 16 << 16), writeClocks);
		break;
	case 2:
		pBus->Write32(temp_addr, (r[rt] >> 8) | (data >> 24 << 24), writeClocks);
		break;
	case 3:
		pBus->Write32(temp_addr, r[rt], writeClocks);
		break;
	}
}

void Cpu::SW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = r[rs] + (int32_t)imm;

	if (temp_addr & 0x3)
		cp0.exceptionHandler(_ADES, temp_addr);
	else
		pBus->Write32(temp_addr, r[rt], writeClocks);
}

void Cpu::XXX1(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << primLookup[opcode >> 26].name
		<< " op " << std::hex << opcode << '\n';
	g_emulationPaused = true;
}

void Cpu::SWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = r[rs] + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;

	pBus->Read32(temp_addr, data, memoryClocks);

	switch (offset)
	{
	case 0:
		pBus->Write32(temp_addr, r[rt], memoryClocks);
		break;
	case 1:
		pBus->Write32(temp_addr, (r[rt] << 8) | (data << 24 >> 24), writeClocks);
		break;
	case 2:
		pBus->Write32(temp_addr, (r[rt] << 16) | (data << 16 >> 16), writeClocks);
		break;
	case 3:
		pBus->Write32(temp_addr, (r[rt] << 24) | (data << 8 >> 8), writeClocks);
		break;
	}
}

//debug

void Cpu::setBiosCallFlag() {
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

void Cpu::printBiosCall() {
	uint8_t call = GetRegister(9);

	if (biosCallFlag.has_value()) {
		switch (biosCallFlag.value()) {
		case 'a':
			if (g_biosCallbacksLogged) {
				if (call > 0xb5)
					call = 0xb5;
				std::cout << "[KERNEL] " << pBus->biosChip.A[call].info << '\n';

				if (call == 0x2a) {
					std::cout << "~[KERNEL] dst 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] src 0x" << std::hex << GetRegister(5) << '\n';
					std::cout << "~[KERNEL] len 0x" << std::hex << GetRegister(6) << '\n';
				}

				if (call == 0xab) {
					std::cout << "~[KERNEL] port 0x" << std::hex << GetRegister(4) << '\n';
				}

				if (call == 0xaf) {
					std::cout << "~[KERNEL] port 0x" << std::hex << GetRegister(4) << '\n';
				}
			}

			if (call == 0x3c) {
				std::stringstream ss;
				ss << (char)GetRegister(4);
				p_debugger->log(ss.str());
				std::cout << ss.str();
			}

			break;
		case 'b':
			if (g_biosCallbacksLogged) {
				if (call >= 0x5e && call <= 0xff)
					call = 0x5e;
				if (call > 0xff)
					call = 0x5f;

				if (oldCall != 0x0b) //(/*&& (call != 0x17)*//* && (call != 0x3d)*/)
					std::cout << "[KERNEL] " << pBus->biosChip.B[call].info << '\n';

				oldCall = call;

				if (call == 0x4a) {
					std::cout << "~[KERNEL] pad_enable 0x" << std::hex << GetRegister(4) << '\n';
				}

				if (call == 0x4d) {
					std::cout << "~[KERNEL] port 0x" << std::hex << GetRegister(4) << '\n';
				}

				if (call == 0x02) {
					std::cout << "~[KERNEL] t 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] reload 0x" << std::hex << GetRegister(5) << '\n';
					std::cout << "~[KERNEL] flags 0x" << std::hex << GetRegister(6) << '\n';
				}

				if (call == 0x08) {
					std::cout << "~[KERNEL] class 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] spec 0x" << std::hex << GetRegister(5) << '\n';
					std::cout << "~[KERNEL] mode 0x" << std::hex << GetRegister(6) << '\n';
					std::cout << "~[KERNEL] func 0x" << std::hex << GetRegister(7) << '\n';
				}

				if ((call == 0x07) || (call == 0x20)) {
					std::cout << "~[KERNEL] event 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] specs 0x" << std::hex << GetRegister(5) << '\n';
				}

				if ((call == 0x0b) || (call == 0x0c) || (call == 0x09)) {
					uint8_t cycles;
					uint32_t ptr;
					uint32_t data;
					Read32(0x120, ptr, cycles);
					ptr += 7 * 4 * (GetRegister(4) & 0xffff);
					Read32(ptr, data, cycles);
					std::cout << "~[KERNEL] event 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] class 0x" << std::hex << data << '\n';
					Read32(ptr + 0x4, data, cycles);
					std::cout << "~[KERNEL] flag 0x" << std::hex << data << '\n';
					Read32(ptr + 0x8, data, cycles);
					std::cout << "~[KERNEL] specs 0x" << std::hex << data << '\n';
					Read32(ptr + 0xc, data, cycles);
					std::cout << "~[KERNEL] mode 0x" << std::hex << data << '\n';
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
					std::cout << "event 0x" << std::hex << get(4) << '\n';
					std::cout << "class 0x" << std::hex << data << '\n';
					read32(ptr + 0x4, data, cycles);
					std::cout << "flag 0x" << std::hex << data << '\n';
					read32(ptr + 0x8, data, cycles);
					std::cout << "specs 0x" << std::hex << data << '\n';
					read32(ptr + 0xc, data, cycles);
					std::cout << "mode 0x" << std::hex << data << '\n';
				}
			}*/

			if (call == 0x3d) {
				std::stringstream ss;
				ss << (char)GetRegister(4);
				p_debugger->log(ss.str());
				std::cout << ss.str();
			}

			if (g_biosCallbacksLogged) {
				if ((call == 0x4e) || (call == 0x4f)) {
					std::cout << "~[KERNEL] port 0x" << std::hex << GetRegister(4) << '\n';
					std::cout << "~[KERNEL] sector 0x" << std::hex << GetRegister(5) << '\n';
					std::cout << "~[KERNEL] dst/src 0x" << std::hex << GetRegister(6) << '\n';
					g_emulationPaused = true;
				}

				if ((call == 0x5c) || (call == 0x5d)) {
					std::cout << "~[KERNEL] slot 0x" << std::hex << GetRegister(4) << '\n';
				}
			}

			break;
		case 'c':
			if (g_biosCallbacksLogged) {
				if (call > 0x20)
					call = 0x20;
				std::cout << "[KERNEL] " << pBus->biosChip.C[call].info << '\n';
			}

			break;
		}

		biosCallFlag.reset();
	}
}

std::string Cpu::getInstructionStr(const uint32_t& opcode, const uint32_t& pc, const bool& b) {
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

std::string Cpu::getDecodedInstructionStr(const uint32_t& opcode, const uint32_t& pc) {
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
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << (int16_t)(opcode & 0x0000ffff) + GetRegister(opcode << 6 >> 27) << "]"
		<< std::dec << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);

	else if (prim_opcode >= 0x28 && prim_opcode <= 0x2f) // store rt,[rs+imm]
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << (int16_t)(opcode & 0x0000ffff) + GetRegister(opcode << 6 >> 27) << "]"
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

void Cpu::sideLoad() {
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

	SetRegister(28, (((uint32_t)buffer[0x17]) << 24) | (((uint32_t)buffer[0x16]) << 16)
		| (((uint32_t)buffer[0x15]) << 8) | (uint32_t)buffer[0x14]);

	uint32_t ramAddr = (((uint32_t)buffer[0x1b]) << 24) | (((uint32_t)buffer[0x1a]) << 16)
		| (((uint32_t)buffer[0x19]) << 8) | (uint32_t)buffer[0x18];

	uint32_t size = (((uint32_t)buffer[0x1f]) << 24) | (((uint32_t)buffer[0x1e]) << 16)
		| (((uint32_t)buffer[0x1d]) << 8) | (uint32_t)buffer[0x1c];

	uint32_t initialSpFp = (((uint32_t)buffer[0x33]) << 24) | (((uint32_t)buffer[0x32]) << 16)
		| (((uint32_t)buffer[0x31]) << 8) | (uint32_t)buffer[0x30];
	if (initialSpFp) {
		SetRegister(29, initialSpFp);
		SetRegister(30, initialSpFp);
	}

	uint32_t initialSpFpOffset = (((uint32_t)buffer[0x37]) << 24) | (((uint32_t)buffer[0x36]) << 16)
		| (((uint32_t)buffer[0x35]) << 8) | (uint32_t)buffer[0x34];
	if (initialSpFpOffset) {
		std::cout << "Unhandled Initial SP/R29 & FP/R30 Offs in PSX.EXE (sideload)\n";
		g_emulationPaused = true;
	}

	uint8_t cycles;
	for (uint32_t i = 0; i < size; i++) {
		Write8(ramAddr + i, buffer[i + 0x800], cycles);
	}
}

bool Cpu::checkForCopOpcode(const uint32_t& opcode) {
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
