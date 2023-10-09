#pragma once

#include <cstdint>
#include <iostream>

#include "../defines/defines.h"

class busInterface;

class Timer {
public:
	busInterface* pBus;

	virtual void clock(uint8_t source) = 0;
	uint16_t clocks;

	union currentValue_t {
		uint16_t data;
		struct {
			uint16_t currentCounterValue : 16;
		}elem;
	}currentValue;

	enum clockSource_t : int32_t {
		SYSTEM_CLOCK = 0,
		HBLANK_CLOCK = 1,
		VBLANK_CLOCK = 2,
		DOT_CLOCK = 3
	}clockSource;

	union counterMode_t {
		uint16_t data;
		struct {
			uint16_t synchronizationEnable : 1;
			uint16_t synchronizationMode : 2;
			uint16_t resetCounterTo : 1;
			uint16_t irqWhenTarget : 1;
			uint16_t irqWhenFFFF : 1;
			uint16_t irqOnceRepeat : 1;
			uint16_t irqPulseToggle : 1;
			uint16_t clockSource : 2;
			uint16_t irq : 1;
			uint16_t reachedTarget : 1;
			uint16_t reachedFFFF : 1;
			uint16_t : 3;
		}elem;
	}counterMode;
	uint16_t mode;
	bool bSynchronize = false;

	union targetValue_t {
		uint16_t data;
		struct {
			uint16_t counterTargetValue : 16;
		}elem;
	}targetValue;

	virtual void ReadTimer32(const uint32_t& addr, uint16_t* data) = 0;
	virtual void WriteTimer32(const uint32_t& addr, const uint16_t& data) = 0;
};