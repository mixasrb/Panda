#pragma once

#include <cstdint>
#include <sstream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>

#include "../debug_utilities/debug_utilities.h"

constexpr uint32_t SYSTEM_CLOCK = 44100 * 0x300; //33 868 800

class BusInterface;

class cdDrive {
public:

	cdDrive();

	BusInterface* pBus;

	void Read8(const uint32_t& addr, uint8_t& data);
	void Write8(const uint32_t& addr, const uint8_t& data);

	void Clock();
	uint32_t clocks = 0;
	uint32_t dataClocks = 0;

	enum DriveStatus :uint8_t {
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
			uint8_t busysts : 1;
		}elem;
	};

	IndexStatus indexStatus = { 0 };

	//it is not vector it is just a register??
	std::vector<uint8_t> commandVector;
	std::vector<uint8_t> paramFifo;

	union RequestRegister {
		uint8_t data;
		struct {
			uint8_t notUsed : 5;
			uint8_t smen : 1;
			uint8_t bfwr : 1;
			uint8_t bfrd : 1;
		}elem;
	};
	RequestRegister requestReg = { 0 };

	//data fifo
	uint8_t* pBuffer = nullptr;
	uint16_t bufferCounter = 0;
	std::vector<char> fileDataVect;

	std::vector<uint8_t> responseFifo;

	union InterruptEnable {
		uint8_t data;
		struct {
			uint8_t int_1_7 : 3;
			uint8_t int_8 : 1;
			uint8_t int_10 : 1;
			uint8_t : 3;
		}elem;
	};

	InterruptEnable interruptEnable = { 0 };

	union IrqFlagRead {
		uint8_t data;
		struct {
			uint8_t int_1_7 : 3;
			uint8_t int_8 : 1;
			uint8_t int_10 : 1;
			uint8_t : 3;
		}elem;
	};
	IrqFlagRead irqFlagRead = { 0 };

	enum ResponseInterrupt :uint8_t {
		INTERRUPT_0,
		INTERRUPT_1,
		INTERRUPT_2,
		INTERRUPT_3,
		INTERRUPT_4,
		INTERRUPT_5
	};

	std::queue<ResponseInterrupt> responsesINT;

	void SendResponseInterrupt(const ResponseInterrupt& responseINT, 
		uint8_t* pResponses, const uint8_t& responseCount);

	ResponseInterrupt firstResponseINT;
	bool bFirstResponse = false;
	uint32_t waitClocksFirstResponse = 0;
	void SetFirstResponse(const uint32_t& waitClocks, const ResponseInterrupt& responseINT, 
		uint8_t* pResponses, const uint8_t& responseCount);

	ResponseInterrupt secondResponseINT;
	bool bSecondResponse = false;
	uint32_t waitClocksSecondResponse = 0;
	void SetSecondResponse(const uint32_t& waitClocks, const ResponseInterrupt& responseINT, 
		uint8_t* pResponses, const uint8_t& responseCount);

	ResponseInterrupt dataResponseINT;
	bool bDataResponse = false;
	uint32_t waitClocksDataResponse = 0;
	void SetDataResponse(const ResponseInterrupt& responseINT, 
		uint8_t* pResponses, const uint8_t& responseCount);

	std::vector<uint8_t> tempFirstResponseVector;
	std::vector<uint8_t> tempSecondResponseVector;
	std::vector<uint8_t> tempDataResponseVector;

	struct Command {
		std::string name;
		void(cdDrive::* command)(const uint8_t& command);
	};

	std::vector<Command> commandLookup;

	void ExecuteCommand(const uint8_t& command);

	void sync(const uint8_t& command);
	void xxxxx(const uint8_t& command);
	void getStat(const uint8_t& command);
	void play(const uint8_t& command);
	void setLoc(const uint8_t& command);
	void readN(const uint8_t& command);
	void motorOn(const uint8_t& command);
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

	enum : uint8_t {
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

	union Status {
		uint8_t data;
		struct {
			uint8_t error : 1;
			uint8_t spindleMotor : 1;
			uint8_t seekError : 1;
			uint8_t idError : 1;
			uint8_t shellOpen : 1;
			uint8_t read : 1;
			uint8_t seek : 1;
			uint8_t play : 1;
		}elem;
	}stat;

	//Shell same as door??
	bool bShellOpen = false;

	uint8_t amm;
	uint8_t ass;
	uint8_t asect;
	uint8_t track;

	uint8_t stored_amm;
	uint8_t stored_ass;
	uint8_t stored_asect;

	uint8_t unprocessed_amm;
	uint8_t unprocessed_ass;
	uint8_t unprocessed_asect;

	void ReadSector(uint8_t& amm, uint8_t& ass, uint8_t& asect);
	void CancelReading();

	uint16_t sectorSize;

	union mode_t {
		uint8_t data;
		struct {
			uint8_t cdda : 1;
			uint8_t autoPause : 1;
			uint8_t report : 1;
			uint8_t xa_filter : 1;
			uint8_t ignoreBit : 1;
			uint8_t sectorSize : 1;
			uint8_t xa_adpcm : 1;
			uint8_t speed : 1;
		}elem;
	}mode;

	uint8_t filterFile;
	uint8_t filterChannel;

	bool bFilter = false;
	bool bPlay = false;
	bool bSendINT1 = true;
	char storedSectorHeader[8];
	bool bSetLocIssued = false;
	bool bSetLocUnprocessed = false;
	bool bMuted = true;

	struct Sector {
		uint16_t amm;
		uint16_t ass;
		uint16_t asect;
	};

	struct Track {
		uint8_t track;
		Sector trackStart;
		Sector trackEnd;
	};

	size_t fileSize;
	std::vector<Track> cdTracks;

	void AddSector(Sector& sector, int8_t mm, int8_t ss, int8_t sect) {
		if (sect < 0)
			if (ss == 0) {
				mm--;
				ss = 59;
				sect = 74;
			}
			else {
				ss--;
				sect = 74;
			}

		sector.asect += sect;
		sector.ass += (sector.asect / 75) + ss;
		sector.asect %= 75;
		sector.amm += (sector.ass / 60) + mm;
		sector.ass %= 60;
	}

	void ReadXaAdpcmSector(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src);

	void ReadCddaSector(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src);

	uint16_t tempVolumeLeftLeft = 0;
	uint16_t tempVolumeLeftRight = 0;
	uint16_t tempVolumeRightRight = 0;
	uint16_t tempVolumeRightLeft = 0;

	uint16_t volumeLeftLeft = 0;
	uint16_t volumeLeftRight = 0;
	uint16_t volumeRightRight = 0;
	uint16_t volumeRightLeft = 0;

	uint8_t* pNullSrc = (uint8_t*)calloc(0x930, sizeof(uint8_t));

	//debug
	DebugUtilities* p_debugger;


	void PrintDriveMode();
	void PrintStatus();
};