#include "cw33300.h"

extern bool isEmulationPaused;
extern bool areExecutedCpuInstructionsLogged;
extern bool areGTEInstructionsLogged;
extern bool isButtonPressed;

extern const char* g_sideloadPath;
extern bool bSideload;

cw33300::cw33300() {
	//connect cpu0
	cp0.pCpuSoC = this;

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
		{&cw33300::SLL}, {&cw33300::XXX}, {&cw33300::SRL}, {&cw33300::SRA}
	};

	shift_reg.resize(4);
	shift_reg = {
		{&cw33300::SLLV}, {&cw33300::XXX2}, {&cw33300::SRLV}, {&cw33300::SRAV}
	};

	sys_brk.resize(2);
	sys_brk = {
		{&cw33300::SYSCALL}, { &cw33300::BREAK }
	};

	alu_reg.resize(16);
	alu_reg = {
		{&cw33300::ADD}, {&cw33300::ADDU}, {&cw33300::SUB}, {&cw33300::SUBU}, {&cw33300::AND}, {&cw33300::OR}, {&cw33300::XOR}, {&cw33300::NOR},
		{&cw33300::XXX2}, {&cw33300::XXX2}, {&cw33300::SLT}, {&cw33300::SLTU}, {&cw33300::XXX2}, {&cw33300::XXX2}, {&cw33300::XXX2}, {&cw33300::XXX2}
	};

	mfhi_mflo.resize(2);
	mfhi_mflo = {
		{&cw33300::MFHI}, {&cw33300::MFLO}
	};

	mthi_mtlo.resize(2);
	mthi_mtlo = {
		{&cw33300::MTHI}, {&cw33300::MTLO}
	};

	mul_div.resize(4);
	mul_div = {
		{&cw33300::MULT}, {&cw33300::MULTU}, {&cw33300::DIV}, {&cw33300::DIVU}
	};

	b_cond_z.resize(4);
	b_cond_z = {
		{&cw33300::BLTZ}, {&cw33300::BGEZ}, {&cw33300::BLTZAL}, {&cw33300::BGEZAL}
	};

	j_jal.resize(2);
	j_jal = {
		{&cw33300::J}, {&cw33300::JAL}
	};

	beq_bne.resize(2);
	beq_bne = {
		{&cw33300::BEQ}, {&cw33300::BNE}
	};

	blez_bgtz.resize(2);
	blez_bgtz = {
		{&cw33300::BLEZ}, {&cw33300::BGTZ}
	};

	alu_imm.resize(7);
	alu_imm = {
		{&cw33300::ADDI}, {&cw33300::ADDIU}, {&cw33300::SLTI}, {&cw33300::SLTIU}, {&cw33300::ANDI}, {&cw33300::ORI}, {&cw33300::XORI}
	};

	load_rt.resize(8);
	load_rt = {
		{&cw33300::LB}, {&cw33300::LH}, {&cw33300::LWL}, {&cw33300::LW}, {&cw33300::LBU}, {&cw33300::LHU}, {&cw33300::LWR}, {&cw33300::XXX2}
	};

	store_rt.resize(8);
	store_rt = {
		{&cw33300::SB}, {&cw33300::SH}, {&cw33300::SWL}, {&cw33300::SW}, {&cw33300::XXX1}, {&cw33300::XXX1}, {&cw33300::SWR}, {&cw33300::XXX1}
	};

	//debug
	executedOpcodes.resize(32);
}

uint32_t cw33300::get(const uint8_t& reg) {
	return r[reg];
}

void cw33300::set(const uint8_t& reg, const uint32_t& value) {
	if (reg != 0)
		r[reg] = value;
}

