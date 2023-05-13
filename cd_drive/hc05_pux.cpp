#include "hc05_pux.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;
extern const char* g_cdPath;
extern uint8_t g_driveStatus;

hc05_pux::hc05_pux() {
	commandLookup.resize(20);
	commandLookup = {
		{"Sync?    ",&hc05_pux::xxxxx},{"Getstat",&hc05_pux::Getstat},{"Setloc   ",&hc05_pux::Setloc},{"Play    ",&hc05_pux::xxxxx},
		{"Forward  ",&hc05_pux::xxxxx},{"Backward ",&hc05_pux::xxxxx},{"ReadN     ",&hc05_pux::ReadN},{"MotorOn ",&hc05_pux::xxxxx},
		{"Stop     ",&hc05_pux::xxxxx},{"Pause    ",&hc05_pux::Pause},{"Init       ",&hc05_pux::Init},{"Mute    ",&hc05_pux::xxxxx},
		{"Demute  ",&hc05_pux::Demute},{"Setfilter",&hc05_pux::xxxxx},{"Setmode ",&hc05_pux::Setmode},{"Getparam",&hc05_pux::xxxxx},
		{"GetlocL  ",&hc05_pux::xxxxx},{"GetlocP  ",&hc05_pux::xxxxx},{"SetSession",&hc05_pux::xxxxx},{"GetTN   ",&hc05_pux::GetTN},
		{"GetTD    ",&hc05_pux::GetTD},{"SeekL    ",&hc05_pux::SeekL},{"SeekP     ",&hc05_pux::xxxxx},{"SetClock",&hc05_pux::xxxxx},
		{"GetClock ",&hc05_pux::xxxxx},{"Test      ",&hc05_pux::Test},{"GetID     ",&hc05_pux::GetID},{"ReadS   ",&hc05_pux::ReadS},
		{"Reset    ",&hc05_pux::xxxxx},{"GetQ     ",&hc05_pux::xxxxx},{"ReadTOC   ",&hc05_pux::xxxxx},{"VideoCD ",&hc05_pux::xxxxx},
	};
	driveStatus = static_cast<driveStatus_t>(g_driveStatus);
}


void hc05_pux::sendResponseINT(const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	responsesINT.push_back(responseINT);

	for (uint8_t i = 0; i < responseCount; i++) {
		responseFifo.push_back(pResponses[i]);
		std::cout << "~[CD_DRIVE] response sent: 0x" << std::hex << (uint16_t)pResponses[responseCount - i - 1] << std::endl;
	}
}

void hc05_pux::setFirstResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bFirstResponse = true;
	waitClocksFirstResponse = waitClocks;

	tempFirstResponseFifo.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempFirstResponseFifo.push_back(pResponses[responseCount - i]);
	}

	firstResponseINT = responseINT;
}

void hc05_pux::setSecondResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bSecondResponse = true;
	waitClocksSecondResponse = waitClocks;

	tempSecondResponseFifo.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempSecondResponseFifo.push_back(pResponses[responseCount - i]);
	}

	secondResponseINT = responseINT;
}

void hc05_pux::setDataResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bDataResponse = true;
	waitClocksDataResponse = waitClocks;

	tempDataResponseFifo.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempDataResponseFifo.push_back(pResponses[responseCount - i]);
	}

	dataResponseINT = responseINT;
}

