#pragma once

#include <cstdint>

#include "../debug_utilities/debug_utilities.h"

class cw33300;
class busInterface;

class cp0 {
public:
	cw33300* pCpu;

	uint32_t cop0r[64];

	uint32_t get(const uint8_t& reg);
	void set(const uint8_t& reg, const uint32_t& value);

	void MFC0(const uint8_t& rt, const uint8_t& rd);
	void CFC0(const uint8_t& rt, const uint8_t& rd);
	void MTC0(const uint8_t& rt, const uint8_t& rd);
	void CTC0(const uint8_t& rt, const uint8_t& rd);
	void BC0F(uint16_t imm);
	void COP0(uint32_t imm);

	void RFE();

	typedef uint8_t excode_t;
	void exceptionHandler(const excode_t& code, const uint32_t addrRef = 0);

	void interruptHandler(const uint32_t& irq);

	void checkForInterrupts();

	bool isolateDataCache = false;

	//debug
	debugUtilities* pDebugger;
};