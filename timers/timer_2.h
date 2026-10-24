#pragma once

#include "../timers/timer.h"

class BusInterface;

class timer2_t : public Timer {
public:
	void Read32(const uint32_t& addr, uint32_t& data) override;
	void Write32(const uint32_t& addr, const uint32_t& data) override;
	void Clock(const uint8_t& source) override;
	uint16_t pulsingTime = 0;
	bool bSynchronizationEnable = false;
};