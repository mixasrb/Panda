#include "hc05_pux.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;
extern const char* g_cdPath;
extern uint8_t g_driveStatus;

cdDrive::cdDrive() {
	commandLookup.resize(20);
	commandLookup = {
		{"Sync      ",&cdDrive::sync},{"Getstat"    ,&cdDrive::getStat},{"Setloc   ",&cdDrive::setLoc},{"Play     ",&cdDrive::play},
		{"Forward  ",&cdDrive::xxxxx},{"Backward     ",&cdDrive::xxxxx},{"ReadN     ",&cdDrive::readN},{"MotorOn ",&cdDrive::motorOn},
		{"Stop      ",&cdDrive::stop},{"Pause        ",&cdDrive::pause},{"Init       ",&cdDrive::init},{"Mute     ",&cdDrive::mute},
		{"Demute  ",&cdDrive::demute},{"Setfilter",&cdDrive::setFilter},{"Setmode ",&cdDrive::setMode},{"Getparam",&cdDrive::getParam},
		{"GetlocL",&cdDrive::getLocL},{"GetlocP    ",&cdDrive::getLocP},{"SetSession",&cdDrive::xxxxx},{"GetTN   ",&cdDrive::getTN},
		{"GetTD    ",&cdDrive::getTD},{"SeekL        ",&cdDrive::seekL},{"SeekP     ",&cdDrive::seekP},{"SetClock",&cdDrive::xxxxx},
		{"GetClock ",&cdDrive::xxxxx},{"Test          ",&cdDrive::test},{"GetID     ",&cdDrive::getID},{"ReadS   ",&cdDrive::readS},
		{"Reset    ",&cdDrive::xxxxx},{"GetQ         ",&cdDrive::xxxxx},{"ReadTOC ",&cdDrive::readTOC},{"VideoCD ",&cdDrive::xxxxx},
	};

	driveStatus = static_cast<driveStatus_t>(g_driveStatus);


	std::string path = g_cdPath;

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cout << "Failed to open rom!\n";
		g_emulationPaused = true;
	}

	size_t fileSize = file.tellg();
	file.seekg(0);
	fileDataVect.resize(fileSize);
	file.read(fileDataVect.data(), fileSize);
	file.close();


	std::cout << path << std::endl;

	int i = 1;
	cdTrack_t currentTrack;
	currentTrack.track = i;

	currentTrack.index1Start.asect = 0;
	currentTrack.index1Start.ass = 2;
	currentTrack.index1Start.amm = 0;


	const uint32_t sectorSize = 0x930;
	uint32_t sectorCount = fileSize / sectorSize;
	uint32_t temp_sect = sectorCount % 75;
	uint32_t secondsCount = sectorCount / 75;
	uint32_t temp_ss = secondsCount % 60;
	uint32_t temp_mm = secondsCount / 60;

	currentTrack.index1End = currentTrack.index1Start;
	addSector(currentTrack.index1End, (uint16_t)temp_mm, (uint16_t)temp_ss, (uint16_t)temp_sect - 1);

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
				fileSizeTemp = file.tellg();
				file.seekg(0);
				fileDataVect.resize(fileSize + fileSizeTemp);
				file.read(fileDataVect.data() + fileSize, fileSizeTemp);
				fileSize += fileSizeTemp;
				file.close();


				std::cout << ss.str() << std::endl;
				cdTrack_t perviousTrack = cdTracks[i - 2];

				cdTrack_t currentTrack;
				currentTrack.track = i;
				/*currentTrack.index0Start = perviousTrack.index1End;
				addSector(currentTrack.index0Start, 0, 0, 1);


				currentTrack.index0End = currentTrack.index0Start;
				addSector(currentTrack.index0End, 0, 1, 74);*/


				currentTrack.index1Start = perviousTrack.index1End;
				addSector(currentTrack.index1Start, 0, 0, 1);

				uint32_t sectorCount = fileSizeTemp / sectorSize;
				uint32_t temp_sect = sectorCount % 75;
				uint32_t secondsCount = sectorCount / 75;
				uint32_t temp_ss = secondsCount % 60;
				uint32_t temp_mm = secondsCount / 60;

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
	switch (addr) {
	case 0x1f801800:
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

		data = indexStatus.data;
		break;

	case 0x1f801801:
		data = responseFifo.back();

		std::cout << "~[CD_DRIVE] response read: 0x" << std::hex << (uint16_t)data << std::endl;
		responseFifo.pop_back();
		break;

	case 0x1f801802:
		data = pBuffer[sectorSize - bufferCounter--];
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
		}
		break;
	}
}

