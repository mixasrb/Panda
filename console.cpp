#include "console.h"
#include "defines/defines.h"

bool isCpuStopped = false;
extern bool isEmulationPaused;

Console::Console() {
	connectComponents();
	//debug
	connectDebugger();
}

void Console::reset() {
	cpuSOC.reset();
}

void Console::clock() {
	if (!isCpuStopped)
		cpuSOC.clock();

	bus.dma.clock();

	gpu.videoClock();

	cdDrive.clock();

	bus.timer0.clock(SYSTEM_CLOCK);
	bus.timer1.clock(SYSTEM_CLOCK);
	bus.timer2.clock(SYSTEM_CLOCK);

	joyMemCard.clock();
}

void Console::connectComponents() {
	bus.dma.ConnectBus(&bus);

	cpuSOC.pBus = &bus;
	bus.pCp0 = &cpuSOC.cp0;

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

void Console::connectDebugger()
{
	bus.pDebugger = &debugger;
	bus.dma.p_debugger = &debugger;
	cpuSOC.p_debugger = &debugger;
	cpuSOC.cp0.pDebugger = &debugger;
	gpu.p_debugger = &debugger;
	cdDrive.p_debugger = &debugger;
	joyMemCard.p_debugger = &debugger;
}