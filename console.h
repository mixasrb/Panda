#pragma once

#include "bus/bus_interface.h"
#include "cpu/cw33300.h"
#include "mdec/mdec.h"
#include "gpu/cxd85xxx.h"
#include "cd_drive/hc05_pux.h"
#include "peripherals/joy_mem_card.h"
#include "debug_utilities/debug_utilities.h"


class psxConsole_t {
public:

	psxConsole_t();

	busInterface_t bus;
	cpu_t cpuSOC;
	mdec_t mdec;
	gpu_t gpu;
	cdDrive cdDrive;
	joyMemCard_t joyMemCard;

	uint64_t clocks = 0;
	uint64_t cpuClocks = 0;
	uint64_t videoClocks = 0;


	void connectComponents();
	void connectDebugger();

	void reset();
	void clock();

	//debug
	debugUtilities debugger;
};