void cdDrive::writeCdDrive8(const uint32_t& addr, const uint8_t& data) {
	switch (addr) {
	case 0x1f801800:
		indexStatus.reg.index = data << 6 >> 6;
		break;

	case 0x1f801801:
		switch (indexStatus.reg.index) {
		case 0:
			commandVector.push_back(data);
			std::cout << "~[CD_DRIVE] command sent : " << commandLookup[data].name << std::endl;
			break;
		case 1:
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! 1F801801h.Index1 - Sound Map Data Out (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;
			g_emulationPaused = true;
			break;
		case 2:
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! 1F801801h.Index2 - Sound Map Coding Info (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;
			g_emulationPaused = true;
			break;
		case 3:
			/*std::cout << "~[CD_DRIVE] Unhandled 1F801801h.Index3 - Audio Volume for Right-CD-Out to Right-SPU-Input (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;*/
			tempVolumeRightRight = (uint16_t)data;
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
			/*std::cout << "~[CD_DRIVE] Unhandled 1F801802h.Index2 - Audio Volume for Left-CD-Out to Left-SPU-Input (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;*/
			tempVolumeLeftLeft = (uint16_t)data;
			break;
		case 3:
			/*std::cout << "~[CD_DRIVE] Unhandled 1F801802h.Index3 - Audio Volume for Right-CD-Out to Left-SPU-Input (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;*/
			tempVolumeRightLeft = (uint16_t)data;
			break;
		}
		break;

	case 0x1f801803:
		switch (indexStatus.reg.index) {
		case 0:

			requestReg.data = data;

			if (requestReg.reg.smen) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! unhandled SMEN 0x1f801803 index 0\n";
				g_emulationPaused = true;
			}

			if (requestReg.reg.bfwr) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! unhandled BFWR 0x1f801803 index 0\n";
				g_emulationPaused = true;
			}

			if (requestReg.reg.bfrd) {
				//bufferCounter = sectorSize; //!?
				indexStatus.reg.drqsts = 1; //!?
				std::cout << "~[CD_DRIVE] load data fifo\n";
			}
			else {
				indexStatus.reg.drqsts = 0;
				std::cout << "~[CD_DRIVE] reset data fifo\n";
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
			/*std::cout << "~[CD_DRIVE] Unhandled 1F801803h.Index2 - Audio Volume for Left-CD-Out to Right-SPU-Input (W) 0x"
				<< std::hex << (uint16_t)data << std::endl;*/
			tempVolumeLeftRight = (uint16_t)data;
			break;
		case 3:
			/*std::cout << "~[CD_DRIVE] Unhandled 1F801803h.Index3 - Audio Volume Apply Changes (by writing bit5=1) 0x"
				<< std::hex << (uint16_t)data << std::endl;*/

			if (data & 0x01) {
				if (mode.elem.xa_adpcm) {
					volumeLeftLeft = 0;
					volumeLeftRight = 0;
					volumeRightRight = 0;
					volumeRightLeft = 0;
				}
			}
			else if (data & 0x20) {
				volumeLeftLeft = tempVolumeLeftLeft;
				volumeLeftRight = tempVolumeLeftRight;
				volumeRightRight = tempVolumeRightRight;
				volumeRightLeft = tempVolumeRightLeft;
			}

			break;
		}
		break;
	}
}

void cdDrive::sendResponseINT(const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	responsesINT.push_back(responseINT);
	;
	for (uint8_t i = 0; i < responseCount; i++) {
		responseFifo.push_back(pResponses[i]);
		std::cout << "~[CD_DRIVE] response sent: 0x" << std::hex << (uint16_t)pResponses[responseCount - i - 1] << std::endl;
	}
}

void cdDrive::setFirstResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bFirstResponse = true;
	waitClocksFirstResponse = waitClocks;

	//debug
	std::cout << "~[CD_DRIVE] first response" << std::endl;
	printStat();

	//tempFirstResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempFirstResponseVector.push_back(pResponses[responseCount - i]);
	}

	firstResponseINT = responseINT;
}

void cdDrive::setSecondResponse(const uint32_t& waitClocks, const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {
	bSecondResponse = true;
	waitClocksSecondResponse = waitClocks;

	//debug
	std::cout << "~[CD_DRIVE] second response" << std::endl;
	printStat();

	tempSecondResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempSecondResponseVector.push_back(pResponses[responseCount - i]);
	}

	secondResponseINT = responseINT;
}

void cdDrive::setDataResponse(const responseINT_t& responseINT, uint8_t* pResponses, const uint8_t& responseCount) {

	tempDataResponseVector.clear();
	for (uint8_t i = 1; i <= responseCount; i++) {
		tempDataResponseVector.push_back(pResponses[responseCount - i]);
	}

	dataResponseINT = responseINT;
}

static uint8_t fromBCDToHex8(const uint8_t& dec) {
	return (dec >> 4) * 10 + (dec & 0x0f);
}

static uint8_t fromHex8ToBCD(const uint8_t& hex) {
	return (hex / 10) * 16 + (hex % 10);
}

