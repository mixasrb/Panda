#pragma once

#include "../timers/timer.h"
#include "../defines/defines.h"

class busInterface_t;

class timer0_t : public timer_t {
public:
	void ReadTimer32(const uint32_t& addr, uint32_t& data) override;
	void WriteTimer32(const uint32_t& addr, const uint32_t& data) override;
	void clock(const uint8_t& source) override;
	uint16_t pulsingTime = 0;
	bool bHblank = false;
	bool bSynchronizationEnable = false;
};