void cw33300::decodeExecute(const uint32_t& opcode) {
	//debug 
	/*executedOpcodes.pop_back();
	std::string s = getInstructionStr(opcode, pc_old);
	executedOpcodes.insert(executedOpcodes.begin(), s);*/
	/*if (s.find("N/A") != std::string::npos)
		isEmulationPaused = true;*/

		//debug
	if (areExecutedCpuInstructionsLogged)
		//p_debugger->log(getInstructionStr(opcode, pc_old, false) + "\n");
		std::cout << getInstructionStr(opcode, pc_old, false) + "\n";

	isCurrentOpcodeBranch = false;
	uint8_t prim_opcode = opcode >> 26;

	if (prim_opcode == 0) { // special
		uint8_t sec_opcode = opcode << 26 >> 26;

		if (sec_opcode >= 0 && sec_opcode <= 0x3) //shift-imm
			(this->*shift_imm[sec_opcode].instr)(
				opcode << 11 >> 27,
				opcode << 16 >> 27, opcode << 21 >> 27);

		else if (sec_opcode >= 0x4 && sec_opcode < 0x8) //shift-reg
			(this->*shift_reg[sec_opcode - 0x04].instr)(
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
			(this->*sys_brk[sec_opcode - 0xc].instr)(opcode >> 6);

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
			(this->*alu_reg[sec_opcode - 0x20].instr)(
				opcode << 6 >> 27,
				opcode << 11 >> 27, opcode << 16 >> 27);

		else {
			std::cout << "[CPU] EMULATION PAUSED! unhandled decode of opcode " << secLookup[opcode << 26 >> 26].name << " op 0x" << std::hex << opcode << " pc 0x" << std::hex << pc_old - 4 << std::endl;
			isEmulationPaused = true;
		}
	}

	else if (prim_opcode == 0x1) { //BcondZ
		isCurrentOpcodeBranch = true;
		switch (opcode << 11 >> 27) {
		case 0x10:
			(this->*b_cond_z[2].instr)(
				opcode << 6 >> 27, opcode & 0x0000ffff); //bltzal
			break;
		case 0x11:
			(this->*b_cond_z[3].instr)(
				opcode << 6 >> 27, opcode & 0x0000ffff); //bgezal
			break;
		default:
			if (!((opcode << 11 >> 27) & 0x1))
				(this->*b_cond_z[0].instr)(
					opcode << 6 >> 27, opcode & 0x0000ffff); //bltz
			else
				(this->*b_cond_z[1].instr)(
					opcode << 6 >> 27, opcode & 0x0000ffff); //bgez
			break;
		}
	}

	else if (prim_opcode >= 0x2 && prim_opcode <= 0x3) { //j_jal
		isCurrentOpcodeBranch = true;
		(this->*j_jal[prim_opcode - 0x2].instr)(
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
		(this->*blez_bgtz[prim_opcode - 0x06].instr)(
			opcode << 6 >> 27, opcode & 0x0000ffff);
	}

	else if (prim_opcode >= 0x08 && prim_opcode < 0x0f) //alu-imm
		(this->*alu_imm[prim_opcode - 0x08].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode & 0x0000ffff);

	else if (prim_opcode == 0x0f) //lui-imm
		LUI(opcode << 11 >> 27, opcode & 0x0000ffff);

	else if (prim_opcode >= 0x20 && prim_opcode <= 0x27) //load rt,[rs+imm]
		(this->*load_rt[prim_opcode - 0x20].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode & 0x0000ffff);

	else if (prim_opcode >= 0x28 && prim_opcode <= 0x2f) //store rt,[rs+imm]
		(this->*store_rt[prim_opcode - 0x28].instr)(
			opcode << 6 >> 27,
			opcode << 11 >> 27, opcode & 0x0000ffff);

	else if (prim_opcode & 0x10) { //coprocessor

		//LWCn/SWCn
		if (prim_opcode & 0x20) {
			if (areGTEInstructionsLogged) {
				std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop opcode " << primLookup[prim_opcode].name << " op " << std::hex << opcode << " pc " << std::hex << pc_old - 4 << std::endl;
				isEmulationPaused = true;
			}
		}
		//other
		else {
			switch (prim_opcode & 0x03) {
				//cp0
			case 0:
				switch (opcode << 6 >> 27)
				{
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
					cp0.CTC0((opcode << 11) >> 27, opcode << 16 >> 27);
					break;
				case 0x8:
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop 0 opcode BCnF/BCnT 0x" << primLookup[prim_opcode].name << " op " << std::hex << opcode << " pc " << std::hex << pc_old - 4 << std::endl;
					isEmulationPaused = true;
					break;
				case 0x10:
					cp0.RFE();
					break;
				default:
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop 0 opcode 0x" << primLookup[prim_opcode].name << " op " << std::hex << opcode << " pc " << std::hex << pc_old - 4 << std::endl;
					isEmulationPaused = true;
					break;
				}
				break;
				//cop2
			case 2:
				if (areGTEInstructionsLogged) {
					std::cout << "[CPU] EMULATION PAUSED! unhandled decode of cop 2 opcode " << primLookup[prim_opcode].name << " op " << std::hex << opcode << " pc " << std::hex << pc_old - 4 << std::endl;
					isEmulationPaused = true;
				}
				break;
			default:
				std::cout << "[CPU] EMULATION PAUSED! unhandled cop " << (uint16_t)(prim_opcode & 0x03) << " op 0x"
					<< std::hex << opcode << std::endl;
				isEmulationPaused = true;
				break;
			}
		}
	}
	else {
		std::cout << "[CPU] EMULATION PAUSED! unhandled decode of opcode " << primLookup[prim_opcode].name << " op " << std::hex << opcode << " pc " << std::hex << pc_old - 4 << std::endl;
		isEmulationPaused = true;
	}

}

void cw33300::reset() {
	isPipelineFull = false;
	cycle = 0;
	pc = 0xbfc00000;
}

void cw33300::clock() {

	//SIDELOAD debug
	if ((pc == 0x80030000) && (bSideload))
		sideLoad();

	//debug
	isInstructionExecuted = false;

	/*if (pc & 0x3) {
		cp0.exceptionHandler(_IBE);
		return;
	}*/

	if (clocks == 0) {

		cp0.checkForInterrupts();

		switch (cycle) {
		case 0:
			cycle = 1;
		label0:
			//debug
			setBiosCallFlag();
			//fetch
			read32(pc, opcodes[0], instructionFetchClocks0);

			opcode_pc_0 = pc;
			opcode_pc = opcode_pc_1;

			opcode = opcodes[1];

			//debug
			pc_old = pc;

			pc += 4;

			if (!isPipelineFull)
				goto label1;

			memoryClocks = 0;
			decodeExecute(opcodes[1]);
			clocks = instructionFetchClocks1 + memoryClocks;

			//debug
			printBiosCall();


			//debug
			isInstructionExecuted = true;

			break;

		case 1:
			if (!isPipelineFull)
				goto label0;

		label1:
			cycle = 0;
			isPipelineFull = true;

			//debug
			setBiosCallFlag();
			//fetch
			read32(pc, opcodes[1], instructionFetchClocks1);

			opcode_pc_1 = pc;
			opcode_pc = opcode_pc_0;

			opcode = opcodes[0];

			//debug
			pc_old = pc;

			pc += 4;

			memoryClocks = 0;
			decodeExecute(opcodes[0]);
			clocks = instructionFetchClocks0 + memoryClocks;

			//debug
			printBiosCall();

			//debug
			isInstructionExecuted = true;

			break;
		}
	}

	clocks--;
}

void cw33300::read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles) {
	pBus->cpuRead32(addr, data, cycles);
}

void cw33300::write32(const uint32_t& addr, const uint32_t& data, uint8_t& wr_cycles) {
	pBus->cpuWrite32(addr, data, wr_cycles);
}

void cw33300::read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles) {
	pBus->cpuRead16(addr, data, cycles);
}

