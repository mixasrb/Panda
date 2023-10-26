#pragma once

#include <cstdint>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>

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
	std::vector<char> fileDataVect;

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

	uint8_t stored_amm;
	uint8_t stored_ass;
	uint8_t stored_asect;

	void readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect);
	uint16_t sectorSize;

	union mode_t {
		uint8_t data;
		struct {
			uint8_t cdda : 1;
			uint8_t autoPause : 1;
			uint8_t report : 1;
			uint8_t xa_Filter : 1;
			uint8_t ignoreBit : 1;
			uint8_t sectorSize : 1;
			uint8_t xa_adpcm : 1;
			uint8_t speed : 1;
		}elem;
	}mode;

	void set_cdDriveMode();

	uint8_t file;
	uint8_t channel;

	bool bFilter = false;
	bool bPlay = false;
	bool bSendINT1 = true;
	char storedSectorHeader[8];

	struct cdSector_t {
		uint16_t amm;
		uint16_t ass;
		uint16_t asect;
	};

	struct cdTrack_t {
		uint8_t track;
		cdSector_t index0Start;
		cdSector_t index0End;
		cdSector_t index1Start;
		cdSector_t index1End;
	};

	size_t fileSize;
	std::vector<cdTrack_t> cdTracks;

	void addSector(cdSector_t& sector, uint16_t mm, uint16_t ss, uint16_t sect) {
		sector.asect += sect;
		sector.ass += (sector.asect / 75) + ss;
		sector.asect %= 75;
		sector.amm += (sector.ass / 60) + mm;
		sector.ass %= 60;
	}

	//debug
	debugUtilities* p_debugger;
};