void cdDrive::clock() {

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

		indexStatus.reg.busysts = 1;
		executeCommand(commandVector[0]);
		commandVector.erase(commandVector.begin());
	}

	if (bFirstResponse || bSecondResponse || bDataResponse) {

		if (clocks >= waitClocksFirstResponse /*+ 0x36cd2 * 5 / 20*/ && bFirstResponse) {
			bFirstResponse = false;
			clocks = 0;

			indexStatus.reg.busysts = 0;
			sendResponseINT(firstResponseINT, tempFirstResponseVector.data(), tempFirstResponseVector.size());
			tempFirstResponseVector.clear();
		}
		// Doom
		else if (clocks >= (waitClocksSecondResponse /*+ 0x36cd2 * 1 / 10*/) && bSecondResponse &&
			!bFirstResponse && responsesINT.empty() && (irqFlagRead.data == 0)) {
			bSecondResponse = false;
			clocks = 0;

			sendResponseINT(secondResponseINT, tempSecondResponseVector.data(), tempSecondResponseVector.size());
			tempFirstResponseVector.clear();
		}
		//if (dataClocks >= (waitClocksDataResponse + 0x36cd2 * 9 / 4) && bDataResponse &&
			//if (dataClocks >= (mode.elem.speed ? 0x3ab2b*3 : 0x72732*2) && bDataResponse &&
																		 //bugs bunny games also  resident evil 1 ??
		else if (dataClocks >= (waitClocksDataResponse /*+ 0x36cd2 * 14 / 10*/ /*+ 0x36cd2 * 20 / 10*/) && bDataResponse &&   //castlevania SOF requires 15/10 or less??
			!bFirstResponse && !bSecondResponse && responsesINT.empty() && (irqFlagRead.data == 0)) {
			dataClocks = 0;

			readSector(amm, ass, asect);

			//debug
			std::cout << "~[CD_DRIVE] data response" << std::endl;
			printStat();

			if (bSendINT1) {
				stat.elem.seek = 0;
				if (!bPlay) {
					uint8_t dataResponse[1] = { stat.data };
					setDataResponse(INT1, dataResponse, 1);
				}
				else { // Play Report	                                                                                        
					uint8_t dataResponse[8] = { stat.data, track, 0x1, fromHex8ToBCD(pBus->spu.ammCDDA), fromHex8ToBCD(pBus->spu.assCDDA),
						fromHex8ToBCD(pBus->spu.asectCDDA), 0x7f, 0x00 };//??

					//uint8_t dataResponse[8] = { stat.data, track, 0x1, fromHex8ToBCD(stored_amm),
					//	fromHex8ToBCD(stored_ass), fromHex8ToBCD(stored_asect), 0x7f, 0x00 };//??
					//peaklo,peakhi??

					setDataResponse(INT1, dataResponse, 8);
				}
				sendResponseINT(dataResponseINT, tempDataResponseVector.data(), tempDataResponseVector.size());
				tempFirstResponseVector.clear();
			}
		}


		if (bSecondResponse || bFirstResponse)
			clocks++;
		if (bDataResponse)
			dataClocks++;
	}
}

void cdDrive::executeCommand(const uint8_t& command) {
	//debug
	std::cout << "[CD_DRIVE] " << commandLookup[command].name << std::endl;

	//execute command
	(this->*commandLookup[command].command)(command);
}