void cw33300::write16(const uint32_t& addr, const uint16_t& data, uint8_t& wr_cycles) {
	pBus->cpuWrite16(addr, data, wr_cycles);
}

void cw33300::read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles) {
	pBus->cpuRead8(addr, data, cycles);
}

void cw33300::write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles) {
	pBus->cpuWrite8(addr, data, writeClocks);
}

//instructions

void cw33300::SLL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	set(rd, get(rt) << imm);
}

void cw33300::XXX(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode:" << secLookup[opcode << 26 >> 26].name << "op:" << std::hex << opcode << std::endl;
	isEmulationPaused = true;
}

void cw33300::SRL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	set(rd, get(rt) >> imm);
}

void cw33300::SRA(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm) {
	int32_t temp = get(rt);
	temp >>= imm;
	set(rd, temp);
}

void cw33300::SLLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rt) << (get(rs) & 0x1f));
}

void cw33300::SRLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rt) >> (get(rs) & 0x1f));
}

void cw33300::SRAV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	int32_t temp = get(rt);
	temp >>= (get(rs) & 0x1f);
	set(rd, temp);
}

void cw33300::JR(const uint8_t& rs) {
	if (get(rs) & 0x3) {
		cp0.exceptionHandler(_ADEL, get(rs));
	}
	else
		pc = get(rs);
}

