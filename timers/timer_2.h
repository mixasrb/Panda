#pragma once

#include "../timers/timer.h"
#include "../defines/defines.h"

class busInterface_t;

class timer2_t : public timer_t {
public:
	void ReadTimer32(const uint32_t& addr, uint16_t* data) override;
	void WriteTimer32(const uint32_t& addr, const uint16_t& data) override;
	void clock(uint8_t source) override;
};