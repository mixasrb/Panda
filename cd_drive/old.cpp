#include "hc05_pux.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;
extern const char* g_cdPath;
extern uint8_t g_driveStatus;

cdDrive::cdDrive() {
	//probably badly implemented commands Stop,GetTN,GetTD,Play,Setfilter,GetlocL,GetlocP,SeekP,Mute,ReadTOC
	commandLookup.resize(20);
	commandLookup = {
		{"Sync?    ",&cdDrive::xxxxx},{"Getstat"    ,&cdDrive::getStat},{"Setloc   ",&cdDrive::setLoc},{"Play     ",&cdDrive::play},
		{"Forward  ",&cdDrive::xxxxx},{"Backward     ",&cdDrive::xxxxx},{"ReadN     ",&cdDrive::readN},{"MotorOn ",&cdDrive::xxxxx},
		{"Stop      ",&cdDrive::stop},{"Pause        ",&cdDrive::pause},{"Init       ",&cdDrive::init},{"Mute     ",&cdDrive::mute},
		{"Demute  ",&cdDrive::demute},{"Setfilter",&cdDrive::setFilter},{"Setmode ",&cdDrive::setMode},{"Getparam",&cdDrive::getParam},
		{"GetlocL",&cdDrive::getLocL},{"GetlocP    ",&cdDrive::getLocP},{"SetSession",&cdDrive::xxxxx},{"GetTN   ",&cdDrive::getTN},
		{"GetTD    ",&cdDrive::getTD},{"SeekL        ",&cdDrive::seekL},{"SeekP     ",&cdDrive::seekP},{"SetClock",&cdDrive::xxxxx},
		{"GetClock ",&cdDrive::xxxxx},{"Test          ",&cdDrive::test},{"GetID     ",&cdDrive::getID},{"ReadS   ",&cdDrive::readS},
		{"Reset    ",&cdDrive::xxxxx},{"GetQ         ",&cdDrive::xxxxx},{"ReadTOC ",&cdDrive::readTOC},{"VideoCD ",&cdDrive::xxxxx},
	};

	stat = STAT_SPINDLE_MOTOR;

	driveStatus = static_cast<driveStatus_t>(g_driveStatus);


	std::string path = g_cdPath;

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Failed to open rom!\n");

	size_t fileSize = file.tellg();
	file.seekg(0);
	fileDataVect.resize(fileSize);
	file.read(fileDataVect.data(), fileSize);
	file.close();


	std::cout << path << std::endl;

	int i = 1;
	cdTrack_t currentTrack;
	currentTrack.track = i;
	currentTrack.index0Start.asect = 0;
	currentTrack.index0Start.ass = 0;
	currentTrack.index0Start.amm = 0;

	currentTrack.index0End.asect = 74;
	currentTrack.index0End.ass = 1;
	currentTrack.index0End.amm = 0;

	currentTrack.index1Start.asect = 0;
	currentTrack.index1Start.ass = 2;
	currentTrack.index1Start.amm = 0;


	const size_t sectorSize = 0x930;
	uint16_t sectorCount = fileSize / sectorSize;
	uint16_t temp_sect = sectorCount % 75;
	uint16_t secondsCount = sectorCount / 75;
	uint16_t temp_ss = secondsCount % 60;
	uint16_t temp_mm = secondsCount / 60;

	currentTrack.index1End = currentTrack.index1Start;
	addSector(currentTrack.index1End, temp_mm, temp_ss, temp_sect - 1);

	std::stringstream ss1;
	ss1 << "(Track 01).bin";
	std::stringstream ss2;
	ss2 << "(Track 1).bin";


	cdTracks.push_back(currentTrack);


	size_t found1 = path.find(ss1.str());
	size_t found2 = path.find(ss2.str());
	if ((found1 != std::string::npos) || (found2 != std::string::npos)) {
		int eraseCount;
		if (found1 != std::string::npos)
			eraseCount = 14;
		else
			eraseCount = 13;

		path.erase(path.end() - eraseCount, path.end());

		do {
			i++;
			std::string iStr = std::to_string(i);

			std::stringstream ss;
			std::ifstream file;
			if (found1 != std::string::npos) {
				ss << path.c_str() << "(Track " << ((i < 10) ? "0" : "") << iStr.c_str() << ").bin";
				file.open(ss.str(), std::ios::binary | std::ios::ate);
			}
			else {
				ss << path.c_str() << "(Track " << iStr.c_str() << ").bin";
				file.open(ss.str(), std::ios::binary | std::ios::ate);
			}

			if (file.is_open()) {

				size_t fileSizeTemp = file.tellg();
				fileSize = file.tellg();
				file.seekg(0);
				fileDataVect.resize(fileSize + fileSizeTemp);
				file.read(fileDataVect.data() + fileSize, fileSizeTemp);
				fileSize += fileSizeTemp;
				file.close();


				std::cout << ss.str() << std::endl;
				cdTrack_t perviousTrack = cdTracks[i - 2];

				cdTrack_t currentTrack;
				currentTrack.track = i;
				currentTrack.index0Start = perviousTrack.index1End;
				addSector(currentTrack.index0Start, 0, 0, 1);


				currentTrack.index0End = currentTrack.index0Start;
				addSector(currentTrack.index0End, 0, 1, 74);


				currentTrack.index1Start = currentTrack.index0End;
				addSector(currentTrack.index1Start, 0, 0, 1);

				uint16_t sectorCount = fileSizeTemp / sectorSize;
				uint16_t temp_sect = sectorCount % 75;
				uint16_t secondsCount = sectorCount / 75;
				uint16_t temp_ss = secondsCount % 60;
				uint16_t temp_mm = secondsCount / 60;

				currentTrack.index1End = currentTrack.index1Start;
				addSector(currentTrack.index1End, temp_mm, temp_ss, temp_sect - 1);


				cdTracks.push_back(currentTrack);
			}
			else
				break;

		} while (1);
	}
}

