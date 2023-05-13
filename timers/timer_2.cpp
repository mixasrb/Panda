#include "timer_2.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void Timer2::clock(uint8_t source) {
	//if (mode == 0) {
	//	if (currentValue.data == 0xffff) {
	//		counterMode.elem.reachedTarget = 1;//
	//		currentValue.data = 0;
	//	}
	//	else
	//		currentValue.data++;
	//}
	//else if (mode == 0x248) {
	//	if (currentValue.data == targetValue.elem.counterTargetValue) {
	//		counterMode.elem.reachedTarget = 1;//
	//		currentValue.data = 0;
	//	}
	//	if (clocks == 8) {
	//		currentValue.data += 1;
	//		clocks = 0;
	//	}
	//	clocks++;
	//}
	//else if (mode == 0x258) {
	//	if (currentValue.data == targetValue.elem.counterTargetValue) {
	//		counterMode.elem.reachedTarget = 1;
	//		currentValue.data = 0;
	//		//Timer2 interrupt is naturally edge trigered
	//		pBus->pCp0->interruptHandler(_TIMER_2);
	//		counterMode.elem.irq = 0;
	//	}
	//}

	if (clockSource == static_cast<clockSource_t>(source)) {
		if ((currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) &&
			counterMode.elem.resetCounterTo) {
			counterMode.elem.reachedTarget = 1;
			currentValue.elem.currentCounterValue = 0;

			if (counterMode.elem.irqWhenTarget) {
				pBus->pCp0->interruptHandler(_TIMER_2);
				counterMode.elem.irq = 0;
			}
			return;
		}

		if ((currentValue.elem.currentCounterValue == 0xffff) &&
			!counterMode.elem.resetCounterTo) {
			counterMode.elem.reachedFFFF = 1;
			currentValue.elem.currentCounterValue = 0;
			return;
		}

		//pulsing
		if (currentValue.elem.currentCounterValue == 16)
			counterMode.elem.irq = 1;

		if (counterMode.elem.clockSource == 3) {
			if (clocks == 8) {
				clocks = 0;
				currentValue.elem.currentCounterValue++;
				return;
			}
			clocks++;
			return;
		}

		currentValue.elem.currentCounterValue++;
	}
}

void Timer2::ReadTimer32(const uint32_t& addr, uint16_t* data) {
	switch (addr << 28 >> 28)
	{
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

void Timer2::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		currentValue.data = data;
		break;
	case 4:
		counterMode.data = data;
		mode = counterMode.data & 0x3ff;

		clockSource = static_cast<clockSource_t>((counterMode.elem.clockSource & 1) ?
			_SYSTEM_CLOCK : _SYSTEM_CLOCK);

		if (counterMode.elem.synchronizationMode == 3)
			bSynchronize = true;

		if ((mode != 0x000) && (mode != 0x248) &&
			(mode != 0x258)) {
			std::cout << "[TIMER2] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
			g_emulationPaused = true;
		}
		break;
	case 8:
		targetValue.data = data;
		break;
	}
}