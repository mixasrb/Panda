#pragma once

#include "cpu/cw33300.h"
#include "bus/bus_interface.h"
#include "gpu/cxd85xxx.h"
#include "cd_drive/hc05_pux.h"
#include "peripherals/joy_mem_card.h"
#include "debug_utilities/debug_utilities.h"

class Console {
public:
	cw33300 cpuSOC;
	busInterface bus;
	cxd85xxx gpu;
	hc05_pux cdDrive;
	joyMemCard joyMemCard;

	Console();

	uint64_t clocks = 0;
	uint64_t cpuClocks = 0;
	uint64_t videoClocks = 0;

	void reset();
	void clock();
	void connectComponents();
	void connectDebugger();

	//debug
	debugUtilities debugger;
};