void cdDrive::readSector(uint8_t& amm, uint8_t& ass, uint8_t& asect) {
	stat.elem.seek = 0;

	if (bPlay) {
		stat.elem.read = 0;
		stat.elem.play = 1;
	}
	else {
		stat.elem.read = 1;
		stat.elem.play = 0;
	}

	uint32_t currentPossition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;

	if ((ass < 2) && (amm == 0)) {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector ass < 2!" << std::endl;
		g_emulationPaused = true;
		return;
	}

	if (fileDataVect.size() < (currentPossition + 0x930)) {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector!" << std::endl;
		g_emulationPaused = true;
		return;
	}

	if ((bFilter && mode.elem.xa_adpcm && mode.elem.xa_filter && (fileDataVect[currentPossition + 0x12] & 0x04))
		|| (stat.elem.play && (mode.elem.report == 0)))
		bSendINT1 = false;
	else if (!bPlay) {
		bSendINT1 = true;
		pBuffer = (uint8_t*)fileDataVect.data() + (currentPossition + (mode.elem.sectorSize ? 0x0c : 0x18));
	}

	if (!bPlay)
		for (int i = 0; i < 8; i++)
			storedSectorHeader[i] = fileDataVect[currentPossition + 0x0c + i];

	stored_amm = amm;
	stored_ass = ass;
	stored_asect = asect;

	bufferCounter = sectorSize;

	if ((mode.elem.xa_adpcm) && (fileDataVect[currentPossition + 0x12] & 0x4))
		indexStatus.reg.adpbusy = 1;

	waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
	//waitClocksDataResponse += 0x36cd2 * 10 / 10;
	std::cout << "~[CD_DRIVE] Read sector:" << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] amm: " << (uint16_t)amm << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] ass: " << (uint16_t)ass << std::endl;
	std::cout << std::dec << "~[CD_DRIVE] asect: " << (uint16_t)asect << std::endl;

	if (!bPlay)
		switch (((uint32_t)fileDataVect[currentPossition + 0x12] & 0x0f) >> 1 << 1) {
		case 0x0:
			std::cout << "~[CD_DRIVE] unused sector" << std::endl;
			waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
			break;
		case 0x2:
			std::cout << "~[CD_DRIVE] video sector" << std::endl;
			waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
			break;
		case 0x4:
			std::cout << "~[CD_DRIVE] audio sector" << std::endl;
			waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
			break;
		case 0x8:

			std::cout << "~[CD_DRIVE] data sector" << std::endl;
			break;
		default:
			std::cout << "[CD_DRIVE] EMULATION PAUSED! not recognized CD sector!" <<
				std::hex << " 0x" << (((uint32_t)fileDataVect[currentPossition + 0x12] & 0x0f) >> 1) << std::endl;
			g_emulationPaused = true;
			break;
		}
	else
		std::cout << "~[CD_DRIVE] CDDA sector" << std::endl;

	for (int i = 0; i < cdTracks.size(); i++) {
		if ((amm == cdTracks[i].index1End.amm) &&
			(ass == cdTracks[i].index1End.ass) &&
			(asect == cdTracks[i].index1End.asect)) {

			if (mode.elem.autoPause) { //Pause
				bDataResponse = false;
				bPlay = false;

				if (stat.elem.read)
					stat.elem.read = 0;
				if (stat.elem.play)
					stat.elem.play = 0;

				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				setSecondResponse(secondWaitClocks, INT4, secondResponse, 1);

				std::cout << "~[CD_DRIVE] auto pause" << std::endl;
			}
			else if (i == cdTracks.size() - 1) { //Stop
				bDataResponse = false;//??
				bPlay = false; //??

				if (stat.elem.read)
					stat.elem.read = 0;
				if (stat.elem.play)
					stat.elem.play = 0;

				stat.elem.spindleMotor = 0;
				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				setSecondResponse(secondWaitClocks, INT4, secondResponse, 1);

				std::cout << "~[CD_DRIVE] auto stop end of the disc" << std::endl;
			}

		}
	}

	for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].index1Start.amm * 60 * 75 + cdTracks[i].index1Start.ass * 75 + cdTracks[i].index1Start.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].index1End.amm * 60 * 75 + cdTracks[i].index1End.ass * 75 + cdTracks[i].index1End.asect))) {

			track = i + 1;
			break;
		}
	}

	if (++asect == 75) {
		asect = 0;
		if (++ass == 60) {
			ass = 0;
			amm++;
		}
	}
}

void cdDrive::setDriveMode() {
	sectorSize = mode.elem.sectorSize ? 0x924 : 0x800;

	if (mode.elem.xa_adpcm == 0)
		indexStatus.reg.adpbusy = 0;

	std::cout << std::hex << "~[CD_DRIVE] mode: " << (uint16_t)mode.data << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] speed: " << (uint16_t)mode.elem.speed << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] XA-ADCPM: " << (uint16_t)mode.elem.xa_adpcm << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] sector size: " << (uint16_t)mode.elem.sectorSize << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] ignore bit: " << (uint16_t)mode.elem.ignoreBit << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] XA-Filter: " << (uint16_t)mode.elem.xa_filter << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] report: " << (uint16_t)mode.elem.report << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] autoPause: " << (uint16_t)mode.elem.autoPause << std::endl;
	std::cout << std::hex << "~[CD_DRIVE] CDDA: " << (uint16_t)mode.elem.cdda << std::endl;
}

//Cd Drive Commands

void cdDrive::sync(const uint8_t& command) {
	uint8_t response[2] = { stat.data | STAT_ERROR, 0x40 };
	setFirstResponse(0xc4e1, INT5, response, 2);
}

void cdDrive::xxxxx(const uint8_t& command) {
	std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled command!" << std::endl;
	g_emulationPaused = true;
}

