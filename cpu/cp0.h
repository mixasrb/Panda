#pragma once

#include <cstdint>

#include "../debug_utilities/debug_utilities.h"

class cw33300;
class busInterface;

class cp0 {
public:
	cw33300* pCpu;

	uint32_t cop0r[64];

	uint32_t get(uint8_t reg);
	void set(uint8_t reg, uint32_t value);

	void MFC0(uint8_t rt, uint8_t rd);
	void CFC0(uint8_t rt, uint8_t rd);
	void MTC0(uint8_t rt, uint8_t rd);
	void CTC0(uint8_t rt, uint8_t rd);
	void BC0F(uint16_t imm);
	void COP0(uint32_t imm);

	void RFE();

	typedef uint8_t excode_t;
	void exceptionHandler(const excode_t& code, const uint32_t addrRef = 0);

	void interruptHandler(const uint32_t& irq);

	void checkForInterrupts();

	//debug
	debugUtilities* pDebugger;
};