void cw33300::JALR(const uint8_t& rs, const uint8_t& rd) {
	uint32_t tempAddr = get(rs);
	set(rd, pc);
	if (tempAddr & 0x3) {
		cp0.exceptionHandler(_ADEL, tempAddr);
	}
	else
		pc = tempAddr;
}

void cw33300::SYSCALL(const uint32_t& imm) {
	cp0.exceptionHandler(_SYSCALL);
}

void cw33300::BREAK(const uint32_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode " << secLookup[opcode << 26 >> 26].name << "op:" << std::hex << opcode << std::endl;
	//cp0.exceptionHandler(_BP);
}

void cw33300::MULT(const uint8_t& rs, const uint8_t& rt) {
	int64_t operandA = (int32_t)get(rs);
	int64_t operandB = (int32_t)get(rt);
	int64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
	/////
	/*int64_t temp = (int64_t)get(rs) * (int64_t)get(rt);
	hi = (int32_t)(temp >> 32);
	lo = (int32_t)(temp << 32 >> 32);*/
}

void cw33300::MULTU(const uint8_t& rs, const uint8_t& rt) {
	uint64_t operandA = (uint32_t)get(rs);
	uint64_t operandB = (uint32_t)get(rt);
	uint64_t temp = operandA * operandB;
	hi = temp >> 32;
	lo = temp << 32 >> 32;
}

void cw33300::DIV(const uint8_t& rs, const uint8_t& rt) {
	int32_t operandA = get(rs);
	int32_t operandB = get(rt);

	if ((operandA == 0x80000000) && (operandB == 0xffffffff)) {
		lo = 0x80000000;
		hi = 0x00000000;
		return;
	}

	if (operandB == 0) {
		if (operandA < 0) {
			lo = 0x00000001;
			hi = operandA;
		}
		else {
			lo = 0xffffffff;
			hi = operandA;
		}
	}

	if (operandB) {
		lo = operandA / operandB;
		hi = operandA % operandB;
	}
	else
		std::cout << "~[CPU] DIV divider = 0, potential problem \n";
}

void cw33300::DIVU(const uint8_t& rs, const uint8_t& rt) {
	uint32_t operand_a = get(rs);
	uint32_t operand_b = get(rt);

	if (operand_b == 0) {
		lo = 0xffffffff;
		hi = operand_a;
	}

	if (operand_b) {
		lo = operand_a / operand_b;
		hi = operand_a % operand_b;
	}
	else
		std::cout << "~[CPU] DIVU divider = 0, potential problem \n";
}

void cw33300::MFHI(const uint8_t& rd) {
	set(rd, hi);
}

void cw33300::MFLO(const uint8_t& rd) {
	set(rd, lo);
}

void cw33300::MTHI(const uint8_t& rs) {
	hi = get(rs);
}

void cw33300::MTLO(const uint8_t& rs) {
	lo = get(rs);
}

void cw33300::ADD(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = get(rs);
	uint32_t b = get(rt);
	uint32_t res = a + b;

	bool overflow = ((a ^ res) & (b ^ res)) >> 31;
	if (overflow)
		cp0.exceptionHandler(_OV);
	else
		set(rd, res);
}

void cw33300::ADDU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rs) + get(rt));
}

void cw33300::SUB(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	uint32_t a = get(rs);
	uint32_t b = get(rt);
	uint32_t res = a - b;

	if (((int32_t)b > 0 && (int32_t)a < INT32_MIN + (int32_t)b) ||
		((int32_t)b < 0 && (int32_t)a > INT32_MAX + (int32_t)b))
		cp0.exceptionHandler(_OV); // ???
	else
		set(rd, res);
}

