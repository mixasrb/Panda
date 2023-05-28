#pragma once

#include <cstdint>
#include <vector>

#include "../debug_utilities/debug_utilities.h"

class cw33300;

class cp2 {
public:
	cp2();
	cw33300* pCpu;

	uint32_t cop2dat[32];
	uint32_t cop2cnt[32];

	union flag_t {
		uint32_t data;
		struct {
			uint32_t : 12;
			uint32_t ir0Sat : 1;
			uint32_t sy2Sat : 1;
			uint32_t sx2Sat : 1;
			uint32_t mac0ResultNegOV : 1;
			uint32_t mac0ResultPosOV : 1;
			uint32_t divOV : 1;
			uint32_t sz3_otcSat : 1;
			uint32_t colorFifoBSat : 1;
			uint32_t colorFifoGSat : 1;
			uint32_t colorFifoRSat : 1;
			uint32_t ir3Sat : 1;
			uint32_t ir2Sat : 1;
			uint32_t ir1Sat : 1;
			uint32_t mac3ResultNegOV : 1;
			uint32_t mac2ResultNegOV : 1;
			uint32_t mac1ResultNegOV : 1;
			uint32_t mac3ResultPosOV : 1;
			uint32_t mac2ResultPosOV : 1;
			uint32_t mac1ResultPosOV : 1;
			uint32_t errorFlag: 1;
		}elem;
	}flag;

	uint32_t getDat(uint8_t reg);
	uint32_t getCnt(uint8_t reg);
	void setDat(uint8_t reg, uint32_t value);
	void setCnt(uint8_t reg, uint32_t value);

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

	int32_t devide();

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
};