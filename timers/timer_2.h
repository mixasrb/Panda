#pragma once

#include "../timers/timer.h"
#include "../defines/defines.h"

class busInterface;

class timer2 : public Timer {
public:
	void clock(uint8_t source) override;
	void ReadTimer32(const uint32_t& addr, uint16_t* data) override;
	void WriteTimer32(const uint32_t& addr, const uint16_t& data) override;
};