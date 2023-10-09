#pragma once

#include <cstdint>
#include <vector>
#include <bit>

#include "../debug_utilities/debug_utilities.h"

class cw33300;

class cp2 {
public:
	cp2();
	cw33300* pCpu;

	union parts_t {
		struct {
			uint8_t l, h1, h2, h3;
		}b;
		struct {
			int8_t l, h1, h2, h3;
		}sb;
		struct {
			uint16_t l, h;
		}hw;
		struct {
			int16_t l, h;
		}shw;
		uint32_t w;
		int32_t sw;
	};

	union gteCP2Ctrl_t {
		parts_t p[32];
	};

	union gteCP2Data_t {
		parts_t p[32];
	};

	struct gteRegisters_t {
		gteCP2Ctrl_t cp2Ctrl;
		gteCP2Data_t cp2Data;
	}gteRegisters;

	static uint32_t countLeadingBits(uint32_t value) {
		if (value & 0x80000000)
			value = ~value;
		return std::countl_zero<uint32_t>(value);
	}


	static int64_t gteSR(int64_t a, int sf) { return sf ? a >> 12 : a; }
	static int64_t gteSL(int64_t a, int sf) { return sf ? a << 12 : a; }

	static uint32_t countLeadingZeros16(uint16_t a) { return std::countl_zero<uint16_t>(a); }
	uint32_t gteDevide(uint16_t numerator, uint16_t denominator);


	uint32_t saturate(int32_t value, int32_t min, int32_t max, uint32_t flag);

	void MFC2(uint8_t rt, uint8_t rd);
	void CFC2(uint8_t rt, uint8_t rd);
	void MTC2(uint8_t rt, uint8_t rd);
	void CTC2(uint8_t rt, uint8_t rd);
	void BC2F(uint16_t imm);
	void BC2T(uint16_t imm);
	void COP2(uint32_t imm);
	void LWC2(uint8_t rs, uint8_t rt, int16_t imm);
	void SWC2(uint8_t rs, uint8_t rt, int16_t imm);

	struct opcode_t {
		std::string name;
		void (cp2::* opcode)(uint32_t imm);
		uint8_t clocks;
	};

	std::vector<opcode_t> opcodeLookup;

	class int44 {
		int64_t m_value;
		bool m_positiveOverflow;
		bool m_negativeOverflow;
	public:
		int44(int64_t value) :m_value(value), m_positiveOverflow(value > 0x7ffffffffff), m_negativeOverflow(value < -0x80000000000) {};
		int44(int64_t value, bool positiveOverflow, bool negativeOverflow) :m_value(value), m_positiveOverflow(positiveOverflow), m_negativeOverflow(negativeOverflow) {};
		int44 operator+(int64_t rhs) {
			int64_t value = ((m_value + rhs) << 20) >> 20;
			return int44(value, m_positiveOverflow || (value < 0 && m_value >= 0 && rhs >= 0)
				, m_negativeOverflow || (value >= 0 && m_value < 0 && rhs < 0));
		}

		bool positiveOverflow() { return m_positiveOverflow; };
		bool negativeOverflow() { return m_negativeOverflow; };
		int64_t value() { return m_value; };
	};

	int32_t bounds(int44 value, uint32_t minFlag, uint32_t maxFlag);

	int sf;
	int64_t s_mac0;
	int64_t s_mac3;

	int32_t A1(int44 a);
	int32_t A2(int44 a);
	int32_t A3(int44 a);
	int32_t Lm_B1(int32_t a, int lm);
	int32_t Lm_B2(int32_t a, int lm);
	int32_t Lm_B3(int32_t a, int lm);
	int32_t Lm_B3_RTP(int64_t a, int sf, int lm);
	int32_t Lm_C1(int32_t a);
	int32_t Lm_C2(int32_t a);
	int32_t Lm_C3(int32_t a);
	int64_t Lm_D(int64_t a, int sf);
	int64_t F(int64_t a);
	int32_t Lm_G1(int64_t a);
	int32_t Lm_G2(int64_t a);
	int32_t Lm_H(int64_t a);

	void XXXXX(uint32_t opcode);
	void RTPS(uint32_t opcode);
	void RTPT(uint32_t opcode);
	void MVMVA(uint32_t opcode);
	void DCPL(uint32_t opcode);
	void DPCS(uint32_t opcode);
	void DPCT(uint32_t opcode);
	void INTPL(uint32_t opcode);
	void SQR(uint32_t opcode);
	void NCS(uint32_t opcode);
	void NCT(uint32_t opcode);
	void NCDS(uint32_t opcode);
	void NCDT(uint32_t opcode);
	void NCCS(uint32_t opcode);
	void NCCT(uint32_t opcode);
	void CDP(uint32_t opcode);
	void CC(uint32_t opcode);
	void NCLIP(uint32_t opcode);
	void AVSZ3(uint32_t opcode);
	void AVSZ4(uint32_t opcode);
	void OP(uint32_t opcode);
	void GPF(uint32_t opcode);
	void GPL(uint32_t opcode);

	//debug
	debugUtilities* pDebugger;
};