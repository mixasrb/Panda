#include "console.h"
#include "defines/defines.h"

extern bool g_isEmulationPaused;
extern uint32_t g_systemClocksFrame;
extern uint32_t g_systemFrames;
extern uint32_t g_systemSeconds;
extern uint32_t g_systemMinutes;
extern uint32_t g_cpuClocksPerFrame;

PsxConsole::PsxConsole() {
	connectComponents();
}

void PsxConsole::connectComponents() {
	bus.dma.ConnectBus(&bus);

	cpuSoc.pBus = &bus;
	bus.pCpu = &cpuSoc;
	bus.pMdec = &mdec;
	mdec.pBus = &bus;

	bus.pGpu = &gpu;

	gpu.BusPtr = &bus;

	bus.pCdDrive = &cdDrive;
	cdDrive.busPtr = &bus;

	bus.timer0.pBus = &bus;
	bus.timer1.pBus = &bus;
	bus.timer2.pBus = &bus;

	bus.spu.busPtr = &bus;

	joyMemCard.pBus = &bus;
	bus.pJoyMemCard = &joyMemCard;
}

void PsxConsole::clock() {
	if (!bus.dma.isCpuStopped)
		cpuSoc.Clock();

	bus.dma.Clock();

	mdec.Clock();

	gpu.Clock();

	cdDrive.Clock();

	bus.timer0.Clock(_SYSTEM_CLOCK);
	bus.timer1.Clock(_SYSTEM_CLOCK);
	bus.timer2.Clock(_SYSTEM_CLOCK);

	joyMemCard.clock();

	g_systemClocksFrame++;
	if (g_systemClocksFrame == g_cpuClocksPerFrame) {
		gpu.scanlineClocksCpu = 0;

		g_systemClocksFrame = 0;
		g_systemFrames++;

		if (g_systemFrames == 60) {
			g_systemFrames = 0;
			g_systemSeconds++;

			if (g_systemSeconds == 60) {
				g_systemSeconds = 0;
				g_systemMinutes++;
			}
		}
	}
}