void cw33300::SUBU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rs) - get(rt));
}

void cw33300::AND(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rs) & get(rt));
}

void cw33300::OR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rs) | get(rt));
}

void cw33300::XOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, get(rs) ^ get(rt));
}

void cw33300::NOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	set(rd, ~(get(rs) | get(rt)));
}

void cw33300::XXX2(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode:" << secLookup[opcode << 26 >> 26].name << "op:" << std::hex << opcode << std::endl;
	isEmulationPaused = true;
}

void cw33300::SLT(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	int32_t openand_a = get(rs);
	int32_t openand_b = get(rt);
	openand_a < openand_b ? set(rd, 1) : set(rd, 0);
}

void cw33300::SLTU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd) {
	get(rs) < get(rt) ? set(rd, 1) : set(rd, 0);
}

void cw33300::BLTZ(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	if (temp < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BGEZ(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	if (temp >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BLTZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	set(31, pc - 8 + 8);
	if (temp < 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BGEZAL(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	set(31, pc - 8 + 8);
	if (temp >= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::J(const uint32_t& imm) {
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void cw33300::JAL(const uint32_t& imm) {
	set(31, pc);
	pc = (pc - 8 & 0xf0000000) | (imm << 2);
}

void cw33300::BEQ(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (get(rs) == get(rt))
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BNE(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	if (get(rs) != get(rt))
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BLEZ(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	if (temp <= 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::BGTZ(const uint8_t& rs, const uint16_t& imm) {
	int32_t temp = get(rs);
	if (temp > 0)
		pc = pc + (int16_t)imm * 4 - 4;
}

void cw33300::ADDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	uint32_t a = get(rs);
	uint16_t b = imm;
	uint32_t temp = a + (int16_t)b;

	if (((int32_t)a > 0 && (int16_t)b > 0 && (int32_t)temp < 0) ||
		((int32_t)a < 0 && (int16_t)b < 0 && (int32_t)temp > 0))
		cp0.exceptionHandler(_OV);
	else
		set(rt, temp);
}

void cw33300::ADDIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	set(rt, get(rs) + (int16_t)imm);
}

void cw33300::SLTI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	(int32_t)get(rs) < (int32_t)((int16_t)imm) ? set(rt, 1) : set(rt, 0);
}

void cw33300::SLTIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	get(rs) < (int32_t)((int16_t)imm) ? set(rt, 1) : set(rt, 0);
}

void cw33300::ANDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	set(rt, get(rs) & imm);
}

void cw33300::ORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm)
{
	set(rt, (get(rs) | imm));
}

void cw33300::XORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm) {
	set(rt, (get(rs) ^ imm));
}

void cw33300::LUI(const uint8_t& rt, const uint16_t& imm) {
	set(rt, imm << 16);
}

void cw33300::LB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	uint8_t temp;
	read8(temp_addr, temp, memoryClocks);
	set(rt, (int32_t)((int8_t)temp));
}

void cw33300::LH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	if (temp_addr & 0x1) {
		cp0.exceptionHandler(_ADEL, temp_addr);
	}
	else {
		uint16_t temp;
		read16(temp_addr, temp, memoryClocks);
		set(rt, (int32_t)((int16_t)temp));
	}
}

void cw33300::LWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = get(rs) + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;
	switch (offset) {
	case 0:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) << 8 >> 8) | (data << 24));
		break;
	case 1:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) << 16 >> 16) | (data << 16));
		break;
	case 2:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) << 24 >> 24) | (data << 8));
		break;
	case 3:
		read32(temp_addr, data, memoryClocks);
		set(rt, data);
		break;
	}
}

void cw33300::LW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	if (temp_addr & 0x3) {
		cp0.exceptionHandler(_ADEL, temp_addr);
	}
	else {
		uint32_t temp;
		read32(temp_addr, temp, memoryClocks);
		set(rt, temp);
	}
}

void cw33300::LBU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	uint8_t temp;
	read8(temp_addr, temp, memoryClocks);
	set(rt, temp);
}

