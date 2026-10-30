#include "console.h"
#include "defines/defines.h"

extern bool g_isEmulationPaused;
extern uint32_t g_systemClocks;

PsxConsole::PsxConsole() {
	connectComponents();
}

void PsxConsole::connectComponents() {
	bus.dma.ConnectBus(&bus);

	cpuSOC.pBus = &bus;
	bus.pCpu = &cpuSOC;
	bus.pMdec = &mdec;
	mdec.pBus = &bus;

	bus.pGpu = &gpu;

	gpu.BusPtr = &bus;

	bus.pCdDrive = &cdDrive;
	cdDrive.busPtr = &bus;

	bus.timer0.pBus = &bus;
	bus.timer1.pBus = &bus;
	bus.timer2.pBus = &bus;

	bus.spu.pBus = &bus;

	joyMemCard.pBus = &bus;
	bus.pJoyMemCard = &joyMemCard;
}

void PsxConsole::clock() {
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