void hc05_pux::clock() {
	//Set stat flags
	if (paramFifo.empty())
		indexStatus.reg.prmempt = 1;
	else
		indexStatus.reg.prmempt = 0;

	if (paramFifo.size() == 16)
		indexStatus.reg.prmwrdy = 0;
	else
		indexStatus.reg.prmwrdy = 1;

	if (responseFifo.empty())
		indexStatus.reg.rslrrdy = 0;
	else
		indexStatus.reg.rslrrdy = 1;

	if (bufferCounter == 0)
		indexStatus.reg.drqsts = 0;
	else
		indexStatus.reg.drqsts = 1;



	bool bDeliverResponseINT = !responsesINT.empty() && (irq_flag_read.data == 0);
	if (bDeliverResponseINT) {
		irq_flag_read.reg.int_1_7 = responsesINT[0];
		responsesINT.erase(responsesINT.begin());
		if (irq_flag_read.reg.int_1_7)
			std::cout << "~[CD_DRIVE] sent INT" << uint16_t(irq_flag_read.reg.int_1_7) << std::endl;
		if (irq_flag_read.reg.int_8)
			std::cout << "~[CD_DRIVE] sent INT8" << std::endl;
		if (irq_flag_read.reg.int_10)
			std::cout << "~[CD_DRIVE] sent INT10" << std::endl;

		//if (requestReg.reg.smen)
		pBus->pCp0->interruptHandler(_CD_ROM);
	}



	bool bExecuteCommand = responsesINT.empty() && (irq_flag_read.data == 0) && !commandFifo.empty()
		&& !bFirstResponse && !bSecondResponse;
	if (bExecuteCommand) {
		clocks = 0;

		executeCommand(commandFifo[0]);
		commandFifo.erase(commandFifo.begin());
		paramFifo.clear();//??
	}



	if (clocks >= (waitClocksFirstResponse+ 0x36cd2) && bFirstResponse) {
		bFirstResponse = false;
		clocks = 0;

		sendResponseINT(firstResponseINT, tempFirstResponseFifo.data(), tempFirstResponseFifo.size());
	}



	if (clocks >= (waitClocksSecondResponse+0x36cd2) && bSecondResponse &&
		!bFirstResponse && !bDataResponse && responsesINT.empty() && (irq_flag_read.data == 0)) {
		bSecondResponse = false;
		clocks = 0;

		sendResponseINT(secondResponseINT, tempSecondResponseFifo.data(), tempSecondResponseFifo.size());
	}



	if (clocks >= (waitClocksDataResponse+ 0x36cd2) && bDataResponse &&
		!bFirstResponse && !bSecondResponse && responsesINT.empty() && (irq_flag_read.data == 0)) {
		clocks = 0;

		sendResponseINT(dataResponseINT, tempDataResponseFifo.data(), tempDataResponseFifo.size());

		readSector(amm, ass, asect);
	}



	if (bSecondResponse || bFirstResponse || bDataResponse)
		clocks++;
}

void hc05_pux::ReadCdDrive8(const uint32_t& addr, uint8_t& data) {
	/*if (addr != 0x1f801802)
		std::cout << "              Read8 CD_rom 0x" << std::hex << addr << '\n';*/

	switch (addr) {
	case 0x1f801800:
		data = indexStatus.data;
		break;

	case 0x1f801801:
		data = responseFifo.back();
		responseFifo.pop_back();
		break;

	case 0x1f801802:
		data = buffer[sectorSize - bufferCounter];
		bufferCounter--;
		break;

	case 0x1f801803:
		switch (indexStatus.reg.index) {
		case 0:
		case 2:
			data = interruptEnable.data;
			break;
		case 1:
		case 3:
			data = 0xe0 | irq_flag_read.reg.int_1_7 | (irq_flag_read.reg.int_8 << 3) | (irq_flag_read.reg.int_10 << 4);
			break;
		default:
			break;
		}
		break;
	}
}

