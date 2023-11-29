#include "console.h"
#include "defines/defines.h"

extern bool g_emulationPaused;

psxConsole_t::psxConsole_t() {
	connectComponents();
	//debug
	connectDebugger();
}

void psxConsole_t::connectComponents() {
	bus.dma.connectBus(&bus);

	cpuSOC.pBus = &bus;
	bus.pCpu= &cpuSOC;
	bus.pMdec = &mdec;
	mdec.pBus = &bus;

	bus.pGpu = &gpu;

	gpu.pBus = &bus;

	bus.pCdDrive = &cdDrive;
	cdDrive.pBus = &bus;

	bus.timer0.pBus = &bus;
	bus.timer1.pBus = &bus;
	bus.timer2.pBus = &bus;

	joyMemCard.pBus = &bus;
	bus.pJoyMemCard = &joyMemCard;
}

void psxConsole_t::connectDebugger() {
	bus.pDebugger = &debugger;
	bus.dma.p_debugger = &debugger;
	cpuSOC.p_debugger = &debugger;
	cpuSOC.cp0.pDebugger = &debugger;
	cpuSOC.cp2.pDebugger = &debugger;
	gpu.p_debugger = &debugger;
	cdDrive.p_debugger = &debugger;
	joyMemCard.p_debugger = &debugger;
}

void psxConsole_t::reset() {
	cpuSOC.reset();
	for (int i = 0; i < 2 * 1024 * 1024; i++) {
		bus.ram[i] = 0;
	}
}

void psxConsole_t::clock() {
	if (!bus.dma.isCpuStopped)
		cpuSOC.clock();

	bus.dma.clock();

	mdec.clock();

	gpu.videoClock();

	cdDrive.clock();

	bus.timer0.clock(_SYSTEM_CLOCK);
	bus.timer1.clock(_SYSTEM_CLOCK);
	bus.timer2.clock(_SYSTEM_CLOCK);

	joyMemCard.clock();
}