void cdDrive::readCdDrive8(const uint32_t& addr, uint8_t& data) {
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
		data = pBuffer[sectorSize - bufferCounter];
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
			data = 0xe0 | irqFlagRead.reg.int_1_7 | (irqFlagRead.reg.int_8 << 3) | (irqFlagRead.reg.int_10 << 4);
			break;
		default:
			break;
		}
		break;
	}
}

void cdDrive::writeCdDrive8(const uint32_t& addr, const uint8_t& data) {
	//std::cout << "                Write8 CD_rom 0x" << std::hex << addr << " " << "0x" << std::hex << (uint16_t)data << '\n';

	switch (addr) {
	case 0x1f801800:
		indexStatus.reg.index = data << 6 >> 6;
		break;

	case 0x1f801801:
		switch (indexStatus.reg.index) {
		case 0:
			commandVector.push_back(data);
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
			paramFifo.insert(paramFifo.begin(), data);
			std::cout << "~[CD_DRIVE] param 0x" << (uint16_t)data << std::endl;
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
				std::cout << "~[CD_DRIVE] load data fifo\n";
			}
			else {
				indexStatus.reg.drqsts = 0;
				std::cout << "~[CD_DRIVE] reset data fifo\n";
			}

			if (requestReg.reg.smen) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! unhandled SMEN 0x1f801803 index 0\n";
				g_emulationPaused = true;
			}

			break;
		case 1:

			if ((data & 0x7) == 0x7) {
				if (irqFlagRead.reg.int_1_7)
					std::cout << "~[CD_DRIVE] acknowledged INT" << (uint16_t)irqFlagRead.reg.int_1_7 << std::endl;
				irqFlagRead.reg.int_1_7 = 0;
			}
			if (data & 0x8) {
				if (irqFlagRead.reg.int_8)
					std::cout << "~[CD_DRIVE] acknowledged INT8" << std::endl;
				irqFlagRead.reg.int_8 = 0;
			}
			if (data & 0x10) {
				if (irqFlagRead.reg.int_10)
					std::cout << "~[CD_DRIVE] acknowledged INT10" << std::endl;
				irqFlagRead.reg.int_10 = 0;
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

void cdDrive::sendResponseINT(const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	responsesINT.push_back(responseINT);

	for (uint8_t i = 0; i < responseCount; i++) {
		responseFifo.push_back(pResponses[i]);
		std::cout << "~[CD_DRIVE] response sent: 0x" << std::hex << (uint16_t)pResponses[responseCount - i - 1] << std::endl;
	}
}

void cdDrive::setFirstResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bFirstResponse = true;
	waitClocksFirstResponse = waitClocks;

	tempFirstResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempFirstResponseVector.push_back(pResponses[responseCount - i]);
	}

	firstResponseINT = responseINT;
}

void cdDrive::setSecondResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bSecondResponse = true;
	waitClocksSecondResponse = waitClocks;

	tempSecondResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempSecondResponseVector.push_back(pResponses[responseCount - i]);
	}

	secondResponseINT = responseINT;
}

