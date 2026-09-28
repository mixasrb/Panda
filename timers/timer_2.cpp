#include "timer_2.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void timer2_t::ReadTimer32(const uint32_t& addr, uint32_t& data) {
	switch (addr & 0xf) {
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

void timer2_t::WriteTimer32(const uint32_t& addr, const uint32_t& data) {
	switch (addr & 0xf) {
	case 0:
		currentValue.data = data;
		break;

	case 4:
		currentValue.data = 0;
		counterMode.data = (counterMode.data & 0x1800) | (data & 0x3ff);
		mode = counterMode.data & 0x3ff;

		if (data & 0x400)
			counterMode.elem.irq = 1;

		clockSource = (counterMode.elem.clockSource & 0x2) ?
			SYSTEM_CLOCK : SYSTEM_CLOCK;

		bSynchronizationEnable = counterMode.elem.synchronizationEnable;

		/*if ((mode != 0x000) && (mode != 0x200) 
			&& (mode != 0x248) && (mode != 0x258)) {
			std::cout << "[TIMER2] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << '\n';
			g_emulationPaused = true;
		}*/

		/*if (counterMode.elem.irqOnceRepeat) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer repeat mode" << '\n';
			g_emulationPaused = true;
		}*/

		if (counterMode.elem.irqPulseToggle) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer toggle mode" << '\n';
			g_emulationPaused = true;
		}
		break;

	case 8:
		targetValue.data = data;
		break;
	}
}

void timer2_t::clock(const uint8_t& source) {
	if (bSynchronizationEnable) {
		if ((counterMode.elem.synchronizationMode == 0) || (counterMode.elem.synchronizationMode == 3)) {
			return;
		}
	}

	if (clockSource == static_cast<clockSource_t>(source)) {

		if (counterMode.elem.clockSource & 0x2)
			if (clocks == 7)
				clocks = 0;
			else {
				clocks++;
				return;
			}

		uint8_t irqOld = counterMode.elem.irq;

		if ((currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) &&
			(targetValue.elem.counterTargetValue > 0)) {

			counterMode.elem.reachedTarget = 1;

			if (counterMode.elem.resetCounterTo0AfterTarget)
				currentValue.elem.currentCounterValue = -1;

			if (counterMode.elem.irqWhenTarget) {
				counterMode.elem.irq = 0;
				pulsingTime = 0;
			}
		}
		else if (currentValue.elem.currentCounterValue == 0xffff) {

			counterMode.elem.reachedFFFF = 1;

			currentValue.elem.currentCounterValue = -1;

			if (counterMode.elem.irqWhenFFFF) {
				counterMode.elem.irq = 0;
				pulsingTime = 0;
			}
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
			pBus->pCpu->cp0.interruptHandler(_IRQ_TIMER_2);
	}
}