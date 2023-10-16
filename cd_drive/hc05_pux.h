#pragma once

#include <cstdint>
#include <sstream>
#include <vector>
#include <fstream>

#include "../debug_utilities/debug_utilities.h"

class busInterface_t;

class cdDrive {
public:

	cdDrive();

	busInterface_t* pBus;

	void readCdDrive8(const uint32_t& addr, uint8_t& data);
	void writeCdDrive8(const uint32_t& addr, const uint8_t& data);

	void clock();
	uint32_t clocks = 0;
	uint32_t dataClocks = 0;

	enum driveStatus_t :uint8_t {
		DOOR_OPEN = 0,
		SPIN_UP = 1,
		DETECT_BUSY = 2,
		NO_DISC = 3,
		AUDIO_DISC = 4,
		UNLICENCED_DISC_MODE_1 = 5,
		UNLICENCED_DISC_MODE_2 = 6,
		UNLICENCED_DISC_MODE_2_AUDIO = 7,
		DEBUG_YAROZE_MODE_2 = 8,
		LICENCED_MODE_2 = 9,
		MODCHIP_AUDIO_MODE_1 = 10
	}driveStatus;

	union indexStatus_t {
		uint8_t data;
		struct {
			uint8_t index : 2;
			uint8_t adpbusy : 1;
			uint8_t prmempt : 1;
			uint8_t prmwrdy : 1;
			uint8_t rslrrdy : 1;
			uint8_t drqsts : 1;
			uint8_t busysts : 1; //not emulated
		}reg;
	};

	indexStatus_t indexStatus = { 0 };

	//it is not vecor it is just a register??
	std::vector<uint8_t> commandVector;
	std::vector<uint8_t> paramFifo;

	union requestRegister_t {
		uint8_t data;
		struct {
			uint8_t enableBits : 5;
			uint8_t smen : 1;
			uint8_t bfwr : 1;
			uint8_t bfrd : 1;
		}reg;
	};
	requestRegister_t requestReg = { 0 };

	//data fifo
	uint8_t* pBuffer = nullptr;
	uint16_t bufferCounter = 0;
	std::vector<char> bufferVect;

	std::vector<uint8_t> responseFifo;

	union interruptEnable_t {
		uint8_t data;
		struct {
			uint8_t enableBits : 5;
			uint8_t : 3;
		}reg;
	};

	interruptEnable_t interruptEnable = { 0 };

	union irqFlagRead_t {
		uint8_t data;
		struct {
			uint8_t int_1_7 : 3;
			uint8_t int_8 : 1;
			uint8_t int_10 : 1;
			uint8_t : 3;
		}reg;
	};
	irqFlagRead_t irqFlagRead = { 0 };

	enum responseINT_t :uint8_t {
		INT0,
		INT1,
		INT2,
		INT3,
		INT4,
		INT5
	};

	std::vector<responseINT_t> responsesINT;

	void sendResponseINT(const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount);

	responseINT_t firstResponseINT;
	bool bFirstResponse = false;
	uint32_t waitClocksFirstResponse = 0;
	void setFirstResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount);

	responseINT_t secondResponseINT;
	bool bSecondResponse = false;
	uint32_t waitClocksSecondResponse = 0;
	void setSecondResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount);

	responseINT_t dataResponseINT;
	bool bDataResponse = false;
	uint32_t waitClocksDataResponse = 0;
	void setDataResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount);

	std::vector<uint8_t> tempFirstResponseVector;
	std::vector<uint8_t> tempSecondResponseVector;
	std::vector<uint8_t> tempDataResponseVector;

	struct command_t {
		std::string name;
		void(cdDrive::* command)(const uint8_t& command);
	};

	std::vector<command_t> commandLookup;

	void executeCommand(const uint8_t& command);

	void xxxxx(const uint8_t& command);
	void getStat(const uint8_t& command);
	void play(const uint8_t& command);
	void setLoc(const uint8_t& command);
	void readN(const uint8_t& command);
	void stop(const uint8_t& command);
	void pause(const uint8_t& command);
	void init(const uint8_t& command);
	void mute(const uint8_t& command);
	void demute(const uint8_t& command);
	void setFilter(const uint8_t& command);
	void setMode(const uint8_t& command);
	void getParam(const uint8_t& command);
	void getLocL(const uint8_t& command);
	void getLocP(const uint8_t& command);
	void getTN(const uint8_t& command);
	void getTD(const uint8_t& command);
	void seekL(const uint8_t& command);
	void seekP(const uint8_t& command);
	void test(const uint8_t& command);
	void getID(const uint8_t& command);
	void readS(const uint8_t& command);
	void readTOC(const uint8_t& command);

	enum stat_t :uint8_t {
		STAT_PLAY = 0x80,
		STAT_SEEK = 0x40,
		STAT_READ = 0x20,
		STAT_SHELL_OPEN = 0x10,
		STAT_ID_ERROR = 0x08,
		STAT_SEEK_ERROR = 0x04,
		STAT_SPINDLE_MOTOR = 0x02,
		STAT_ERROR = 0x01,
		STAT_NULL = 0x0
	};
	uint8_t stat;

	uint8_t amm;
	uint8_t ass;
	uint8_t asect;
	uint8_t ammOld;
	uint8_t assOld;
	uint8_t asectOld;

	void readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect);
	uint16_t sectorSize;

	bool doubleSpeed = false;
	bool bReport = false;
	uint8_t mode;
	uint8_t file;
	uint8_t channel;

	bool bFilter = false;
	char storedSectorHeader[8];

	//debug
	debugUtilities* p_debugger;
};