void cw33300::LHU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	if (temp_addr & 0x1) {
		cp0.exceptionHandler(_ADEL, temp_addr);
	}
	else {
		uint16_t temp;
		read16(temp_addr, temp, memoryClocks);
		set(rt, temp);
	}
}

void cw33300::LWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = get(rs) + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;
	switch (offset) {
	case 0:
		read32(temp_addr, data, memoryClocks);
		set(rt, data);
		break;
	case 1:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) >> 24 << 24) | (data >> 8));
		break;
	case 2:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) >> 16 << 16) | (data >> 16));
		break;
	case 3:
		read32(temp_addr, data, memoryClocks);
		set(rt, (get(rt) >> 8 << 8) | (data >> 24));
		break;
	}
}

void cw33300::XXX2(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode:" << primLookup[opcode >> 26].name << "op:" << std::hex << opcode << std::endl;
	isEmulationPaused = true;
}

void cw33300::SB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	write8(temp_addr, get(rt) & 0xff, writeClocks);
}

void cw33300::SH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	if (temp_addr & 0x1) {
		cp0.exceptionHandler(_ADES, temp_addr);
	}
	else
		write16(temp_addr, get(rt) & 0xffff, writeClocks);
}

void cw33300::SWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = get(rs) + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;
	switch (offset) {
	case 0:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) >> 24) | (data >> 8 << 8), writeClocks);
		break;
	case 1:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) >> 16) | (data >> 16 << 16), writeClocks);
		break;
	case 2:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) >> 8) | (data >> 24 << 24), writeClocks);
		break;
	case 3:
		write32(temp_addr, get(rt), writeClocks);
		break;
	}
}

void cw33300::SW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	uint32_t temp_addr = get(rs) + (int32_t)imm;
	if (temp_addr & 0x3) {
		cp0.exceptionHandler(_ADES, temp_addr);
	}
	else
		write32(temp_addr, get(rt), writeClocks);
}

void cw33300::XXX1(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	std::cout << "[CPU] EMULATION PAUSED! unhandled opcode:" << primLookup[opcode >> 26].name << "op:" << std::hex << opcode << std::endl;
	isEmulationPaused = true;
}

void cw33300::SWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm) {
	int32_t addr_offset = (int32_t)imm & 0xfffffffc;
	uint32_t temp_addr = get(rs) + addr_offset;
	uint32_t offset = (uint32_t)imm & 0x3;
	uint32_t data;
	switch (offset)
	{
	case 0:
		write32(temp_addr, get(rt), memoryClocks);
		break;
	case 1:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) << 8) | (data << 24 >> 24), writeClocks);
		break;
	case 2:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) << 16) | (data << 16 >> 16), writeClocks);
		break;
	case 3:
		read32(temp_addr, data, memoryClocks);
		write32(temp_addr, (get(rt) << 24) | (data << 8 >> 8), writeClocks);
		break;
	}
}

//debug

void cw33300::setBiosCallFlag() {
	if (pc == 0xa0) {
		biosCallFlag = 'a';
		//isEmulationPaused = true;
	}
	if (pc == 0xb0) {
		biosCallFlag = 'b';
		//isEmulationPaused = true;
	}
	if (pc == 0xc0) {
		biosCallFlag = 'c';
		//isEmulationPaused = true;
	}
}

