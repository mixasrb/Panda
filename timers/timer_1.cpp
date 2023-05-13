#include "timer_1.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void Timer1::clock(uint8_t source) {
	if (bSynchronize) {
		if (static_cast<clockSource_t>(source) == VBLANK_CLOCK) {
			bSynchronize = false;
			currentValue.elem.currentCounterValue = 0;
		}
		return;
	}

	if (clockSource == static_cast<clockSource_t>(source)) {
		if ((currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) &&
			counterMode.elem.resetCounterTo) {
			counterMode.elem.reachedTarget = 1;
			currentValue.elem.currentCounterValue = 0;
			return;
		}

		if ((currentValue.elem.currentCounterValue == 0xffff) &&
			!counterMode.elem.resetCounterTo) {
			counterMode.elem.reachedFFFF = 1;
			currentValue.elem.currentCounterValue = 0;
			return;
		}

		currentValue.elem.currentCounterValue++;
	}
}

void Timer1::ReadTimer32(const uint32_t& addr, uint16_t* data) {
	switch (addr << 28 >> 28) {
	case 0:
		*data = currentValue.data;
		break;
	case 4:
		*data = counterMode.data;
		counterMode.elem.reachedTarget = 0;
		counterMode.elem.reachedFFFF = 0;
		break;
	case 8:
		*data = targetValue.data;
		break;
	}
}

void Timer1::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		currentValue.data = data;
		break;
	case 4:
		counterMode.data = data;
		mode = counterMode.data & 0x3ff;

		clockSource = static_cast<clockSource_t>((counterMode.elem.clockSource & 1) ?
			_HBLANK_CLOCK : _SYSTEM_CLOCK);

		if (counterMode.elem.synchronizationMode == 3)
			bSynchronize = true;

		if ((mode != 0x000) && (mode != 0x100) &&
			(mode != 0x107) && (mode != 0x148)) {
			std::cout << "[TIMER1] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
			g_emulationPaused = true;
		}

		break;
	case 8:
		targetValue.data = data;
		break;
	}
}