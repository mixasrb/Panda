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
		currentValue.data = 0;

		counterMode.data = (counterMode.data & 0x1800) | (data & 0x3ff);
		mode = counterMode.data & 0x3ff;

		counterMode.elem.irq = 1;

		clockSource = (counterMode.elem.clockSource & 1) ?
			DOT_CLOCK : SYSTEM_CLOCK;

		bSynchronizationEnable = counterMode.elem.synchronizationEnable;

		/*if ((mode != 0x000) && (mode != 0x100)
			&& (mode != 0x14b) && (mode != 0x158)) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
			g_emulationPaused = true;
		}*/

		/*if (counterMode.elem.irqOnceRepeat) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer repeat mode" << std::endl;
			g_emulationPaused = true;
		}*/

		if (counterMode.elem.irqPulseToggle) {
			std::cout << "[TIMER0] EMULATION PAUSED! unhandled timer toggle mode" << std::endl;
			g_emulationPaused = true;
		}

		break;
	case 8:
		targetValue.data = data;
		break;
	}
}

void timer0_t::clock(const uint8_t& source) {
	if (bSynchronizationEnable) {
		if (counterMode.elem.synchronizationMode == 0) {
			if (bHblank)
				return;
		}
		else if (counterMode.elem.synchronizationMode == 1) {
			if (static_cast<clockSource_t>(source) == HBLANK_CLOCK) {
				currentValue.elem.currentCounterValue = 0;
				return;
			}
		}
		else if (counterMode.elem.synchronizationMode == 2) {
			if (static_cast<clockSource_t>(source) == HBLANK_CLOCK) {
				currentValue.elem.currentCounterValue = 0;
				return;
			}
			else if (bHblank)
				return;
		}
		else if (counterMode.elem.synchronizationMode == 3) {
			if (static_cast<clockSource_t>(source) == HBLANK_CLOCK) {
				bSynchronizationEnable = false;
				return;
			}
			else
				return;
		}
	}

	if (clockSource == static_cast<clockSource_t>(source)) {
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
			pBus->pCpu->cp0.interruptHandler(_IRQ_TIMER_0);
	}
}