void cdDrive::getStat(const uint8_t& command) {
	if (!bShellOpen)
		stat.elem.shellOpen = 0;

	uint8_t response[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::play(const uint8_t& command) {
	bPlay = true;

	//stat.elem.read = 0;//??

	stat.elem.spindleMotor = 1;
	if (bSetLoc) {
		bSetLoc = false;

		amm = unprocessed_amm;
		ass = unprocessed_ass;
		asect = unprocessed_asect;

		for (int i = 0; i < cdTracks.size(); i++) {
			if (((amm * 60 * 75 + ass * 75 + asect) >=
				(cdTracks[i].index1Start.amm * 60 * 75 + cdTracks[i].index1Start.ass * 75 + cdTracks[i].index1Start.asect)) &&
				((amm * 60 * 75 + ass * 75 + asect) <=
					(cdTracks[i].index1End.amm * 60 * 75 + cdTracks[i].index1End.ass * 75 + cdTracks[i].index1End.asect))) {

				track = i + 1;
				break;
			}
		}

		stat.elem.seek = 1;
	}
	else if (!paramFifo.empty()) { //parameter track
		track = paramFifo.back();
		paramFifo.pop_back();

		amm = cdTracks[track - 1].index1Start.amm;
		ass = cdTracks[track - 1].index1Start.ass;
		asect = cdTracks[track - 1].index1Start.asect;

		stat.elem.seek = 1;
	}
	else {
		for (int i = 0; i < cdTracks.size(); i++) {
			if (((amm * 60 * 75 + ass * 75 + asect) >=
				(cdTracks[i].index1Start.amm * 60 * 75 + cdTracks[i].index1Start.ass * 75 + cdTracks[i].index1Start.asect)) &&
				((amm * 60 * 75 + ass * 75 + asect) <=
					(cdTracks[i].index1End.amm * 60 * 75 + cdTracks[i].index1End.ass * 75 + cdTracks[i].index1End.asect))) {

				track = i + 1;
				break;
			}
		}
	}

	pBus->spu.ammCDDA = amm;
	pBus->spu.assCDDA = ass;
	pBus->spu.asectCDDA = asect;
	pBus->spu.counterCDDA = 0x930;

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat.elem.seek = 0; //~ used for gran turismo 2
	stat.elem.play = 1;

	bDataResponse = true;
	waitClocksDataResponse = mode.elem.speed ? 0x3ab2b : 0x72732;
}

void cdDrive::setLoc(const uint8_t& command) {
	bSetLoc = true;

	uint8_t temp;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x9) || (temp >= 0x99)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock amm value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	unprocessed_amm = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] amm: " << std::dec << (uint16_t)unprocessed_amm << std::endl;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x5) || (temp >= 0x60)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock ass value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	unprocessed_ass = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] ass: " << std::dec << (uint16_t)unprocessed_ass << std::endl;

	temp = paramFifo.back();
	paramFifo.pop_back();
	if (((temp & 0x0f) > 0x9) || ((temp >> 4) > 0x7) || (temp >= 0x75)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x" << std::hex << (uint16_t)temp << std::endl;
		g_emulationPaused = true;
		temp = 0;
	}
	unprocessed_asect = fromBCDToHex8(temp);
	std::cout << "~[CD_DRIVE] asect: " << std::dec << (uint16_t)unprocessed_asect << std::endl;

	uint8_t response[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::readN(const uint8_t& command) {
	stat.elem.spindleMotor = 1;

	//bPlay = false;//??
	//stat.elem.play = 0;//??

	//seek 
	if (bSetLoc) {
		bSetLoc = false;

		amm = unprocessed_amm;
		ass = unprocessed_ass;
		asect = unprocessed_asect;

		stat.elem.read = 0; //??
		stat.elem.seek = 1;
	}

	if (mode.elem.xa_adpcm) {
		pBus->spu.ammXA = amm;
		pBus->spu.assXA = ass;
		pBus->spu.asectXA = asect;
		pBus->spu.currentSample = 28 * 2 * 4 * 18 / (pBus->spu.b8BitsPerSample ? 2 : 1);
		pBus->spu.submode.data = 0;
	}

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	//stat.elem.seek = 0; //~ used for gran turismo 2

	bDataResponse = true;
	waitClocksDataResponse = mode.elem.speed ? 0x3ab2b : 0x72732;
}

void cdDrive::motorOn(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	if (stat.elem.spindleMotor) {
		uint8_t secondResponse[2] = { stat.data, 0x20 };
		setSecondResponse(0x1c421, INT5, secondResponse, 2);
	}
	else {
		stat.elem.spindleMotor = 1;
		uint8_t secondResponse[1] = { stat.data };
		setSecondResponse(0x1c421, INT2, secondResponse, 1);
	}
}

void cdDrive::stop(const uint8_t& command) {
	bDataResponse = false;//??
	bPlay = false; //??

	if (stat.elem.read)
		stat.elem.read = 0;
	if (stat.elem.play)
		stat.elem.play = 0;

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat.elem.spindleMotor = 0;
	uint8_t secondResponse[1] = { stat.data };
	uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 / 40 : 0x21181c / 40; //??
	setSecondResponse(secondWaitClocks, INT2, secondResponse, 1);
}

void cdDrive::pause(const uint8_t& command) {
	bDataResponse = false;
	bPlay = false;

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	if (stat.elem.read)
		stat.elem.read = 0;
	if (stat.elem.play)
		stat.elem.play = 0;

	uint8_t secondResponse[1] = { stat.data };
	uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 / 40 : 0x21181c / 40; //??
	setSecondResponse(secondWaitClocks, INT2, secondResponse, 1);
}

void cdDrive::init(const uint8_t& command) {
	bDataResponse = false;//??
	bPlay = false;//??

	mode.data = 0x20;
	setDriveMode();

	sectorSize = 0x924;
	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0x13cce, INT3, firstResponse, 1);

	stat.elem.spindleMotor = 1;
	if (stat.elem.read)
		stat.elem.read = 0;
	if (stat.elem.play)
		stat.elem.play = 0;

	uint8_t secondResponse[1] = { stat.data };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::mute(const uint8_t& command) {
	uint8_t response[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, response, 1);
}

void cdDrive::demute(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0x5cce, INT3, firstResponse, 1);
}

void cdDrive::setFilter(const uint8_t& command) {
	bFilter = true;

	uint8_t response[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, response, 1);
	filterFile = paramFifo.back();
	paramFifo.pop_back();
	std::cout << std::hex << "~[CD_DRIVE] file: 0x" << (uint16_t)filterFile << std::endl;
	filterChannel = paramFifo.back();
	paramFifo.pop_back();
	std::cout << std::hex << "~[CD_DRIVE] channel: 0x" << (uint16_t)filterChannel << std::endl;
}

void cdDrive::setMode(const uint8_t& command) {
	mode.data = paramFifo.back();
	paramFifo.pop_back();
	setDriveMode();

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);
}