void hc05_pux::WriteCdDrive8(const uint32_t& addr, const uint8_t& data) {
	//std::cout << "                Write8 CD_rom 0x" << std::hex << addr << " " << "0x" << std::hex << (uint16_t)data << '\n';

	switch (addr) {
	case 0x1f801800:
		indexStatus.reg.index = data << 6 >> 6;
		break;

	case 0x1f801801:
		switch (indexStatus.reg.index) {
		case 0:
			commandFifo.push_back(data);
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
		break;

	case 0x1f801802:
		switch (indexStatus.reg.index) {
		case 0:
			paramFifo.push_back(data);
			break;
		case 1:
			interruptEnable.data = data;
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
		break;

	case 0x1f801803:
		switch (indexStatus.reg.index) {
		case 0:

			requestReg.data = data;
			if (data & 0x80) {
				//bufferCounter = sectorSize;
				std::cout << "~[CD_DRIVE] load data fifo\n";
			}
			else {
			//	bufferCounter = 0;
				//std::cout << "~[CD_DRIVE] reset data fifo, bufferCounter " << std::hex << "0x" << bufferCounter << std::endl;
			}

			break;
		case 1:

			if ((data & 0x7) == 0x7) {
				if (irq_flag_read.reg.int_1_7)
					std::cout << "~[CD_DRIVE] acknowledged INT" << uint16_t(irq_flag_read.reg.int_1_7) << std::endl;
				irq_flag_read.reg.int_1_7 = 0;
				//responseFifo.clear();
			}
			if (data & 0x8) {
				if (irq_flag_read.reg.int_8)
					std::cout << "~[CD_DRIVE] acknowledged INT8" << std::endl;
				irq_flag_read.reg.int_8 = 0;
				//responseFifo.clear();
			}
			if (data & 0x10) {
				if (irq_flag_read.reg.int_10)
					std::cout << "~[CD_DRIVE] acknowledged INT10" << std::endl;
				irq_flag_read.reg.int_10 = 0;
				//responseFifo.clear();
			}
			if (data & 0x40) {
				std::cout << "~[CD_DRIVE] parameter fifo reseted" << std::endl;
				paramFifo.clear();
			}
			if ((0x0 < (data & 0x7)) && ((data & 0x7) < 0x7)) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! unhandled Interrupt Flag Register (W) state 0x"
					<< std::hex << (uint16_t)data << std::endl;
				g_emulationPaused = true;
			}

			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
		break;
	}
}

void hc05_pux::executeCommand(const uint8_t& command) {
	//debug
	std::cout << "[CD_DRIVE] " << commandLookup[command].name << std::endl;

	//execute command
	(this->*commandLookup[command].command)(command);
}

void hc05_pux::readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect) {
	std::ifstream file(g_cdPath, std::ios::binary);
	if (file.is_open()) {

		if (sectorSize == 0x800)
			file.seekg(((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x18);
		else
			file.seekg(((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x0c);

		file.read((char*)buffer, sectorSize);

		std::cout << std::dec << "~[CD_DRIVE] amm: " << (uint16_t)amm << std::endl;
		std::cout << std::dec << "~[CD_DRIVE] ass: " << (uint16_t)ass << std::endl;
		std::cout << std::dec << "~[CD_DRIVE] asect: " << (uint16_t)asect << std::endl;

		asect++;
		if (asect == 75) {
			asect = 0;
			ass++;
		}
		if (ass == 60) {
			ass = 0;
			amm++;
		}
		//bufferCounter = 0;

		bufferCounter = sectorSize;
		file.close();
	}
	else
		throw std::runtime_error("Failed to open rom!\n");
}

uint8_t hc05_pux::fromHexToDec8(const uint8_t& hex) {
	return (hex >> 4) * 10 + (hex & 0x0f);
}

//Cd Drive Commands

uint8_t hc05_pux::xxxxx(const uint8_t& command) {
	std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled command!" << std::endl;
	g_emulationPaused = true;
	return 0;
}

uint8_t hc05_pux::Getstat(const uint8_t& command) {
	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
	return 0;
}

uint8_t hc05_pux::Setloc(const uint8_t& command) {
	uint8_t temp;

	temp = paramFifo[paramFifo.size() - 3];
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x9) || (temp >= 0x99)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock amm value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
	}
	amm = fromHexToDec8(temp);
	std::cout << "~[CD_DRIVE] amm: " << std::dec << (uint16_t)amm << std::endl;

	temp = paramFifo[paramFifo.size() - 2];
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x5) || (temp >= 0x60)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock ass value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
	}
	ass = fromHexToDec8(temp);
	std::cout << "~[CD_DRIVE] ass: " << std::dec << (uint16_t)ass << std::endl;

	temp = paramFifo[paramFifo.size() - 1];
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x7) || (temp >= 0x75)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
	}
	asect = fromHexToDec8(temp);
	std::cout << "~[CD_DRIVE] asect: " << std::dec << (uint16_t)asect << std::endl;

	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
	return 0;
}