void cdDrive::setDataResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bDataResponse = true;
	waitClocksDataResponse = waitClocks;

	tempDataResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempDataResponseVector.push_back(pResponses[responseCount - i]);
	}

	dataResponseINT = responseINT;
}

void cdDrive::clock() {
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



	bool bDeliverResponseINT = !responsesINT.empty() && (irqFlagRead.data == 0);
	if (bDeliverResponseINT) {
		irqFlagRead.reg.int_1_7 = responsesINT[0];
		responsesINT.erase(responsesINT.begin());
		if (irqFlagRead.reg.int_1_7)
			std::cout << "~[CD_DRIVE] sent INT" << uint16_t(irqFlagRead.reg.int_1_7) << " flag" << std::endl;
		if (irqFlagRead.reg.int_8)
			std::cout << "~[CD_DRIVE] sent INT8" << " flag" << std::endl;
		if (irqFlagRead.reg.int_10)
			std::cout << "~[CD_DRIVE] sent INT10" << " flag" << std::endl;

		if ((interruptEnable.reg.enableBits & 0x7) >= irqFlagRead.reg.int_1_7) {
			pBus->pCpu->cp0.interruptHandler(_IRQ_CD_ROM);
			std::cout << "~[CD_DRIVE] sent IRQ" << std::endl;
		}
	}



	bool bExecuteCommand = responsesINT.empty() && (irqFlagRead.data == 0) && !commandVector.empty()
		&& !bFirstResponse && !bSecondResponse;
	if (bExecuteCommand) {
		clocks = 0;

		executeCommand(commandVector[0]);
		commandVector.erase(commandVector.begin());
	}



	if (clocks >= waitClocksFirstResponse && bFirstResponse) {
		bFirstResponse = false;
		clocks = 0;

		sendResponseINT(firstResponseINT, tempFirstResponseVector.data(), tempFirstResponseVector.size());
	}



	if (clocks >= (waitClocksSecondResponse + 0x36cd2 * 1 / 10) && bSecondResponse &&
		!bFirstResponse && responsesINT.empty() && (irqFlagRead.data == 0)) {
		bSecondResponse = false;
		clocks = 0;

		sendResponseINT(secondResponseINT, tempSecondResponseVector.data(), tempSecondResponseVector.size());
	}



	//if (dataClocks >= (waitClocksDataResponse + 0x36cd2 * 9 / 4) && bDataResponse &&
		//if (dataClocks >= (mode.elem.speed ? 0x3ab2b*3 : 0x72732*2) && bDataResponse &&
	if (dataClocks >= (waitClocksDataResponse + 0x36cd2 * 15 / 10) && bDataResponse &&            //castlevania SOF requires 15/10 or less??
		!bFirstResponse && !bSecondResponse && responsesINT.empty() && (irqFlagRead.data == 0)) {
		dataClocks = 0;

		readSector(amm, ass, asect);

		if (bSendINT1)
			sendResponseINT(dataResponseINT, tempDataResponseVector.data(), tempDataResponseVector.size());

		//if (mode.elem.autoPause)
		//	executeCommand(0x9);
	}



	if (bSecondResponse || bFirstResponse)
		clocks++;
	if (bDataResponse)
		dataClocks++;
}

void cdDrive::executeCommand(const uint8_t& command) {
	//debug
	std::cout << "[CD_DRIVE] " << commandLookup[command].name << std::endl;

	//execute command
	(this->*commandLookup[command].command)(command);
}