void cdDrive::getParam(const uint8_t& command) {
	const uint8_t NULL_BYTE = 0x00;
	uint8_t firstResponse[5] = { stat.data, mode.data, NULL_BYTE, filterFile, filterChannel };
	setFirstResponse(0xc4e1, INT3, firstResponse, 5);
}

void cdDrive::getLocL(const uint8_t& command) {
	setFirstResponse(0xc4e1, INT3, (uint8_t*)storedSectorHeader, 8);
}

void cdDrive::getLocP(const uint8_t& command) {

	/*std::vector<uint8_t> firstResponse = { 0x1, 0x1, fromHex8ToDec(amm), fromHex8ToDec(ass), fromHex8ToDec(asect),
		fromHex8ToDec(amm), fromHex8ToDec(ass), fromHex8ToDec(asect) };*/
		/*std::vector<uint8_t> firstResponse = { 0x1, 0x1, 0,0,0,0,0,0 };*/

	uint8_t track = 0xff;
	uint8_t index;
	uint8_t mm;
	uint8_t ss;
	uint8_t sect;

	for (int i = 1; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].index0Start.amm * 60 * 75 + cdTracks[i].index0Start.ass * 75 + cdTracks[i].index0Start.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].index0End.amm * 60 * 75 + cdTracks[i].index0End.ass * 75 + cdTracks[i].index0End.asect))) {

			track = cdTracks[i].track;
			index = 0x00;
			mm = amm - cdTracks[i].index0Start.amm;
			ss = ass - cdTracks[i].index0Start.ass;
			sect = asect - cdTracks[i].index0Start.asect;
		}
	}

	for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].index1Start.amm * 60 * 75 + cdTracks[i].index1Start.ass * 75 + cdTracks[i].index1Start.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].index1End.amm * 60 * 75 + cdTracks[i].index1End.ass * 75 + cdTracks[i].index1End.asect))) {

			track = cdTracks[i].track;
			index = 0x01;
			mm = amm - cdTracks[i].index1Start.amm;
			ss = ass - cdTracks[i].index1Start.ass;
			if (track == 1)
				ss += 2;
			sect = asect - cdTracks[i].index1Start.asect;
		}
	}

	if (track == 0xff) {
		std::cout << std::hex << "~[CD_DRIVE] GetLocP incorrect track number: " << std::dec << (uint16_t)track << std::endl;
		g_emulationPaused = true;
	}
	std::vector<uint8_t> firstResponse;
	firstResponse.resize(8);
	if (bPlay)
		firstResponse = { fromHex8ToBCD(track), fromHex8ToBCD(index), fromHex8ToBCD(pBus->spu.ammCDDA), fromHex8ToBCD(pBus->spu.assCDDA), fromHex8ToBCD(pBus->spu.asectCDDA),
			fromHex8ToBCD(pBus->spu.ammCDDA), fromHex8ToBCD(pBus->spu.assCDDA), fromHex8ToBCD(pBus->spu.asectCDDA) };
	else
		firstResponse = { fromHex8ToBCD(track), fromHex8ToBCD(index), fromHex8ToBCD(pBus->spu.ammXA), fromHex8ToBCD(pBus->spu.assXA), fromHex8ToBCD(pBus->spu.asectXA),
			fromHex8ToBCD(pBus->spu.ammXA), fromHex8ToBCD(pBus->spu.assXA), fromHex8ToBCD(pBus->spu.asectXA) }; //??

	/*std::vector<uint8_t> firstResponse = { fromHex8ToBCD(track), fromHex8ToBCD(index), fromHex8ToBCD(mm), fromHex8ToBCD(ss), fromHex8ToBCD(sect),
		fromHex8ToBCD(amm), fromHex8ToBCD(ass), fromHex8ToBCD(asect) };*/

	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
}

