#pragma once

#include <cstdint>
#include <iostream>

#include "../defines/defines.h"

class Timer {
public:
	virtual void clock(uint8_t source) = 0;

	union CurrentValue {
		uint16_t data;
		struct {
			uint16_t current_counter_value : 16;
		}reg;
	};

	union CounterMode {
		uint16_t data;
		struct {
			uint16_t synchronization_enable : 1;
			uint16_t synchronization_mode : 2;
			uint16_t reset_counter : 1;
			uint16_t irq_target : 1;
			uint16_t irq_ffff : 1;
			uint16_t irq_once_repeat : 1;
			uint16_t irq_pulse_toggle : 1;
			uint16_t clock_source : 2;
			uint16_t irq_request : 1;
			uint16_t reached_target : 1;
			uint16_t reached_ffff : 1;
			uint16_t : 3;
		}reg;
	};

	union TargetValue {
		uint16_t data;
		struct {
			uint16_t counter_target_value : 16;
		}reg;
	};

	uint16_t mode;

	virtual void ReadTimer32(const uint32_t& addr, uint16_t* data) = 0;
	virtual void WriteTimer32(const uint32_t& addr, const uint16_t& data) = 0;
};