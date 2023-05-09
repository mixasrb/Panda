#include "timer_2.h"
#include "../bus/bus_interface.h"

extern bool isEmulationPaused;

void Timer2::clock(uint8_t source) {
	if (mode == 0x248) {
		if (curr_val.data == target_val.reg.counter_target_value) {
			count_mode.reg.reached_target = 1;
			curr_val.data = 0;
		}

		if (clocks == 8) {
			curr_val.data += 1;
			clocks = 0;
		}
		clocks++;
	}
	else if (mode == 0x258) {
		if (curr_val.data == target_val.reg.counter_target_value) {
			count_mode.reg.reached_target = 1;
			curr_val.data = 0;
			//Timer2 interrupt is naturally edge trigered
			pBus->pCp0->interruptHandler(_TIMER_2);
			count_mode.reg.irq_request = 0;
		}

		if (clocks == 8) {
			curr_val.data += 1;
			//pulsing
			if(curr_val.data == 2)
				count_mode.reg.irq_request = 1;
			clocks = 0;
		}
		clocks++;
	}
	else if (mode) {
		std::cout << "[TIMER2] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
		isEmulationPaused = true;
	}
}

void Timer2::ReadTimer32(const uint32_t& addr, uint16_t* data) {
	switch (addr << 28 >> 28)
	{
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

void Timer2::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
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