void cdDrive::getTN(const uint8_t& command) {
	uint8_t firstTrack = 1;
	uint8_t lastTrack = (uint8_t)cdTracks.size();
	//uint8_t firstResponse[3] = { stat.data, 0x1, 0x1 };
	uint8_t firstResponse[3] = { stat.data, fromHex8ToBCD(firstTrack), fromHex8ToBCD(lastTrack) };
	setFirstResponse(0xc4e1, INT3, firstResponse, 3);
}

void cdDrive::getTD(const uint8_t& command) {
	std::vector<uint8_t> firstResponse;

	//uint8_t specifiedTrack = fromBCDToHex8(paramFifo.back());
	uint8_t specifiedTrack = fromBCDToHex8(paramFifo[0]);
	paramFifo.pop_back();

	if (specifiedTrack > cdTracks.size()) {
		uint8_t errorCode = 0x10;
		firstResponse.resize(1);
		firstResponse = { errorCode };
		setFirstResponse(0xc4e1, INT5, firstResponse.data(), firstResponse.size());

		std::cout << std::hex << "~[CD_DRIVE] GetTD incorrect track number: " << std::dec << (uint16_t)specifiedTrack << std::endl;
		g_emulationPaused = true;
		return;
	}

	uint8_t ass;
	uint8_t amm;

	if (specifiedTrack == 0x00) {
		ass = cdTracks[cdTracks.size() - 1].index1End.ass;
		amm = cdTracks[cdTracks.size() - 1].index1End.amm;
	}
	else {
		ass = cdTracks[specifiedTrack - 1].index1Start.ass;
		amm = cdTracks[specifiedTrack - 1].index1Start.amm;
		if (specifiedTrack > 1)
			ass += 0x2;
	}

	/*ass = 2;
	amm = 0;*/
	firstResponse.resize(3);
	firstResponse = { stat.data, fromHex8ToBCD(amm), fromHex8ToBCD(ass) };

	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());
}

