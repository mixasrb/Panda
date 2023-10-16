#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "../debug_utilities/debug_utilities.h"

class busInterface_t;

class joyMemCard_t {
public:
	joyMemCard_t();
	~joyMemCard_t();

	busInterface_t* pBus;

	void cpuRead32(const uint32_t& addr, uint32_t& data);
	void cpuWrite32(const uint32_t& addr, const uint32_t& data);

	void cpuRead16(const uint32_t& addr, uint16_t& data);
	void cpuWrite16(const uint32_t& addr, const uint16_t& data);

	void cpuRead8(const uint32_t& addr, uint8_t& data);
	void cpuWrite8(const uint32_t& addr, const uint8_t& data);

	void clock();

	void readMemCardFiles();
	void writeMemCardFiles();

	union joyTxData_t {
		uint32_t data;
		struct {
			uint32_t dataToBeSent : 8;
			uint32_t : 24;
		}elem;
	}joyTxData;

	union joyRxData_t {
		uint32_t data;
		struct {
			uint32_t firstEntry : 8;
			uint32_t secondEntry : 8;
			uint32_t thirdEntry : 8;
			uint32_t fourthEntry : 8;
		}elem;
	}joyRxData;


	enum voltageLevel_t : uint32_t {
		HIGH = 0,
		LOW = 1
	};

	union joyStat_t {
		uint32_t data;
		struct {
			uint32_t txReadFlag1 : 1;
			uint32_t rxFifoNotEmpty : 1;
			uint32_t txReadFlag2 : 1;
			uint32_t rxParityError : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			voltageLevel_t ackInputLevel : 1;
			uint32_t : 1;
			uint32_t irq : 1;
			uint32_t : 1;
			uint32_t baudrateTimer : 21;
		}elem;
	}joyStat;

	union joyMode_t {
		uint16_t data;
		struct {
			uint16_t baudrateReloadFactor : 2;
			uint16_t characterLength : 2;
			uint16_t parityEnable : 1;
			uint16_t parityType : 1;
			uint16_t : 2;
			uint16_t clkOutputPolarity : 1;
			uint16_t : 7;
		}elem;
	}joyMode;

	union joyCtrl_t {
		uint16_t data;
		struct {
			uint16_t txEnable : 1;
			uint16_t joynOutput : 1;
			uint16_t rxEnable : 1;
			uint16_t : 1;
			uint16_t acknowledge : 1;
			uint16_t : 1;
			uint16_t reset : 1;
			uint16_t : 1;
			uint16_t rxInterruptMode : 2;
			uint16_t txInterruptEnable : 1;
			uint16_t rxInterruptEnable : 1;
			uint16_t ackInterruptEnable : 1;
			uint16_t desiredSlotNumber : 1;
			uint16_t : 2;
		}elem;
	}joyCtrl;

	union joyBaud_t {
		uint16_t baudrateReloadValue;
	}joyBaud;

	void reset();
	void acknowledge();

	enum slot_t {
		JOY1 = 0,
		JOY2 = 1
	};

	voltageLevel_t ackInputLevel = HIGH;

	uint32_t baudrateTimerClocks = 0x3fffff;

	struct joy_t {
		voltageLevel_t output;
		slot_t slotNumber;
	}joy;

	enum device_t {
		PAD = 0,
		MEM_CARD = 1
	}device;

	enum mode_t : uint8_t{
		unused1 = 0x40,
		unused2 = 0x41,
		readButtons = 0x42,
		enterExitConfig = 0x43,
		setLedState = 0x44,
		getLedState = 0x45,
		getVariableResponseA = 0x46,
		getWhateverValues = 0x47,
		unknown = 0x48,
		unused3 = 0x49,
		unused4 = 0x4a,
		unused5 = 0x4b,
		getVariableResponseB = 0x4c,
		getSetRumbleProtocol = 0x4d,
		unused6 = 0x4e,
		unused7 = 0x4f,
	}mode;

	/*enum memCardCommands_t : uint8_t {
		readCommand = 'R',
		GetIDCommand = 'S',
		writeCommand = 'W',
	}*/

	uint8_t memCardCommand;
	uint8_t memCardFlags[2];
	uint16_t sectorNum;

	uint8_t sequvenceIndex = 0;

	bool txen = false;
	bool rxen = false;
	uint64_t clocks = 0;
	uint64_t sendingClocks = 0;
	uint64_t receivingClocks = 0;
	uint64_t ackClocks = 0;
	uint64_t ackPendingClocks = 0;

	bool bSendingData = false;
	bool bReceivingData = false;
	bool bAcknowledging = false;
	bool bACK = false;

	std::vector<uint8_t> rxFifo;

	bool b_irq_7_change = false;
	uint8_t swloPad1 = 0xff;
	uint8_t swhiPad1 = 0xff;
	uint8_t swloPad2 = 0xff;
	uint8_t swhiPad2 = 0xff;

	const uint32_t NUMBER_OF_BLOCKS = 16;
	const uint32_t NUMBER_OF_FRAMES_PRE_BLOCK = 64; //sectors
	const uint32_t SIZE_OF_FRAME = 128;

	uint8_t memCards[2][128 * 1024];

	//debug
	debugUtilities* p_debugger;
};