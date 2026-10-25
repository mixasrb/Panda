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

class CdDrive {
public:

	CdDrive();

	BusInterface* busPtr;

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
			uint8_t bank : 2;
			uint8_t adpbusy : 1;
			uint8_t prmempt : 1;
			uint8_t prmwrdy : 1;
			uint8_t rslrrdy : 1;
			uint8_t drqsts : 1;
			uint8_t busysts : 1;
		}elem;
	};

	IndexStatus indexStatus = { 0 };

	//Is it a FIFO, or it is just a register??
	std::queue<uint8_t> commandFifo;
	std::queue<uint8_t> paramFifo;
	void GetParameters(uint8_t* paramPtr, const uint8_t& paramCount);

	union RequestRegister {
		uint8_t data;
		struct {
			uint8_t : 5;
			uint8_t smen : 1;
			uint8_t bfwr : 1;
			uint8_t bfrd : 1;
		}elem;
	};
	RequestRegister requestReg = { 0 };

	//data fifo
	uint8_t* dataBufferPtr = nullptr;
	uint16_t bufferCounter = 0;
	std::vector<char> fileDataVect;

	std::queue<uint8_t> resultFifo;

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

	enum ResponseInterrupt :uint8_t {
		NO_INTERRUPT,
		INTERRUPT_1,
		INTERRUPT_2,
		INTERRUPT_3,
		INTERRUPT_4,
		INTERRUPT_5,
		INTERRUPT_6,
		INTERRUPT_7,
	};

	union IrqFlagRead {
		uint8_t data;
		struct {
			ResponseInterrupt int_1_7 : 3;
			uint8_t int_8 : 1;
			uint8_t int_10 : 1;
			uint8_t : 3;
		}elem;
	};
	IrqFlagRead irqFlagRead = { 0 };

	std::queue<ResponseInterrupt> pendingInterrupts;

	void SetResponseInterrupt(const ResponseInterrupt& pendingInterrupt);

	ResponseInterrupt firstResponseInterrupt;
	bool isFirstResponsePending = false;
	uint32_t waitClocksFirstResponse = 0;
	void SetFirstResult(const uint32_t& waitClocks, const ResponseInterrupt& responseInterrupt, 
		uint8_t* responsesPtr, const uint8_t& responseCount);

	ResponseInterrupt secondResponseInterrupt;
	bool isSecondResponsePending = false;
	uint32_t waitClocksSecondResponse = 0;
	void SetSecondResult(const uint32_t& waitClocks, const ResponseInterrupt& responseInterrupt, 
		uint8_t* responsesPtr, const uint8_t& responseCount);

	ResponseInterrupt dataResponseInterrupt;
	bool isDataResponseActive = false;
	uint32_t waitClocksDataResponse = 0;
	void SetDataResult(const ResponseInterrupt& responseInterrupt, 
		uint8_t* responsesPtr, const uint8_t& responseCount);

	struct Command {
		std::string name;
		void(CdDrive::* command)();
	};

	std::vector<Command> commandLookup;

	void ExecuteNextCommand();

	void sync();
	void xxxxx();
	void getStat();
	void play();
	void setLoc();
	void readN();
	void motorOn();
	void stop();
	void pause();
	void init();
	void mute();
	void demute();
	void setFilter();
	void setMode();
	void getParam();
	void getLocL();
	void getLocP();
	void getTN();
	void getTD();
	void seekL();
	void seekP();
	void test();
	void getID();
	void readS();
	void readTOC();

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
	bool isShellOpen = false;

	uint8_t amm;
	uint8_t ass;
	uint8_t asect;
	uint8_t track;

	uint8_t storedAmm;
	uint8_t storedAss;
	uint8_t storedAsect;

	uint8_t unprocessedAmm;
	uint8_t unprocessedAss;
	uint8_t unprocessedAsect;

	void ReadSector(uint8_t& amm, uint8_t& ass, uint8_t& asect);
	void CancelReading();

	uint16_t sectorSize;

	union Mode {
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

	uint8_t filteredFile;
	uint8_t filteredChannel;

	bool bFilter = false;
	bool bPlay = false;
	bool sendINT1 = true;
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

	uint8_t* emptySectorPtr = (uint8_t*)calloc(0x930, sizeof(uint8_t));

	//debug
	DebugUtilities* p_debugger;


	void PrintDriveMode();
	void PrintStatus();
};