void cdDrive::seekL(const uint8_t& command) {
	bDataResponse = false;

	if (stat.elem.read)
		stat.elem.read = 0;

	if (bSetLoc) {
		bSetLoc = false;

		amm = unprocessed_amm;
		ass = unprocessed_ass;
		asect = unprocessed_asect;

		stat.elem.seek = 1;
	}
	else {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! Issued SeekL before Setlock!\n";
		g_emulationPaused = true;
	}

	stat.elem.spindleMotor = 1;


	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat.elem.seek = 0;
	uint8_t secondResponse[1] = { stat.data };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::seekP(const uint8_t& command) {
	bDataResponse = false;

	if (stat.elem.read)
		stat.elem.read = 0;

	if (bSetLoc)
		bSetLoc = false;
	else {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! Issued SeekP before Setlock!\n";//??
		g_emulationPaused = true;
	}

	amm = unprocessed_amm;
	ass = unprocessed_ass;
	asect = unprocessed_asect;

	stat.elem.spindleMotor = 1;
	stat.elem.seek = 1;

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	stat.elem.seek = 0;
	stat.elem.play = 0;//??
	uint8_t secondResponse[1] = { stat.data };
	setSecondResponse(0x1c421, INT2, secondResponse, 1);
}

void cdDrive::test(const uint8_t& command) {
	switch (paramFifo.back()) {
	case 0x04: {
		stat.elem.spindleMotor = 1;
		std::vector<uint8_t> firstResponse = { stat.data };
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
		//std::vector<uint8_t> firstResponse = { 0x94, 0x09, 0x19, 0xc0 };
		std::vector<uint8_t> firstResponse = { 0xa1, 0x03, 0x06, 0xc3 };
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
	std::vector<uint8_t> firstResponse = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse.data(), firstResponse.size());

	std::vector<uint8_t> secondResponse;
	responseINT_t secondINT;
	switch (driveStatus) {
	case NO_DISC:
		stat.elem.idError = 1;
		secondResponse = { stat.data, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INT5;
		break;
	case UNLICENCED_DISC_MODE_1:
		stat.elem.spindleMotor = 1;
		stat.elem.idError = 1;
		secondResponse = { stat.data, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INT5;
		break;
	case LICENCED_MODE_2:
		stat.elem.spindleMotor = 1; // can differ after stop command??
		secondResponse = { stat.data, 0x00, 0x20, 0x00, 0x53, 0x43, 0x45, 0x41 };
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
	stat.elem.spindleMotor = 1;

	bPlay = false;//??
	stat.elem.play = 0;//??
	stat.elem.read = 0;//??

	//seek 
	if (bSetLoc) {
		bSetLoc = false;

		amm = unprocessed_amm;
		ass = unprocessed_ass;
		asect = unprocessed_asect;

		stat.elem.seek = 1;
	}

	if (mode.elem.xa_adpcm) {
		pBus->spu.ammXA = amm;
		pBus->spu.assXA = ass;
		pBus->spu.asectXA = asect;
		pBus->spu.currentSample = 28 * 2 * 4 * 18 / (pBus->spu.b8BitsPerSample ? 2 : 1);
		pBus->spu.submode.data = 0;
	}

	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	//stat.elem.seek = 0; //~ used for gran turismo 2

	bDataResponse = true;
	waitClocksDataResponse = mode.elem.speed ? 0x3ab2b : 0x72732;
}

void cdDrive::readTOC(const uint8_t& command) {
	uint8_t firstResponse[1] = { stat.data };
	setFirstResponse(0xc4e1, INT3, firstResponse, 1);

	uint8_t secondResponse[10] = { stat.data };
	setSecondResponse(44100 * 0x300, INT2, secondResponse, 1);
}

void cdDrive::readSectorXA_ADPCM(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src) {
	bool bSectorFound = false;

	while (!bSectorFound) {
		if (fileDataVect.size() < (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x930)) {
			asect = 0;
			ass = 2;
			amm = 0;
		}

		uint32_t currentPossition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;

		src = (uint8_t*)fileDataVect.data() + currentPossition;

		if (fileDataVect[currentPossition + 0x10 + 0x02] & 0x04)
			if (mode.elem.xa_filter)
				bSectorFound = (fileDataVect[currentPossition + 0x10] == filterFile) &&
				(fileDataVect[currentPossition + 0x10 + 0x01] == filterChannel);
			else
				bSectorFound = true;

		/*if (bSectorFound) {
			std::cout << "~[CD_DRIVE] Read sectorXA:" << std::endl;
			std::cout << std::dec << "~[CD_DRIVE] ammXA: " << (uint16_t)amm << std::endl;
			std::cout << std::dec << "~[CD_DRIVE] assXA: " << (uint16_t)ass << std::endl;
			std::cout << std::dec << "~[CD_DRIVE] asectXA: " << (uint16_t)asect << std::endl;
		}*/

		if (++asect == 75) {
			asect = 0;
			if (++ass == 60) {
				ass = 0;
				amm++;
			}
		}
	}
	pBus->spu.submode.data = src[0x10 + 0x02];
	pBus->spu.codinginfo.data = src[0x10 + 0x03];
	pBus->spu.bStereo = pBus->spu.codinginfo.elem.monoStereo;
	pBus->spu.bSampleRate18900Hz = pBus->spu.codinginfo.elem.sampleRate;
	pBus->spu.b8BitsPerSample = pBus->spu.codinginfo.elem.bitsPerSample;
}

void cdDrive::readSectorCDDA(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src) {
	uint32_t currentPossition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;
	src = (uint8_t*)fileDataVect.data() + currentPossition;

	/*for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].index1Start.amm * 60 * 75 + cdTracks[i].index1Start.ass * 75 + cdTracks[i].index1Start.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].index1End.amm * 60 * 75 + cdTracks[i].index1End.ass * 75 + cdTracks[i].index1End.asect))) {

			trackCDDA = i + 1;
			break;
		}
	}

	if (trackCDDA == 1)
		src = pNullSrc;*/

	if (++asect == 75) {
		asect = 0;
		if (++ass == 60) {
			ass = 0;
			amm++;
		}
	}
}

//debug


void cdDrive::printStat() {
	std::cout << "~[CD_DRIVE] stat: 0x" << std::hex << (uint16_t)stat.data << std::endl;
	if (stat.elem.play)
		std::cout << "   Playing CD-DA\n";
	if (stat.elem.seek)
		std::cout << "   Seeking\n";
	if (stat.elem.read)
		std::cout << "   Reading data sectors\n";
	if (stat.elem.shellOpen)
		std::cout << "   Shell is/was open\n";
	if (stat.elem.idError)
		std::cout << "   GetID denied\n";
	if (stat.elem.seekError)
		std::cout << "   Seek error\n";
	if (stat.elem.spindleMotor)
		std::cout << "   Motor on\n";
	else
		std::cout << "   Motor off, or in spin-up phase\n";
	if (stat.elem.error)
		std::cout << "   Invalid command or command parameters!!!\n";

	if ((stat.elem.read && stat.elem.seek) ||
		(stat.elem.read && stat.elem.play) ||
		(stat.elem.seek && stat.elem.play)) {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! Play/Seek/Read status bits set at the same time!\n";
		//g_emulationPaused = true;
	}
}