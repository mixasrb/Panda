#pragma once

#include <cstdint>
#include <vector>


#include <bit>

#include "../debug_utilities/debug_utilities.h"

#define gteop(instruction) ((instruction)&0x1ffffff)


class cw33300;

class cp2 {
public:
	cp2();
	cw33300* pCpu;

	void MFC2(uint8_t rt, uint8_t rd);
	void CFC2(uint8_t rt, uint8_t rd);
	void MTC2(uint8_t rt, uint8_t rd);
	void CTC2(uint8_t rt, uint8_t rd);
	void BC2F(uint16_t imm);
	void BC2T(uint16_t imm);
	void COP2(uint32_t imm);
	void LWC2(uint8_t rs, uint8_t rt, int16_t imm);
	void SWC2(uint8_t rs, uint8_t rt, int16_t imm);

	struct command_t {
		std::string name;
		uint8_t(cp2::* command)(uint32_t imm);
		uint8_t clocks;
	};

	std::vector<command_t> commandLookup;

	union commandEncoding_t {
		uint32_t data;
		struct {
			uint32_t realOpcode : 6;
			uint32_t : 4;
			uint32_t lm : 1;
			uint32_t : 2;
			uint32_t translationVect : 2;
			uint32_t multVect : 2;
			uint32_t multMatrix : 2;
			uint32_t sf : 1;
			uint32_t fakeOpcode : 6;
			uint32_t : 6;
		};
	};

	std::vector<uint8_t> unrTable;

	uint8_t XXXXX(uint32_t command);
	uint8_t RTPS(uint32_t commandData);
	uint8_t NCLIP(uint32_t command);
	uint8_t OP(uint32_t command);
	uint8_t DPCS(uint32_t command);
	uint8_t INTPL(uint32_t command);
	uint8_t MVMVA(uint32_t commandData);
	uint8_t NCDS(uint32_t command);
	uint8_t CDP(uint32_t command);
	uint8_t NCDT(uint32_t command);
	uint8_t NCCS(uint32_t command);
	uint8_t CC(uint32_t command);
	uint8_t NCS(uint32_t command);
	uint8_t NCT(uint32_t command);
	uint8_t SQR(uint32_t command);
	uint8_t DCPL(uint32_t command);
	uint8_t DPCT(uint32_t command);
	uint8_t AVSZ3(uint32_t command);
	uint8_t AVSZ4(uint32_t command);
	uint8_t RTPT(uint32_t commandData);
	uint8_t GPF(uint32_t command);
	uint8_t GPL(uint32_t command);
	uint8_t NCCT(uint32_t command);

	//debug
	debugUtilities* pDebugger;

	typedef struct {
		int16_t x, y, z, unused;
	} SVector3D;

	typedef struct {
		uint8_t r, g, b, c;
	} CBGR;

	typedef struct {
		int16_t x, y;
	} SVector2D;

	typedef struct {
		int16_t z, unused;
	} SVector2Dz;

	//??
	typedef union {
		struct {
			uint8_t l, h, h2, h3;
		} b;
		struct {
			uint16_t l, h;
		} w;
		struct {
			int8_t l, h, h2, h3;
		} sb;
		struct {
			int16_t l, h;
		} sw;
		uint32_t d;
		int32_t sd;
	} PAIR;

	typedef struct {
		int16_t m11, m12, m13, m21, m22, m23, m31, m32, m33, unused;
	} SMatrix3D;

	typedef union {
		struct {//??
			SMatrix3D rMatrix;
			int32_t trX, trY, trZ;
			SMatrix3D lMatrix;
			int32_t rbk, gbk, bbk;
			SMatrix3D cMatrix;
			int32_t rfc, gfc, bfc;
			int32_t ofx, ofy;
			int32_t h;
			int32_t dqa, dqb;
			int32_t zsf3, zsf4;
			int32_t flag;
		} n;
		uint32_t r[32];
		PAIR p[32];
	} psxCP2Ctrl;

	typedef union {
		struct {
			SVector3D v0, v1, v2;
			CBGR rgb;
			int32_t otz;
			int32_t ir0, ir1, ir2, ir3;
			SVector2D sxy0, sxy1, sxy2, sxyp;
			SVector2Dz sz0, sz1, sz2, sz3;
			CBGR rgb0, rgb1, rgb2;
			int32_t reserved;
			int32_t mac0, mac1, mac2, mac3;
			uint32_t irgb, orgb;
			int32_t lzcs, lzcr;
		} n;
		uint32_t r[32];
		PAIR p[32];
	} psxCP2Data;

	struct psxRegisters {
		psxCP2Data CP2D;  // COP2 data registers
		psxCP2Ctrl CP2C;  // COP2 control registers
	};

	psxRegisters m_regs;

	int32_t LIM(int32_t value, int32_t max, int32_t min, uint32_t flag);
	uint32_t gte_divide(uint16_t numerator, uint16_t denominator);
	int32_t Lm_G1(int64_t a);
	int32_t Lm_G2(int64_t a);
	int32_t Lm_B3_sf(int64_t value, int sf, int lm);
	int32_t Lm_H(int64_t value, int sf);
	int32_t Lm_B1(int32_t a, int lm);
	int32_t Lm_B2(int32_t a, int lm);
	int32_t Lm_B3(int32_t a, int lm);
	int32_t Lm_C1(int32_t a);
	int32_t Lm_C2(int32_t a);
	int32_t Lm_C3(int32_t a);
	int32_t Lm_D(int64_t a, int sf);


	// If MSB is set, return the number of leading ones, else return the number of leading zeroes
   // For an input of 0, 32 is returned
	static uint32_t countLeadingBits(uint32_t value) {
		if (value & 0x80000000) {
			value = ~value;
		}
		return std::countl_zero<uint32_t>(value);
	}

	// Count leading zeroes of a 16-bit value. For an input of 0, 16 is returned
	static uint32_t countLeadingZeros16(uint16_t value) {
		// Use a 32-bit CLZ as it's what's most commonly available and Clang/GCC fail to optimize 16-bit CLZ
		const auto count = std::countl_zero<uint32_t>((uint32_t)value);
		return count - 16;
	}

private:
	class int44 {
	public:
		int44(int64_t value)
			: m_value(value), m_positive_overflow(value > 0x7ffffffffff), m_negative_overflow(value < -0x80000000000) {}

		int44(int64_t value, bool positive_overflow, bool negative_overflow)
			: m_value(value), m_positive_overflow(positive_overflow), m_negative_overflow(negative_overflow) {}

		int44 operator+(int64_t rhs) {
			int64_t value = ((m_value + rhs) << 20) >> 20;
			return int44(value, m_positive_overflow || (value < 0 && m_value >= 0 && rhs >= 0),//??
				m_negative_overflow || (value >= 0 && m_value < 0 && rhs < 0));
		}

		bool positiveOverflow() { return m_positive_overflow; }
		bool negativeOverflow() { return m_negative_overflow; }
		int64_t value() { return m_value; }

	private:
		int64_t m_value;
		bool m_positive_overflow;
		bool m_negative_overflow;
	};



	int s_sf;
	int64_t s_mac0;
	int64_t s_mac3;

	int32_t BOUNDS(int44 value, int max_flag, int min_flag);
	int32_t A1(int44 a);
	int32_t A2(int44 a);
	int32_t A3(int44 a);
	int64_t F(int64_t a);

	void pushZ(uint16_t z);
};