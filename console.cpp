#include "console.h"
#include "defines/defines.h"

extern bool g_emulationPaused;
extern uint32_t g_systemClocks;

psxConsole_t::psxConsole_t() {
	connectComponents();
	//debug
	connectDebugger();
}

void psxConsole_t::connectComponents() {
	bus.dma.ConnectBus(&bus);

	cpuSOC.pBus = &bus;
	bus.pCpu = &cpuSOC;
	bus.pMdec = &mdec;
	mdec.pBus = &bus;

	bus.pGpu = &gpu;

	gpu.busPtr = &bus;

	bus.pCdDrive = &cdDrive;
	cdDrive.busPtr = &bus;

	bus.timer0.pBus = &bus;
	bus.timer1.pBus = &bus;
	bus.timer2.pBus = &bus;

	bus.spu.pBus = &bus;

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
	cpuSOC.Reset();
	for (int i = 0; i < 2 * 1024 * 1024; i++) {
		bus.ram[i] = 0;
	}
}

void psxConsole_t::clock() {
	if (!bus.dma.isCpuStopped)
		cpuSOC.Clock();

	bus.dma.Clock();

	mdec.Clock();

	gpu.Clock();

	cdDrive.Clock();

	bus.timer0.Clock(_SYSTEM_CLOCK);
	bus.timer1.Clock(_SYSTEM_CLOCK);
	bus.timer2.Clock(_SYSTEM_CLOCK);

	joyMemCard.clock();
}