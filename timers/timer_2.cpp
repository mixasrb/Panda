#include "timer_2.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void timer2::clock(uint8_t source) {
	if (clockSource == static_cast<clockSource_t>(source)) {
		if ((currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) &&
			counterMode.elem.resetCounterTo) {
			currentValue.elem.currentCounterValue = 0;
			counterMode.elem.reachedTarget = 1;

			if (counterMode.elem.irqWhenTarget) {
				if (counterMode.elem.irq == 1)
					pBus->pCpu->cp0.interruptHandler(_IRQ_TIMER_2);
				counterMode.elem.irq = 0;
			}
			return;
		}

		if ((currentValue.elem.currentCounterValue == 0xffff) &&
			!counterMode.elem.resetCounterTo) {
			currentValue.elem.currentCounterValue = 0;
			counterMode.elem.reachedFFFF = 1;
			return;
		}

		//pulsing
		if ((!counterMode.elem.irqPulseToggle) &&
			(counterMode.elem.irq == 0) &&
			(currentValue.elem.currentCounterValue == 32))
			counterMode.elem.irq = 1;

		if (counterMode.elem.clockSource & 0x2) {
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

void timer2::ReadTimer32(const uint32_t& addr, uint16_t* data) {
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

void timer2::WriteTimer32(const uint32_t& addr, const uint16_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		currentValue.data = data;
		break;
	case 4:
		counterMode.data = data;
		mode = counterMode.data & 0x3ff;

		clockSource = (counterMode.elem.clockSource & 0x2) ?
			SYSTEM_CLOCK : SYSTEM_CLOCK;

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