void cdDrive::readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect) {
	if (stat & STAT_SEEK)
		stat = (stat ^ STAT_SEEK);

	if (bPlay)
		stat |= STAT_PLAY;
	else
		stat |= STAT_READ;

	if ((ass < 2) && (amm == 0)) {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector!" << std::endl;
		//g_emulationPaused = true;
		return;
	}

	if (fileDataVect.size() < (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x930)) {
		//if (stat & STAT_PLAY) { //??
		//	bufferCounter = sectorSize;
		//	pBuffer = (uint8_t*)fileDataVect.data();
		//}
		//else {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector!" << std::endl;
		//g_emulationPaused = true;
	//}
		return;
	}

	/*if (bFilter) {
		if ((bufferVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x10] == file) &&
			(bufferVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x11] == channel)) {

			pBuffer = (uint8_t*)bufferVect.data() + (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + (mode.elem.sectorSize ? 0x0c : 0x18));

			for (int i = 0; i < 8; i++) {
				storedSectorHeader[i] = bufferVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x0c + i];
			}

			stored_amm = amm;
			stored_ass = ass;
			stored_asect = asect;
		}
	}
	else {*/

	bufferCounter = sectorSize;

	if ((bFilter && mode.elem.xa_adpcm && mode.elem.xa_Filter && (fileDataVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x12] & 0x04))
		|| ((stat & STAT_PLAY) && (mode.elem.report == 0)))
		bSendINT1 = false;
	else
		bSendINT1 = true;

	/*if (!(bFilter && mode.elem.xa_adpcm && mode.elem.xa_Filter && (bufferVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x12] & 0x04))) {
		pBuffer = (uint8_t*)bufferVect.data() + (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + (mode.elem.sectorSize ? 0x0c : 0x18));

		for (int i = 0; i < 8; i++) {
			storedSectorHeader[i] = bufferVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x0c + i];
		}

		stored_amm = amm;
		stored_ass = ass;
		stored_asect = asect;
	}*/



	pBuffer = (uint8_t*)fileDataVect.data() + (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + (mode.elem.sectorSize ? 0x0c : 0x18));

	for (int i = 0; i < 8; i++) {
		storedSectorHeader[i] = fileDataVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x0c + i];
	}

	stored_amm = amm;
	stored_ass = ass;
	stored_asect = asect;



	if ((mode.elem.xa_adpcm) && (fileDataVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x12] & 0x4))
		indexStatus.reg.adpbusy = 1;


	std::cout << "~[CD_DRIVE] Read sector:" << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] amm: " << (uint16_t)amm << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] ass: " << (uint16_t)ass << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] asect: " << (uint16_t)asect << std::endl;
	switch (((uint32_t)fileDataVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x12] & 0x0f) >> 1) {
	case 0x1:
		std::cout << "~[CD_DRIVE] video sector" << std::endl;
		break;
	case 0x2:
		std::cout << "~[CD_DRIVE] audio sector" << std::endl;
		break;
	case 0x4:
		std::cout << "~[CD_DRIVE] data sector" << std::endl;
		break;
	default:
		std::cout << "[CD_DRIVE] EMULATION PAUSED! not recognized CD sector!" <<
			std::hex << " 0x" << (((uint32_t)fileDataVect[((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x12] & 0x0f) >> 1) << std::endl;
		//g_emulationPaused = true;
		break;
	}

	asect++;
	if (asect == 75) {
		asect = 0;
		ass++;
	}
	if (ass == 60) {
		ass = 0;
		amm++;
	}
}

void cdDrive::set_cdDriveMode() {
	sectorSize = mode.elem.sectorSize ? 0x924 : 0x800;

	if (mode.elem.xa_adpcm == 0)
		indexStatus.reg.adpbusy = 0;

	std::cout << std::hex << "~[CD_DRIVE] mode: " << (uint16_t)mode.data << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] Speed: " << (uint16_t)mode.elem.speed << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] XA-ADCPM: " << (uint16_t)mode.elem.xa_adpcm << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] Sector size: " << (uint16_t)mode.elem.sectorSize << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] Ignore bit: " << (uint16_t)mode.elem.ignoreBit << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] XA-Filter: " << (uint16_t)mode.elem.xa_Filter << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] Report: " << (uint16_t)mode.elem.report << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] AutoPause: " << (uint16_t)mode.elem.autoPause << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] CDDA: " << (uint16_t)mode.elem.cdda << std::endl;
}

static uint8_t fromBCDToHex8(const uint8_t& dec) {
	return (dec >> 4) * 10 + (dec & 0x0f);
}

static uint8_t fromHex8ToBCD(const uint8_t& hex) {
	return (hex / 10) * 16 + (hex % 10);
}

//Cd Drive Commands

void cdDrive::xxxxx(const uint8_t& command) {
	std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled command!" << std::endl;
	g_emulationPaused = true;
}

void cdDrive::getStat(const uint8_t& command) {
	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::play(const uint8_t& command) {
	bPlay = true;

	if (stat & STAT_SEEK)
		stat = (stat ^ STAT_SEEK) | STAT_PLAY;
	else
		stat |= STAT_SEEK;


	//parameter track
	if (!paramFifo.empty())
		paramFifo.pop_back();

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);


	uint8_t tempStat = 0;

	if (stat & STAT_SEEK)
		tempStat = (stat ^ STAT_SEEK);

	tempStat |= STAT_PLAY;

	uint8_t dataResponse[1] = { tempStat };
	uint32_t dataWaitClocks = mode.elem.speed ? 0x3ab2b : 0x72732;
	setDataResponse(dataWaitClocks, INT1, dataResponse, 1);
}

