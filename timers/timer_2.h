#pragma once

#include "../timers/timer.h"
#include "../defines/defines.h"

class busInterface;

class Timer2 : public Timer {
public:
	busInterface* pBus;
	void clock(uint8_t source) override;
	void ReadTimer32(const uint32_t& addr, uint16_t* data) override;
	void WriteTimer32(const uint32_t& addr, const uint16_t& data) override;

	CurrentValue curr_val;
	CounterMode count_mode;
	TargetValue target_val;

	uint16_t clocks = 0;
};