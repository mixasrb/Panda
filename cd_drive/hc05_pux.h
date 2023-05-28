#pragma once

#include <cstdint>
#include <sstream>
#include <vector>
#include <fstream>

#include "../debug_utilities/debug_utilities.h"

class busInterface;

class hc05_pux {
public:
	hc05_pux();

	busInterface* pBus;

	void ReadCdDrive8(const uint32_t& addr, uint8_t& data);
	void WriteCdDrive8(const uint32_t& addr, const uint8_t& data);

	void clock();
	uint32_t clocks = 0;

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

	union IndexStatus {
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
	IndexStatus indexStatus = { 0 };

	//it is not fifo it is just a register
	std::vector<uint8_t> commandFifo;
	std::vector<uint8_t> paramFifo;

	union RequestRegister {
		uint8_t data;
		struct {
			uint8_t enableBits : 5;
			uint8_t smen : 1;
			uint8_t bfwr : 1;
			uint8_t bfrd : 1;
		}reg;
	};
	RequestRegister requestReg = { 0 };

	//data fifo
	uint8_t* buffer = nullptr;
	uint16_t bufferCounter = 0;
	std::vector<char> bufferVect;

	std::vector<uint8_t> responseFifo;

	union InterruptEnable {
		uint8_t data;
		struct {
			uint8_t enableBits : 5;
			uint8_t : 3;
		}reg;
	};

	InterruptEnable interruptEnable = { 0 };
	union IRQ_Flag_R_t {
		uint8_t data;
		struct {
			uint8_t int_1_7 : 3;
			uint8_t int_8 : 1;
			uint8_t int_10 : 1;
			uint8_t : 3;
		}reg;
	};
	IRQ_Flag_R_t irq_flag_read = { 0 };

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

	std::vector<uint8_t> tempFirstResponseFifo;
	std::vector<uint8_t> tempSecondResponseFifo;
	std::vector<uint8_t> tempDataResponseFifo;

	struct command_t {
		std::string name;
		uint8_t(hc05_pux::* command)(const uint8_t& command);
	};

	std::vector<command_t> commandLookup;

	void executeCommand(const uint8_t& command);

	uint8_t xxxxx(const uint8_t& command);
	uint8_t Getstat(const uint8_t& command);
	uint8_t Play(const uint8_t& command);
	uint8_t Setloc(const uint8_t& command);
	uint8_t ReadN(const uint8_t& command);
	uint8_t Stop(const uint8_t& command);
	uint8_t Pause(const uint8_t& command);
	uint8_t Init(const uint8_t& command);
	uint8_t Demute(const uint8_t& command);
	uint8_t Setfilter(const uint8_t& command);
	uint8_t Setmode(const uint8_t& command);
	uint8_t GetlocL(const uint8_t& command);
	uint8_t GetTN(const uint8_t& command);
	uint8_t GetTD(const uint8_t& command);
	uint8_t SeekL(const uint8_t& command);
	uint8_t Test(const uint8_t& command);
	uint8_t GetID(const uint8_t& command);
	uint8_t ReadS(const uint8_t& command);

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
	uint8_t stat = STAT_SPINDLE_MOTOR;

	uint8_t amm;
	uint8_t ass;
	uint8_t asect;
	uint8_t ammOld;
	uint8_t assOld;
	uint8_t asectOld;

	void readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect);
	uint16_t sectorSize;

	bool doubleSpeed = false;


	uint8_t fromHexToDec8(const uint8_t& h);

	//debug
	debugUtilities* p_debugger;
};