uint8_t hc05_pux::ReadN(const uint8_t& command) {
	stat = stat | STAT_READ;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t dataResponse[1] = { stat };
	uint32_t dataWaitClocks = doubleSpeed ? 0x36cd2 : 0x6e1cd;
	setDataResponse(dataWaitClocks, INT1, dataResponse, 1);
	return 0;
}

uint8_t hc05_pux::Pause(const uint8_t& command) {
	bDataResponse = false;;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
	stat = stat & (~(STAT_READ | STAT_PLAY));
	uint8_t secondResponse[1] = { stat };
	uint32_t secondWaitClocks = doubleSpeed ? 0x10bd93 : 0x21181c;
	setSecondResponse(secondWaitClocks, INT2, secondResponse, 1);
	return 0;
}

uint8_t hc05_pux::Init(const uint8_t& command) {
	//mode 0x20
	sectorSize = 0x924;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0x13cce, INT3, firstResponse, 1);

	stat = STAT_NULL | STAT_SPINDLE_MOTOR;
	uint8_t secondResponse[1] = { stat };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
	return 0;
}

uint8_t hc05_pux::Demute(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0x5cce, INT3, firstResponse, 1);
	return 0;
}

uint8_t hc05_pux::Setmode(const uint8_t& command) {
	auto SetMode = [=] {
		uint8_t mode = paramFifo.back();
		sectorSize = (mode & 0x20) ? 0x924 : 0x800;
		doubleSpeed = (mode & 0x80) ? true : false;
		std::cout << std::hex << "~[CD_DRIVE] mode: 0x" << (uint16_t)mode << std::endl;
		std::cout << std::hex << "~[CD_DRIVE] doubleSpeed: 0x" << (uint16_t)(doubleSpeed) << std::endl;
		std::cout << std::hex << "~[CD_DRIVE] sector size: 0x" << (uint16_t)(sectorSize) << std::endl;
	};

	SetMode();

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
	return 0;
}

uint8_t hc05_pux::GetTN(const uint8_t& command) {
	std::vector<uint8_t> firstResponse = { stat, 0x1, 0x1 };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
	return 0;
}

uint8_t hc05_pux::GetTD(const uint8_t& command) {
	std::vector<uint8_t> firstResponse = { stat, amm, ass };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
	return 0;
}

uint8_t hc05_pux::SeekL(const uint8_t& command) {
	stat = stat | STAT_SEEK;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat = stat & (~STAT_SEEK);
	uint8_t secondResponse[1] = { stat };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
	return 0;
}

uint8_t hc05_pux::Test(const uint8_t& command) {
	switch (paramFifo.back()) {
	case 0x20: {
		std::vector<uint8_t> firstResponse = { 0x94, 0x09, 0x19, 0xc0 };
		setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
		break;
	}
	default:
		std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled test drive sub function\n";
		g_emulationPaused = true;
		break;
	}
	return 0;
}

uint8_t hc05_pux::GetID(const uint8_t& command) {
	std::vector<uint8_t> firstResponse = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());

	std::vector<uint8_t> secondResponse;
	responseINT_t secondINT;
	switch (driveStatus) {
	case NO_DISC:
		stat = STAT_NULL | STAT_ID_ERROR;
		secondResponse = { stat, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INT5;
		break;
	case UNLICENCED_DISC_MODE_1:
		stat = stat | STAT_ID_ERROR;
		secondResponse = { stat, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INT5;
		break;
	case LICENCED_MODE_2:
		secondResponse = { stat, 0x00, 0x20, 0x00, 0x53, 0x43, 0x45, 0x41 };
		secondINT = INT2;
		break;
	default:
		std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled drive status 0x"
			<< std::hex << driveStatus;
		g_emulationPaused = true;
		break;
	}

	setSecondResponse(0x4a00, secondINT, secondResponse.data(), secondResponse.size());
	return 0;
}

uint8_t hc05_pux::ReadS(const uint8_t& command) {
	stat = stat | STAT_READ;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t dataResponse[1] = { stat };
	uint32_t dataWaitClocks = doubleSpeed ? 0x36cd2 : 0x6e1cd;
	setDataResponse(dataWaitClocks, INT1, dataResponse, 1);
	return 0;
}