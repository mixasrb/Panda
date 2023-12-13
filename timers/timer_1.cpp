#include "timer_1.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

void timer1_t::ReadTimer32(const uint32_t& addr, uint32_t& data) {
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

void timer1_t::WriteTimer32(const uint32_t& addr, const uint32_t& data) {
	switch (addr << 28 >> 28) {
	case 0:
		currentValue.data = 0;
		break;

	case 4:
		counterMode.data = data & 0x3ff;
		mode = counterMode.data & 0x3ff;

		counterMode.elem.irq = 1;

		clockSource = (counterMode.elem.clockSource & 1) ?
			HBLANK_CLOCK : SYSTEM_CLOCK;

		if ((counterMode.elem.synchronizationEnable) && (counterMode.elem.synchronizationMode == 3))
			bSynchronize = true;

		if ((mode != 0x000) && (mode != 0x100) &&
			(mode != 0x107) && (mode != 0x148) &&
			(mode != 0x149) && (mode != 0x158)) {
			std::cout << "[TIMER1] EMULATION PAUSED! unhandled timer mode 0x" << std::hex << mode << std::endl;
			g_emulationPaused = true;
		}

		break;

	case 8:
		targetValue.data = data;
		break;
	}
}

void timer1_t::clock(const uint8_t& source) {
	if (bSynchronize) {
		if (static_cast<clockSource_t>(source) == VBLANK_CLOCK) {
			bSynchronize = false;
		}
		return;
	}

	if (clockSource == static_cast<clockSource_t>(source)) {
		uint8_t irqOld = counterMode.elem.irq;

		if (currentValue.elem.currentCounterValue == targetValue.elem.counterTargetValue) {

			counterMode.elem.reachedTarget = 1;

			if (counterMode.elem.resetCounterTo0AfterTarget)
				currentValue.elem.currentCounterValue = 0;

			if (counterMode.elem.irqWhenTarget) {
				counterMode.elem.irq = 0;
				pulsingTime = 0;
			}
		}
		else if (currentValue.elem.currentCounterValue == 0xffff) {

			counterMode.elem.reachedFFFF = 1;

			currentValue.elem.currentCounterValue = 0;
		}

		//pulsing
		if ((!counterMode.elem.irqPulseToggle) &&
			(counterMode.elem.irq == 0))
			if (pulsingTime == 8)
				counterMode.elem.irq = 1;
			else
				pulsingTime++;

		if ((irqOld == 1) && (counterMode.elem.irq == 0))
			pBus->pCpu->cp0.interruptHandler(_IRQ_TIMER_1);

		if ((counterMode.elem.synchronizationEnable) && (counterMode.elem.synchronizationMode == 0)) {
			if (pBus->pGpu->scanlineCount < 240)
				currentValue.elem.currentCounterValue++;

			return;
		}

		currentValue.elem.currentCounterValue++;
	}
}