void cw33300::printBiosCall() {
	uint32_t call = get(9);

	if (biosCallFlag.has_value())
		switch (biosCallFlag.value()) {
		case 'a':
			biosCallFlag.reset();
			if (call > 0xb5)
				call = 0xb5;
			std::cout << pBus->biosChip.A[call].info << std::endl;

			if (call == 0x3c) {
				std::stringstream ss;
				//std::cout << (char)get(4);
				ss << (char)get(4);
				p_debugger->log(ss.str());
			}

			break;
		case 'b':
			biosCallFlag.reset();
			if (call >= 0x5e && call <= 0xff)
				call = 0x5e;
			if (call > 0xff)
				call = 0x5f;
			if ((call != 0x0b) && (call != 0x17)/* && (call != 0x3d)*/)
				std::cout << pBus->biosChip.B[call].info << std::endl;

			if (call == 0x02) {
				//isEmulationPaused = true;
				std::cout << "t 0x" << std::hex << get(4) << std::endl;
				std::cout << "reload 0x" << std::hex << get(5) << std::endl;
				std::cout << "flags 0x" << std::hex << get(6) << std::endl;
			}

			if (call == 0x08) {
				std::cout << "class 0x" << std::hex << get(4) << std::endl;
				std::cout << "spec 0x" << std::hex << get(5) << std::endl;
				std::cout << "mode 0x" << std::hex << get(6) << std::endl;
				std::cout << "func 0x" << std::hex << get(7) << std::endl;
			}

			if ((call == 0x07) || (call == 0x20)) {
				std::cout << "event 0x" << std::hex << get(4) << std::endl;
				std::cout << "specs 0x" << std::hex << get(5) << std::endl;
			}

			if (/*(v == 0x0b) || */(call == 0x0c) || (call == 0x09)) {
				uint8_t cycles;
				uint32_t ptr;
				uint32_t data;
				read32(0x120, ptr, cycles);
				ptr += 7 * 4 * (get(4) & 0xffff);
				read32(ptr, data, cycles);
				std::cout << "event 0x" << std::hex << get(4) << std::endl;
				std::cout << "class 0x" << std::hex << data << std::endl;
				read32(ptr + 0x4, data, cycles);
				std::cout << "flag 0x" << std::hex << data << std::endl;
				read32(ptr + 0x8, data, cycles);
				std::cout << "specs 0x" << std::hex << data << std::endl;
				read32(ptr + 0xc, data, cycles);
				std::cout << "mode 0x" << std::hex << data << std::endl;
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
				//std::cout << (char)get(4);
				ss << (char)get(4);
				p_debugger->log(ss.str());
			}

			if (call == 0x4f) {
				std::cout << "port 0x" << std::hex << get(4) << std::endl;
				std::cout << "sector 0x" << std::hex << get(5) << std::endl;
				std::cout << "dst 0x" << std::hex << get(6) << std::endl;
				//isEmulationPaused = true;
			}

			break;
		case 'c':
			biosCallFlag.reset();
			if (call > 0x20)
				call = 0x20;
			std::cout << pBus->biosChip.C[call].info << std::endl;

			break;
		}
}

