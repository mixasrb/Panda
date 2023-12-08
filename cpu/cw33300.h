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

class busInterface_t;

class cpu_t {
public:

	cpu_t();

	void read32(const uint32_t& addr, uint32_t& data, uint8_t& cycles, bool debug = false);
	void write32(const uint32_t& addr, const uint32_t& data, uint8_t& cycles);

	void read16(const uint32_t& addr, uint16_t& data, uint8_t& cycles);
	void write16(const uint32_t& addr, const uint16_t& data, uint8_t& cycles);

	void read8(const uint32_t& addr, uint8_t& data, uint8_t& cycles);
	void write8(const uint32_t& addr, const uint8_t& data, uint8_t& cycles);

	void reset();
	void clock();

	cp0_t cp0;
	cp2_t cp2;

	busInterface_t* pBus;

	uint32_t pc;
	uint32_t pc_old = 0xbfc00000;

	void decodeExecuteOpcode(const uint32_t& op);

	uint32_t opcodes[2];
	uint32_t opcode_pc_0;
	uint32_t opcode_pc_1;
	uint32_t opcode_pc;
	bool isCurrentOpcodeBranch = false;

	struct iCache_t {
		uint32_t pc;
		uint32_t opcode;
	};

	iCache_t iCache[16];
	bool iCacheEnabled = true;
	uint16_t iCacheCounter = 0;


	bool isPipelineFull = false;
	uint32_t opcode;
	uint8_t instructionFetchClocks = 0;
	uint8_t instructionFetchClocks1 = 0;
	uint8_t clocks = 0;
	uint8_t cycle = 0;
	uint8_t writeClocks = 0;
	uint8_t memoryClocks = 0;
	uint8_t gteClocks = 0;

	uint32_t get(const uint8_t& reg);
	void set(const uint8_t& reg, const uint32_t& v, const bool loadOpcode = false);

	uint32_t r[32] = {};
	uint32_t hi;
	uint32_t lo;

	//inctruction
private:
	struct instruction {
		std::string name;
	};

	std::vector <instruction> primLookup;
	std::vector <instruction> secLookup;

	struct ShiftImm {
		void (cpu_t::* instr)(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	};

	std::vector <ShiftImm> shift_imm;

	void SLL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void XXX(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void SRL(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);
	void SRA(const uint8_t& rt, const uint8_t& rd, const uint8_t& imm);

	struct ShiftReg {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	};

	std::vector <ShiftReg> shift_reg;

	void SLLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	//   XXX
	void SRLV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	void SRAV(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);

	void JR(const uint8_t& rs);
	void JALR(const uint8_t& rs, const uint8_t& rd);

	struct SysBrk {
		void (cpu_t::* instr)(const uint32_t& imm);
	};

	std::vector <SysBrk> sys_brk;
	void SYSCALL(const uint32_t& imm);
	void BREAK(const uint32_t& imm);

	struct MulDiv {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt);
	};

	std::vector <MulDiv>mul_div;

	void MULT(const uint8_t& rs, const uint8_t& rt);
	void MULTU(const uint8_t& rs, const uint8_t& rt);
	void DIV(const uint8_t& rs, const uint8_t& rt);
	void DIVU(const uint8_t& rs, const uint8_t& rt);

	struct MfhiMflo {
		void (cpu_t::* instr)(const uint8_t& rd);
	};

	std::vector <MfhiMflo>mfhi_mflo;

	void MFHI(const uint8_t& rd);
	void MFLO(const uint8_t& rd);

	struct MthiMtlo {
		void (cpu_t::* instr)(const uint8_t& rs);
	};

	std::vector <MthiMtlo>mthi_mtlo;

	void MTHI(const uint8_t& rs);
	void MTLO(const uint8_t& rs);

	struct AluReg {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const uint8_t& rd);
	};

	std::vector <AluReg> alu_reg;

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
		void (cpu_t::* instr)(const uint8_t& rs, const uint16_t& imm);
	};

	std::vector<BcondZ> b_cond_z;

	void BLTZ(const uint8_t& rs, const uint16_t& imm);
	void BGEZ(const uint8_t& rs, const uint16_t& imm);
	void BLTZAL(const uint8_t& rs, const uint16_t& imm);
	void BGEZAL(const uint8_t& rs, const uint16_t& imm);

	struct JJal {
		void (cpu_t::* instr)(const uint32_t& imm);
	};

	std::vector <JJal> j_jal;

	void J(const uint32_t& imm);
	void JAL(const uint32_t& imm);

	struct BeqBne {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);;
	};

	std::vector <BeqBne> beq_bne;

	void BEQ(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void BNE(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);

	struct BlezBgtz {
		void (cpu_t::* instr)(const uint8_t& rs, const uint16_t& imm);
	};

	std::vector <BlezBgtz> blez_bgtz;

	void BLEZ(const uint8_t& rs, const uint16_t& imm);
	void BGTZ(const uint8_t& rs, const uint16_t& imm);

	struct AluImm {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	};

	std::vector <AluImm> alu_imm;

	void ADDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ADDIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void SLTI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void SLTIU(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ANDI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void ORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);
	void XORI(const uint8_t& rs, const uint8_t& rt, const uint16_t& imm);

	void LUI(const uint8_t& rt, const uint16_t& imm);

	struct LoadRt {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	};

	std::vector <LoadRt> load_rt;

	void LB(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LH(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LWL(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LW(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LBU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LHU(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	void LWR(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);;
	void XXX2(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);

	struct StoreRt {
		void (cpu_t::* instr)(const uint8_t& rs, const uint8_t& rt, const int16_t& imm);
	};

	std::vector <StoreRt> store_rt;

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
	bool irqFlag = false;


	//debug
public:
	debugUtilities* p_debugger;

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
	std::vector<std::string> loggedOpcodes;

	void sideLoad();

	bool checkForCopOpcode(const uint32_t& opcode);
};