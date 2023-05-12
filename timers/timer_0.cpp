#include "timer_0.h"
#include "../bus/bus_interface.h"

extern bool isEmulationPaused;

void Timer0::clock(uint8_t source) {
	if (mode == 0) {
		if (curr_val.data == 0xffff) {
			count_mode.reg.reached_target = 1;
			curr_val.data = 0;
		}
		else
			curr_val.data++;
	}
}

void Timer0::ReadTimer32(const uint32_t& addr, uint16_t* data) {
	switch (addr << 28 >> 28) {
	case 0:
		*data = curr_val.data;
		break;
	case 4:
		*data = count_mode.data;
		count_mode.reg.reached_target = 0;
		count_mode.reg.reached_ffff = 0;
		break;
	case 8:
		*data = target_val.data;
		break;
	}
}

void Timer0::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		curr_val.data = data;
		break;
	case 4:
		count_mode.data = data;
		mode = count_mode.data & 0x3ff;
		if (mode) {
			isEmulationPaused = true;
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
		}
		break;
	case 8:
		target_val.data = data;
		break;
	}
}