std::string cw33300::getInstructionStr(const uint32_t& opcode, const uint32_t& pc, const bool& b) {
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

std::string cw33300::getDecodedInstructionStr(const uint32_t& opcode, const uint32_t& pc) {
	uint8_t prim_opcode = opcode >> 26;

	std::string s = " ";
	std::stringstream ss;

	if (prim_opcode == 0) { // special

		uint8_t sec_opcode = opcode << 26 >> 26;

		if (sec_opcode >= 0 && sec_opcode <= 0x3) { // shift-imm
			ss << "rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27) << " imm5 0x" << std::hex << (opcode << 21 >> 27);
			s = ss.str();
		}

		else if (sec_opcode >= 0x4 && sec_opcode < 0x8) { //shift-reg
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27);
			s = ss.str();
		}

		else if (sec_opcode == 0x08) { // jr
			ss << "rs" << (opcode << 6 >> 27);
			s = ss.str();
		}

		else if (sec_opcode == 0x09) { // jalr
			ss << "rs" << (opcode << 6 >> 27) << " rd" << (opcode << 16 >> 27);
			s = ss.str();
		}

		else if (sec_opcode >= 0xc && sec_opcode <= 0xd) { //sys/brk
			ss << "comment" << (opcode << 6 >> 12);
			s = ss.str();
		}

		else if (sec_opcode == 0x10 || sec_opcode == 0x12) { //mfhi/mflo
			ss << "rd" << (opcode << 16 >> 27);
			s = ss.str();
		}

		else if (sec_opcode == 0x11 || sec_opcode == 0x13) { //mthi/mtlo
			ss << "rs" << (opcode << 6 >> 27);
			s = ss.str();
		}

		else if ((sec_opcode >> 2) == 0x06) { //mul/div
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27);
			s = ss.str();
		}

		else if (sec_opcode >= 0x20 && sec_opcode <= 0x2f) { // alu_reg
			ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " rd" << (opcode << 16 >> 27);
			s = ss.str();
		}
	}

	else if (prim_opcode == 0x1) { //BcondZ
		switch (opcode << 11 >> 27) {
		case 0x10: //bltzal
			ss << "BLTZAL rs" << (opcode << 6 >> 27) << " imm" << std::hex << (opcode & 0x0000ffff);
			s = ss.str();
			break;
		case 0x11: //bgezal
			ss << "BGEZAL rs" << (opcode << 6 >> 27) << " imm" << std::hex << (opcode & 0x0000ffff);
			s = ss.str();
			break;
		default:
			if (!((opcode << 11 >> 27) & 0x1)) { //bltz
				ss << "BLTZ rs" << (opcode << 6 >> 27) << " imm" << std::hex << (opcode & 0x0000ffff);
				s = ss.str();
			}
			else { //bgez
				ss << "BGEZ rs" << (opcode << 6 >> 27) << " imm" << std::hex << (opcode & 0x0000ffff);
				s = ss.str();
			}
			break;
		}
	}

	else if (prim_opcode >= 0x2 && prim_opcode <= 0x3) { // j_jal
		ss << "imm26 0x" << std::hex << (opcode << 6 >> 6);
		s = ss.str();
	}

	else if (prim_opcode == 0x04 || prim_opcode == 0x05) { // beq/bne
		ss << "{0x" << std::hex << pc + (int16_t)(opcode & 0x0000ffff) * 4 - 4 << "} "
			<< std::dec << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode == 0x06 || prim_opcode == 0x07) { // blez/bgtz
		ss << "rs" << (opcode << 6 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode >= 0x08 && prim_opcode < 0x0f) { // alu-imm
		ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode == 0x0f) { // lui-imm
		ss << "rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode >= 0x20 && prim_opcode <= 0x27) { // load rt,[rs+imm]
		int16_t imm16 = opcode & 0x0000ffff;
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << imm16 + get(opcode << 6 >> 27) << "]"
			<< std::dec << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode >= 0x28 && prim_opcode <= 0x2f) { // store rt,[rs+imm]
		//ss << "rs" << (opcode << 6 >> 27) << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		int16_t imm16 = opcode & 0x0000ffff;
		ss << "rs" << (opcode << 6 >> 27) << " [0x" << std::hex << imm16 + get(opcode << 6 >> 27) << "]"
			<< std::dec << " rt" << (opcode << 11 >> 27) << " imm16 0x" << std::hex << (opcode & 0x0000ffff);
		s = ss.str();
	}

	else if (prim_opcode & 0x10) { // coprocessor
		if (prim_opcode & 0x20) {
		}
		else {
			switch (prim_opcode & 0x03) {
			case 0:
				switch (opcode << 6 >> 27) {
				case 0x0:
					ss << "MFC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					s = ss.str();
					break;
				case 0x2:
					ss << "CFC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					s = ss.str();
					break;
				case 0x4:
					ss << "MTC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					s = ss.str();
					break;
				case 0x6:
					ss << "CTC0 rd" << (opcode << 11 >> 27) << " rt" << (opcode << 16 >> 27);
					s = ss.str();
					break;
				case 0x8:
					break;
				case 0x10:
					ss << "RFE";
					s = ss.str();
					break;
				default:
					break;
				}
				break;
			}
		}
	}
	else {
	}
	return s;
}

void cw33300::sideLoad() {
	bSideload = false;
	std::ifstream file(g_sideloadPath, std::ifstream::binary | std::ifstream::ate);
	if (!file.is_open())
		throw std::runtime_error("failed to open sideload!\n");

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

	set(29, (((uint32_t)buffer[0x33]) << 24) | (((uint32_t)buffer[0x32]) << 16)
		| (((uint32_t)buffer[0x31]) << 8) | (uint32_t)buffer[0x30]);

	set(30, (((uint32_t)buffer[0x33]) << 24) | (((uint32_t)buffer[0x32]) << 16)
		| (((uint32_t)buffer[0x31]) << 8) | (uint32_t)buffer[0x30]);

	uint8_t cycles;
	for (int i = 0; i < size; i++) {
		write8(ramAddr + i, buffer[i + 0x800], cycles);
	}
}