void cdDrive::setLoc(const uint8_t& command) {
	uint8_t temp;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x9) || (temp >= 0x99)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock amm value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	amm = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] amm: " << std::dec << (uint16_t)amm << std::endl;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x5) || (temp >= 0x60)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock ass value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	ass = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] ass: " << std::dec << (uint16_t)ass << std::endl;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x7) || (temp >= 0x75)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	asect = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] asect: " << std::dec << (uint16_t)asect << std::endl;

	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::readN(const uint8_t& command) {
	if (stat & STAT_SEEK)
		stat = (stat ^ STAT_SEEK) | STAT_READ;
	else
		stat |= STAT_SEEK;

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t tempStat = 0;

	if (stat & STAT_SEEK)
		tempStat = (stat ^ STAT_SEEK);

	tempStat |= STAT_READ;

	uint8_t dataResponse[1] = { tempStat };
	uint32_t dataWaitClocks = mode.elem.speed ? 0x3ab2b : 0x72732;
	setDataResponse(dataWaitClocks, INT1, dataResponse, 1);
}

void cdDrive::stop(const uint8_t& command) {
	bDataResponse = false;//??
	bPlay = false; //??
	stat ^= STAT_READ;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
	stat ^= STAT_SPINDLE_MOTOR;
	uint8_t secondResponse[1] = { stat };
	uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
	setSecondResponse(secondWaitClocks, INT2, secondResponse, 1);
}

void cdDrive::pause(const uint8_t& command) {
	bDataResponse = false;
	bPlay = false;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
	stat = stat & (~(STAT_READ | STAT_PLAY));
	uint8_t secondResponse[1] = { stat };
	uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
	setSecondResponse(secondWaitClocks, INT2, secondResponse, 1);
}

void cdDrive::init(const uint8_t& command) {
	bDataResponse = false;//??
	bPlay = false;//??
	mode.data = 0x20;
	set_cdDriveMode();

	sectorSize = 0x924;
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0x13cce, INT3, firstResponse, 1);

	stat = STAT_NULL | STAT_SPINDLE_MOTOR;
	uint8_t secondResponse[1] = { stat };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::mute(const uint8_t& command) {
	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::demute(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0x5cce, INT3, firstResponse, 1);
}

void cdDrive::setFilter(const uint8_t& command) {
	bFilter = true;

	uint8_t response[1] = { stat };
	setFirstResponse(0xc4e1, INT3, response, 1);
	file = paramFifo.back();
	paramFifo.pop_back();
	std::cout << std::hex << "~[CD_DRIVE] file: 0x" << (uint16_t)file << std::endl;
	channel = paramFifo.back();
	paramFifo.pop_back();
	std::cout << std::hex << "~[CD_DRIVE] channel: 0x" << (uint16_t)channel << std::endl;
}

void cdDrive::setMode(const uint8_t& command) {
	mode.data = paramFifo.back();
	paramFifo.pop_back();
	set_cdDriveMode();

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
}

void cdDrive::getParam(const uint8_t& command) {
	uint8_t firstResponse[5] = { stat, mode.data, 0x00, 0x01, 0x01 };
	setFirstResponse(0xc4e1, INT3, firstResponse, 5);
}

void cdDrive::getLocL(const uint8_t& command) {
	setFirstResponse(0xc4e1, INT3, (uint8_t*)storedSectorHeader, 8);
}

void cdDrive::getLocP(const uint8_t& command) {
	std::vector<uint8_t> firstResponse = { 0x1, 0x1, fromHex8ToBCD(amm), fromHex8ToBCD(ass), fromHex8ToBCD(asect),
		fromHex8ToBCD(amm), fromHex8ToBCD(ass), fromHex8ToBCD(asect) };
	/*std::vector<uint8_t> firstResponse = { 0x1, 0x1, fromHex8ToDec(amm), fromHex8ToDec(ass), fromHex8ToDec(asect),
		fromHex8ToDec(amm), fromHex8ToDec(ass), fromHex8ToDec(asect) };*/
		/*std::vector<uint8_t> firstResponse = { 0x1, 0x1, 0,0,0,0,0,0 };*/
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
}

