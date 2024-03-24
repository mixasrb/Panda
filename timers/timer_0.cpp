#include "timer_0.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void timer0_t::ReadTimer32(const uint32_t& addr, uint32_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		data = currentValue.data;
		break;
	case 4:
		data = counterMode.data;

		counterMode.elem.reachedTarget = 0;
		counterMode.elem.reachedFFFF = 0;
		break;
	case 8:
		data = targetValue.data;
		break;
	}
}

void timer0_t::WriteTimer32(const uint32_t& addr, const uint32_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		currentValue.data = data;
		break;
	case 4:
		counterMode.data = data & 0x3ff;
		mode = counterMode.data & 0x3ff;

		counterMode.elem.irq = 1;

		clockSource = (counterMode.elem.clockSource & 1) ?
			DOT_CLOCK : SYSTEM_CLOCK;

		if ((mode != 0x000) && (mode != 0x14b) && 
			(mode != 0x158)) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
			g_emulationPaused = true;
		}
		break;
	case 8:
		targetValue.data = data;
		break;
	}
}

void timer0_t::clock(const uint8_t& source) {
	if (counterMode.elem.synchronizationEnable) {
		if ((static_cast<clockSource_t>(source) == HBLANK_CLOCK) && (counterMode.elem.synchronizationMode == 1)) {
			currentValue.elem.currentCounterValue = 0;
			return;
		}
	}
	else if (clockSource == static_cast<clockSource_t>(source)) {
		uint8_t irqOld = counterMode.elem.irq;

		if (currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) {

			counterMode.elem.reachedTarget = 1;

			if (counterMode.elem.resetCounterTo0AfterTarget)
				currentValue.elem.currentCounterValue = -1;

			if (counterMode.elem.irqWhenTarget) {
				counterMode.elem.irq = 0;
				pulsingTime = 0;
			}
		}

		if (currentValue.elem.currentCounterValue == 0xffff) {

			counterMode.elem.reachedFFFF = 1;

			currentValue.elem.currentCounterValue = -1;
		}

		currentValue.elem.currentCounterValue++;

		//pulsing
		if ((!counterMode.elem.irqPulseToggle) &&
			(counterMode.elem.irq == 0))
			if (pulsingTime == 8)
				counterMode.elem.irq = 1;
			else
				pulsingTime++;

		if ((irqOld == 1) && (counterMode.elem.irq == 0))
			pBus->pCpu->cp0.interruptHandler(_IRQ_TIMER_0);

	}
}