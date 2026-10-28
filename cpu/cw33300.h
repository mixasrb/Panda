#pragma once

#include <cstdint>
#include <vector>
#include <sstream>
#include <optional>
#include <fstream>

#include "cp0.h"
#include "cp2.h"
#include "../defines/defines.h"

#include "../debug_utilities/debug_utilities.h"

class BusInterface;

class Cpu {
public:

	Cpu();

	void Read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles,const bool& debug = false);
	void Write32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);

	void Read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles);
	void Write16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void Read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles);
	void Write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	void Clock();

	CP0 cp0;
	CP2 cp2;

	BusInterface* pBus;

	uint32_t pc;
	uint32_t pc_old = 0xbfc00000;

	void DecodeExecuteOpcode(const uint32_t& op);

	uint32_t opcodes[2];
	uint32_t opcode_pc_0;
	uint32_t opcode_pc_1;
	uint32_t opcode_pc;
	bool isCurrentOpcodeBranch = false;

	uint32_t opcode;
	uint8_t instructionFetchClocks = 0;
	uint8_t instructionFetchClocks1 = 0;
	uint8_t clocks = 0;
	uint8_t cycle = 0;
	uint8_t writeClocks = 0;
	uint8_t memoryClocks = 0;
	uint8_t gteClocks = 0;

	uint32_t GetRegister(const uint8_t& reg);
	void SetRegister(const uint8_t& reg, const uint32_t& v, const bool& loadOpcode = false);

	uint32_t r[32] = {};
	uint32_t hi;
	uint32_t lo;

	//inctruction
private:
	struct Instruction {
		std::string name;
	};

	std::vector <Instruction> primLookup;
	std::vector <Instruction> secLookup;

	struct ShiftImm {
		void (Cpu::* instr)(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	};

	ShiftImm shift_imm[4];
	void SLL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void XXX(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void SRL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void SRA(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);

	struct ShiftReg {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	};

	ShiftReg shift_reg[4];
	void SLLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	//   XXX
	void SRLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SRAV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);

	void JR(const uint8_t& rs);
	void JALR(const uint8_t& rs, const uint8_t& rd);

	struct SysBrk {
		void (Cpu::* instr)(const uint32_t& imm);
	};

	SysBrk sys_brk[2];
	void SYSCALL(const uint32_t& imm);
	void BREAK(const uint32_t& imm);

	struct MulDiv {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt);
	};

	MulDiv mul_div[4];
	void MULT(const uint8_t& rs, const uint8_t& rt);
	void MULTU(const uint8_t& rs, const uint8_t& rt);
	void DIV(const uint8_t& rs, const uint8_t& rt);
	void DIVU(const uint8_t& rs, const uint8_t& rt);

	struct MfhiMflo {
		void (Cpu::* instr)(const uint8_t& rd);
	};

	MfhiMflo mfhi_mflo[2];
	void MFHI(const uint8_t& rd);
	void MFLO(const uint8_t& rd);

	struct MthiMtlo {
		void (Cpu::* instr)(const uint8_t& rs);
	};

	MthiMtlo mthi_mtlo[2];
	void MTHI(const uint8_t& rs);
	void MTLO(const uint8_t& rs);

	struct AluReg {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	};

	AluReg alu_reg[16];
	void ADD(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void ADDU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SUB(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SUBU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void AND(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void OR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void XOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void NOR(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);

	void XXX2(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SLT(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SLTU(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);

	struct BcondZ {
		void (Cpu::* instr)(const uint8_t& rs, const uint16_t& imm);
	};

	BcondZ b_cond_z[4];
	void BLTZ(const uint8_t& rs, const uint16_t& imm);
	void BGEZ(const uint8_t& rs, const uint16_t& imm);
	void BLTZAL(const uint8_t& rs, const uint16_t& imm);
	void BGEZAL(const uint8_t& rs, const uint16_t& imm);

	struct JJal {
		void (Cpu::* instr)(const uint32_t& imm);
	};

	JJal j_jal[2];
	void J(const uint32_t& imm);
	void JAL(const uint32_t& imm);

	struct BeqBne {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);;
	};

	BeqBne beq_bne[2];
	void BEQ(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void BNE(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);

	struct BlezBgtz {
		void (Cpu::* instr)(const uint8_t& rs, const uint16_t& imm);
	};

	BlezBgtz blez_bgtz[2];
	void BLEZ(const uint8_t& rs, const uint16_t& imm);
	void BGTZ(const uint8_t& rs, const uint16_t& imm);

	struct AluImm {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	};

	AluImm alu_imm[7];
	void ADDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ADDIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void SLTI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void SLTIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ANDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void XORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);

	void LUI(const uint8_t& rt, const uint16_t& imm);

	struct LoadRt {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	};

	LoadRt load_rt[8];
	void LB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LBU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LHU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);;
	void XXX2(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);

	struct StoreRt {
		void (Cpu::* instr)(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	};

	StoreRt store_rt[8];
	void SB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void SH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void SWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void SW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void XXX1(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void SWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);

public:
	struct write_t {
		uint32_t addr;
		uint32_t data;
		uint32_t writeClocks;
		uint8_t bitness;
		bool bWrite;
		bool bExecute;
		uint32_t old;
	}cycle0, cycle1;

	bool gteCommandFlag = false;
	bool gteCommandFlagTemp = false;
	bool bInterrupt = false;


	//debug
public:
	DebugUtilities* p_debugger;

	uint32_t oldCall;

	bool isInstructionExecuted;
	uint8_t i = 0;
	uint8_t pipelineCycle = 0;

	void setBiosCallFlag();
	void printBiosCall();
	std::optional<char> biosCallFlag;

	std::string getInstructionStr(const uint32_t& opcode, const uint32_t& pc = 0, const bool& b = false);
	std::string getDecodedInstructionStr(const uint32_t& opcode, const uint32_t& pc);


	std::vector<std::string> executedOpcodes;

	void sideLoad();

	bool checkForCopOpcode(const uint32_t& opcode);
};