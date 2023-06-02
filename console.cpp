#include "console.h"
#include "defines/defines.h"

extern bool g_emulationPaused;

Console::Console() {
	connectComponents();
	//debug
	connectDebugger();
}

void Console::reset() {
	cpuSOC.reset();
}

void Console::clock() {
	if (!bus.dma.isCpuStopped)
		cpuSOC.clock();

	bus.dma.clock();

	gpu.videoClock();

	cdDrive.clock();

	bus.timer0.clock(_SYSTEM_CLOCK);
	bus.timer1.clock(_SYSTEM_CLOCK);
	bus.timer2.clock(_SYSTEM_CLOCK);

	joyMemCard.clock();
}

void Console::connectComponents() {
	bus.dma.ConnectBus(&bus);

	cpuSOC.pBus = &bus;
	bus.pCpu= &cpuSOC;

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

void Console::connectDebugger() {
	bus.pDebugger = &debugger;
	bus.dma.p_debugger = &debugger;
	cpuSOC.p_debugger = &debugger;
	cpuSOC.cp0.pDebugger = &debugger;
	cpuSOC.cp2.pDebugger = &debugger;
	gpu.p_debugger = &debugger;
	cdDrive.p_debugger = &debugger;
	joyMemCard.p_debugger = &debugger;
}