void cdDrive::getTN(const uint8_t& command) {
	uint8_t firstTrack = 1;
	uint8_t lastTrack = (uint8_t)cdTracks.size();
	std::vector<uint8_t> firstResponse = { stat, fromHex8ToBCD(firstTrack), fromHex8ToBCD(lastTrack) };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());

	//Gran Turismo
	//std::vector<uint8_t> firstResponse = { stat, 0x1, 0x1 };
	// 
	//WipEout 2097 (Europe)
	//std::vector<uint8_t> firstResponse = { stat, 0x1, 0x12 };
	//std::vector<uint8_t> firstResponse = { stat, 0x1, 0x23 };
	//mortal kombat 3
	//std::vector<uint8_t> firstResponse = { stat, 0x1, 0x63 };
	//old
	//std::vector<uint8_t> firstResponse = { stat, 0x1, 0x23 };
}

void cdDrive::getTD(const uint8_t& command) {
	std::vector<uint8_t> firstResponse;

	uint8_t track = fromBCDToHex8(paramFifo[0]);
	paramFifo.pop_back();

	if (track > cdTracks.size()) {
		uint8_t errorCode = 0x10;
		firstResponse = { errorCode };
		setFirstResponse(0xc4e1, INT5, firstResponse.data(), firstResponse.size());

		std::cout << std::hex << "~[CD_DRIVE] GetTD incorrect track number: " << std::dec << (uint16_t)track << std::endl;
		g_emulationPaused = true;
		return;
	}

	uint8_t ass;
	uint8_t amm;

	if (track == 0x00) {
		ass = cdTracks[cdTracks.size() - 1].index1End.ass;
		amm = cdTracks[cdTracks.size() - 1].index1End.amm;
	}
	//if (track == 0x00) {
	//	ass = 0;
	//	amm = 0;
	//}
	else {
		ass = cdTracks[track - 1].index1Start.ass;
		amm = cdTracks[track - 1].index1Start.amm;
	}

	firstResponse = { stat, fromHex8ToBCD(amm), fromHex8ToBCD(ass) };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());

	//mortal kombat 3
//std::vector<uint8_t> firstResponse = { stat, 4, 15 };

	//WipEout 2097 (Europe)
	//firstResponse = { stat, 0x0, 0x2 };

	//Gran Turismo
	//if (paramFifo[0] == 1)
	//	firstResponse = { stat, 0x0, 0x0 };
	//if (paramFifo[0] == 0)
	//	firstResponse = { stat, 0x63, 0x45 };

	//old
	//std::vector<uint8_t> firstResponse = { stat, 20, 40 };

}

void cdDrive::seekL(const uint8_t& command) {
	//stat &= (STAT_READ ^ STAT_READ) | (STAT_PLAY ^ STAT_PLAY);//!!??
	stat = stat | STAT_SEEK;

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat = stat & (~STAT_SEEK);
	uint8_t secondResponse[1] = { stat };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::seekP(const uint8_t& command) {
	stat = stat | STAT_SEEK;

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat = stat & (~STAT_SEEK);
	uint8_t secondResponse[1] = { stat };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::test(const uint8_t& command) {
	switch (paramFifo.back()) {
	case 0x04: {
		stat = stat | STAT_SPINDLE_MOTOR;
		std::vector<uint8_t> firstResponse = { stat };
		setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
		break;
	}
	case 0x05: {
		std::vector<uint8_t> firstResponse = { 0x20, 0x00 }; //crash bash skip
		//std::vector<uint8_t> firstResponse = { 0x01, 0x01 };
		setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
		break;
	}
	case 0x20: {
		std::vector<uint8_t> firstResponse = { 0x94, 0x09, 0x19, 0xc0 };
		setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
		break;
	}
	default:
		std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled test drive sub function 0x" << std::hex << (uint16_t)paramFifo.back() << "\n";
		g_emulationPaused = true;
		break;
	}
	paramFifo.pop_back();
}

void cdDrive::getID(const uint8_t& command) {
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
}

void cdDrive::readS(const uint8_t& command) {
	if (stat & STAT_SEEK)
		stat = (stat ^ STAT_SEEK) | STAT_READ;
	else
		stat |= STAT_SEEK;

	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t tempStat = 0;

	if (stat & STAT_SEEK)
		tempStat = (stat ^ STAT_SEEK);

	tempStat |= STAT_READ;

	uint8_t dataResponse[1] = { tempStat };
	uint32_t dataWaitClocks = mode.elem.speed ? 0x3ab2b : 0x72732;
	setDataResponse(dataWaitClocks, INT1, dataResponse, 1);
}

void cdDrive::readTOC(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t secondResponse[10] = { stat };
	setSecondResponse(44100 * 0x300, INT2, secondResponse, 1);
}
