#include "timer_1.h"
#include "../bus/bus_interface.h"

extern bool isEmulationPaused;

void Timer1::clock(uint8_t source) {
	if (mode == 0) {
		if (source == SYSTEM_CLOCK)
			if (curr_val.data == 0xffff) {
				count_mode.reg.reached_target = 1;//
				curr_val.data = 0;
			}
			else
				curr_val.data++;
	}
	else if (mode == 0x148) {
		if (source == HBLANK_CLOCK)
			if (curr_val.data == target_val.reg.counter_target_value) {
				count_mode.reg.reached_target = 1;
				curr_val.data = 0;
			}
			else
				curr_val.data++;
	}
	else if (mode == 0x107) {
		if (source == VBLANK_CLOCK) {
			mode = 0x100;
			curr_val.data = 0;
		}
	}
	else if (mode == 0x100) {
		if (source == HBLANK_CLOCK)
			if (curr_val.data == 0xffff) {
				count_mode.reg.reached_ffff = 1;//
				curr_val.data = 0;
			}
			else
				curr_val.data++;
	}
	else if (mode) {
		isEmulationPaused = true;
		std::cout << "[TIMER1] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
	}
}

void Timer1::ReadTimer32(const uint32_t& addr, uint16_t* data) {
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

void Timer1::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		curr_val.data = data;
		break;
	case 4:
		count_mode.data = data;
		mode = count_mode.data & 0x3ff;
		break;
	case 8:
		target_val.data = data;
		break;
	}
}