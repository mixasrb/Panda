#include "hc05_pux.h"
#include "../bus/bus_interface.h"

extern bool g_isEmulationPaused;
extern const char* g_cdPath;
extern uint8_t g_driveStatus;

static uint8_t ConvFromBcdToHex8(const uint8_t& dec) {
	return (dec >> 4) * 10 + (dec & 0x0f);
}

static uint8_t ConvFromHex8ToBcd(const uint8_t& hex) {
	return (hex / 10) * 16 + (hex % 10);
}

CdDrive::CdDrive() {
	commandLookup.resize(20);
	commandLookup = {
		{"Sync",      &CdDrive::sync},{"Getstat",    &CdDrive::getStat},{"Setloc",   &CdDrive::setLoc},{"Play",        &CdDrive::play},
		{"Forward",  &CdDrive::xxxxx},{"Backward",     &CdDrive::xxxxx},{"ReadN",     &CdDrive::readN},{"MotorOn",  &CdDrive::motorOn},
		{"Stop",      &CdDrive::stop},{"Pause",        &CdDrive::pause},{"Init",       &CdDrive::init},{"Mute",        &CdDrive::mute},
		{"Demute",  &CdDrive::demute},{"Setfilter",&CdDrive::setFilter},{"Setmode", &CdDrive::setMode},{"Getparam",&CdDrive::getParam},
		{"GetlocL",&CdDrive::getLocL},{"GetlocP",    &CdDrive::getLocP},{"SetSession",&CdDrive::xxxxx},{"GetTN",      &CdDrive::getTN},
		{"GetTD",    &CdDrive::getTD},{"SeekL",        &CdDrive::seekL},{"SeekP",     &CdDrive::seekP},{"SetClock",   &CdDrive::xxxxx},
		{"GetClock", &CdDrive::xxxxx},{"Test",          &CdDrive::test},{"GetID",     &CdDrive::getID},{"ReadS",      &CdDrive::readS},
		{"Reset",    &CdDrive::xxxxx},{"GetQ",         &CdDrive::xxxxx},{"ReadTOC", &CdDrive::readTOC},{"VideoCD",    &CdDrive::xxxxx},
	};

	driveStatus = static_cast<DriveStatus>(g_driveStatus);


	std::string path = g_cdPath;

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cout << "Failed to open rom!\n";
		g_isEmulationPaused = true;
	}

	size_t fileSize = file.tellg();
	file.seekg(0);
	fileDataVect.resize(fileSize);
	file.read(fileDataVect.data(), fileSize);
	file.close();


	std::cout << path << '\n';

	int i = 1;
	Track currentTrack;
	currentTrack.track = i;

	currentTrack.trackStart.asect = 0;
	currentTrack.trackStart.ass = 2;
	currentTrack.trackStart.amm = 0;


	const uint32_t sectorSize = 0x930;
	uint32_t sectorCount = fileSize / sectorSize;
	uint32_t temp_sect = sectorCount % 75;
	uint32_t secondsCount = sectorCount / 75;
	uint32_t temp_ss = secondsCount % 60;
	uint32_t temp_mm = secondsCount / 60;

	currentTrack.trackEnd = currentTrack.trackStart;
	AddSector(currentTrack.trackEnd, temp_mm, temp_ss, temp_sect - 1);

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


				std::cout << ss.str() << '\n';
				Track perviousTrack = cdTracks[i - 2];

				Track currentTrack;
				currentTrack.track = i;

				currentTrack.trackStart = perviousTrack.trackEnd;
				AddSector(currentTrack.trackStart, 0, 0, 1);

				uint32_t sectorCount = fileSizeTemp / sectorSize;
				uint32_t temp_sect = sectorCount % 75;
				uint32_t secondsCount = sectorCount / 75;
				uint32_t temp_ss = secondsCount % 60;
				uint32_t temp_mm = secondsCount / 60;

				currentTrack.trackEnd = currentTrack.trackStart;
				AddSector(currentTrack.trackEnd, temp_mm, temp_ss, temp_sect - 1);


				cdTracks.push_back(currentTrack);
			}
			else
				break;

		} while (1);
	}
}

void CdDrive::Read8(const uint32_t& addr, uint8_t& data) {
	data = 0;

	switch (addr) {
	case 0x1f801800:
		//Set stat flags
		indexStatus.elem.prmempt = (paramFifo.empty()) ? 1 : 0;

		indexStatus.elem.prmwrdy = (paramFifo.size() == 16) ? 0 : 1;

		indexStatus.elem.rslrrdy = (resultFifo.empty()) ? 0 : 1;

		if (bufferCounter == 0)
			indexStatus.elem.drqsts = 0;

		data = indexStatus.data;
		break;

	case 0x1f801801:
		if (resultFifo.empty()) {
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! Response FIFO is empty\n";
			g_isEmulationPaused = true;
			break;
		}

		data = resultFifo.front();
		resultFifo.pop();

		std::cout << "~[CD_DRIVE] Response read: 0x" << std::hex << (uint16_t)data << '\n';
		break;

	case 0x1f801802:
		data = dataBufferPtr[sectorSize - bufferCounter--];
		break;

	case 0x1f801803:
		switch (indexStatus.elem.bank) {
		case 0:
		case 2:
			data = interruptEnable.data;
			break;
		case 1:
		case 3:
			data = 0xe0 | irqFlagRead.elem.int_1_7 | (irqFlagRead.elem.int_8 << 3) | (irqFlagRead.elem.int_10 << 4);
			break;
		}
		break;
	}
}

void CdDrive::Write8(const uint32_t& addr, const uint8_t& data) {
	switch (addr) {
	case 0x1f801800:
		indexStatus.elem.bank = data & 0x03;
		break;

	case 0x1f801801:
		switch (indexStatus.elem.bank) {
		case 0:
			std::cout << "~[CD_DRIVE] Command received : " << commandLookup[data].name << '\n';

			if (!commandFifo.empty() || indexStatus.elem.busysts) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! Command received before previous one is executed\n";
				g_isEmulationPaused = true;
			}

			commandFifo.push(data);
			break;
		case 1:
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! 1F801801h.Index1 - Sound Map Data Out (W) 0x"
				<< std::hex << (uint16_t)data << '\n';
			g_isEmulationPaused = true;
			break;
		case 2:
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! 1F801801h.Index2 - Sound Map Coding Info (W) 0x"
				<< std::hex << (uint16_t)data << '\n';
			g_isEmulationPaused = true;
			break;
		case 3:
			tempVolumeRightRight = (uint16_t)data;
			break;
		}
		break;

	case 0x1f801802:
		switch (indexStatus.elem.bank) {
		case 0:
			paramFifo.push(data);
			std::cout << "~[CD_DRIVE] Parameter received: 0x" << std::hex << (uint16_t)data << '\n';
			break;
		case 1:
			interruptEnable.data = data;
			break;
		case 2:
			tempVolumeLeftLeft = (uint16_t)data;
			break;
		case 3:
			tempVolumeRightLeft = (uint16_t)data;
			break;
		}
		break;

	case 0x1f801803:
		switch (indexStatus.elem.bank) {
		case 0:

			requestReg.data = data;

			if (requestReg.elem.smen) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! nhandled SMEN 0x1f801803 index 0\n";
				g_isEmulationPaused = true;
			}

			if (requestReg.elem.bfwr) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! Unhandled BFWR 0x1f801803 index 0\n";
				g_isEmulationPaused = true;
			}

			if (requestReg.elem.bfrd) {
				indexStatus.elem.drqsts = 1; //!?
				std::cout << "~[CD_DRIVE] Load data fifo\n";
			}

			break;
		case 1:

			if ((data & 0x7) == 0x7) {
				if (irqFlagRead.elem.int_1_7)
					std::cout << "~[CD_DRIVE] Acknowledged INT" << (uint16_t)irqFlagRead.elem.int_1_7 << '\n';
				irqFlagRead.elem.int_1_7 = NO_INTERRUPT;
			}
			if ((0x0 < (data & 0x7)) && ((data & 0x7) < 0x7)) { // 0 < (data & 0x7) < 7
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! Unhandled Interrupt Flag Register (W) state 0x"
					<< std::hex << (uint16_t)data << '\n';
				g_isEmulationPaused = true;
			}
			if (data & 0x8) {
				if (irqFlagRead.elem.int_8)
					std::cout << "~[CD_DRIVE] Acknowledged INT8" << '\n';
				irqFlagRead.elem.int_8 = 0;
			}
			if (data & 0x10) {
				if (irqFlagRead.elem.int_10)
					std::cout << "~[CD_DRIVE] Acknowledged INT10" << '\n';
				irqFlagRead.elem.int_10 = 0;
			}
			if (data & 0x20) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! Unhandled Clear sound map XA - ADPCM buffer" << '\n';
			}
			if (data & 0x40) {
				std::cout << "~[CD_DRIVE] Parameter fifo reseted, count: " << paramFifo.size() << '\n';
				while (!paramFifo.empty())
					paramFifo.pop();
			}
			if (data & 0x80) {
				std::cout << "~[CD_DRIVE] EMULATION PAUSED! Unhandled Reset decoder chip" << '\n';
				g_isEmulationPaused = true;
			}

			break;
		case 2:
			tempVolumeLeftRight = (uint16_t)data;
			break;
		case 3:

			if (data & 0x01) {
				if (mode.elem.xa_adpcm) { //??
					volumeLeftLeft = 0;
					volumeLeftRight = 0;
					volumeRightRight = 0;
					volumeRightLeft = 0;
				}
			}
			if (data & 0x20) {
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

void CdDrive::SetResponseInterrupt(const ResponseInterrupt& responseINT) {
	pendingInterrupts.push(responseINT);
}

void CdDrive::SetFirstResult(const uint32_t& waitClocks, const ResponseInterrupt& responseINT,
	uint8_t* resultsPtr, const uint8_t& resultsCount) {
	isFirstResponsePending = true;
	waitClocksFirstResponse = waitClocks;

	//debug
	std::cout << "~[CD_DRIVE] First response" << '\n';
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPtr[i]);
		std::cout << "~[CD_DRIVE] First Response sent: 0x" << std::hex << (uint16_t)resultsPtr[i] << '\n';
	}

	firstResponseInterrupt = responseINT;
}

void CdDrive::SetSecondResult(const uint32_t& waitClocks, const ResponseInterrupt& responseINT,
	uint8_t* resultsPtr, const uint8_t& resultsCount) {
	isSecondResponsePending = true;
	waitClocksSecondResponse = waitClocks;

	//debug
	std::cout << "~[CD_DRIVE] Second response" << '\n';
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPtr[i]);
		std::cout << "~[CD_DRIVE] Second Response sent: 0x" << std::hex << (uint16_t)resultsPtr[i] << '\n';
	}

	secondResponseInterrupt = responseINT;
}

void CdDrive::SetDataResult(const ResponseInterrupt& responseINT,
	uint8_t* resultsPrt, const uint8_t& resultsCount) {

	//debug
	std::cout << "~[CD_DRIVE] data response" << '\n';
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPrt[i]);
		std::cout << "~[CD_DRIVE] Data Response sent: 0x" << std::hex << (uint16_t)resultsPrt[i] << '\n';
	}

	dataResponseInterrupt = responseINT;
}

void CdDrive::Clock() {

	if (isFirstResponsePending || isSecondResponsePending)
		clocks++;
	if (isDataResponseActive)
		dataClocks++;

	if (irqFlagRead.data != 0)
		return;

	if (!pendingInterrupts.empty()) {
		irqFlagRead.elem.int_1_7 = pendingInterrupts.front();
		pendingInterrupts.pop();

		if (irqFlagRead.elem.int_1_7)
			std::cout << "~[CD_DRIVE] Sent Interrupt " << uint16_t(irqFlagRead.elem.int_1_7) << " flag" << '\n';
		if (irqFlagRead.elem.int_8) {
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! Interrupt 8 Unhandled" << '\n';
			g_isEmulationPaused = true;
		}
		if (irqFlagRead.elem.int_10) {
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! Interrupt 10 Unhandled" << '\n';
			g_isEmulationPaused = true;
		}

		if (interruptEnable.data & irqFlagRead.data)
			busPtr->pCpu->cp0.interruptHandler(_IRQ_CD_ROM);
	}

	if (isFirstResponsePending
		&& clocks >= waitClocksFirstResponse) {
		isFirstResponsePending = false;
		clocks = 0;

		SetResponseInterrupt(firstResponseInterrupt);

		indexStatus.elem.busysts = 0;
	}
	else if (!isFirstResponsePending && isSecondResponsePending
		&& clocks >= waitClocksSecondResponse) {
		isSecondResponsePending = false;
		clocks = 0;

		SetResponseInterrupt(secondResponseInterrupt);
	}
	else if (!isFirstResponsePending && !isSecondResponsePending && isDataResponseActive
		&& dataClocks >= (waitClocksDataResponse /*+ 0x36cd2 * 1 / 10*/)) {
		dataClocks -= waitClocksDataResponse;

		ReadSector(amm, ass, asect);

		if (sendINT1) {
			if (!bPlay) {
				uint8_t dataResponse[1] = { stat.data };
				SetDataResult(INTERRUPT_1, dataResponse, 1);
				SetResponseInterrupt(dataResponseInterrupt);
			}
			else { // Play Report
				if ((storedAsect % 10) == 0) {
					uint32_t diff = ((storedAmm * 60 * 75 + storedAss * 75 + storedAsect) -
						(cdTracks[track - 1].trackStart.amm * 60 * 75 +
							cdTracks[track - 1].trackStart.ass * 75 +
							cdTracks[track - 1].trackStart.asect));

					uint8_t mm = diff / (60 * 75);
					diff -= mm * (60 * 75);

					uint8_t ss = diff / 75;
					diff -= ss * 75;

					uint8_t sect = diff;

					uint8_t index = 0;
					if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75))
						index = 1;

					std::vector<uint8_t>dataResponse;
					dataResponse.resize(8);
					if ((storedAsect / 10) & 0x1)
						dataResponse = { stat.data,  ConvFromHex8ToBcd(track), index, 
						ConvFromHex8ToBcd(mm), ConvFromHex8ToBcd(ss + 80), ConvFromHex8ToBcd(sect), 0x7f, 0x00 };//??
					else
						dataResponse = { stat.data, ConvFromHex8ToBcd(track), index, 
						ConvFromHex8ToBcd(storedAmm), ConvFromHex8ToBcd(storedAss),ConvFromHex8ToBcd(storedAsect), 0x7f, 0x00 };//??

					SetDataResult(INTERRUPT_1, dataResponse.data(), 8);
					SetResponseInterrupt(dataResponseInterrupt);
				}
			}
		}
	}

	if (!isFirstResponsePending && !isSecondResponsePending &&
		!commandFifo.empty()) {
		clocks = 0;

		ExecuteNextCommand();
	}
}

void CdDrive::GetParameters(uint8_t* paramPtr, const uint8_t& paramCount) {
	for (uint8_t i = 0; i < paramCount; i++) {
		if (paramFifo.empty()) {
			std::cout << "~[CD_DRIVE] EMULATION PAUSED! Parameters requested before they are available\n";
			g_isEmulationPaused = true;
			break;
		}

		paramPtr[i] = paramFifo.front();
		paramFifo.pop();
		std::cout << "~[CD_DRIVE] Parameters read: 0x" << std::hex << (uint16_t)paramPtr[i] << '\n';
	}
}

void CdDrive::ExecuteNextCommand() {
	uint8_t command = commandFifo.front();
	commandFifo.pop();

	indexStatus.elem.busysts = 1;

	//debug
	std::cout << "~[CD_DRIVE] Command Executed: " << commandLookup[command].name << '\n';

	//execute command
	(this->*commandLookup[command].command)();
}

void CdDrive::ReadSector(uint8_t& amm, uint8_t& ass, uint8_t& asect) {
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
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector ass < 2!" << '\n';
		g_isEmulationPaused = true;
		return;
	}

	if (fileDataVect.size() < (currentPossition + 0x930)) {
		std::cout << "[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector!" << '\n';
		g_isEmulationPaused = true;
		return;
	}

	sendINT1 = false;
	if ((bFilter && mode.elem.xa_adpcm && mode.elem.xa_filter && (fileDataVect[currentPossition + 0x12] & 0x04)) ||
		(stat.elem.play && (mode.elem.report == 0)))
		sendINT1 = false;
	else if ((!bPlay) ||
		(bPlay && mode.elem.report)) {
		sendINT1 = true;
		dataBufferPtr = (uint8_t*)fileDataVect.data() + (currentPossition + (mode.elem.sectorSize ? 0x0c : 0x18));
	}

	if (!bPlay)
		for (int i = 0; i < 8; i++)
			storedSectorHeader[i] = fileDataVect[currentPossition + 0x0c + i];

	auto fillBufferXA_ADPCM = [&]() {
		busPtr->spu.pBufferXA_ADPCM = (uint8_t*)fileDataVect.data() + currentPossition;
		busPtr->spu.decodeSectorXA_ADPCM(busPtr->spu.pBufferXA_ADPCM);
		busPtr->spu.currentSample = 0;
		};

	if (mode.elem.xa_adpcm &&
		(fileDataVect[currentPossition + 0x0f] == 0x02) &&
		(fileDataVect[currentPossition + 0x10 + 0x02] & 0x04) &&
		(fileDataVect[currentPossition + 0x10 + 0x02] & 0x40))
		if (mode.elem.xa_filter) {
			if ((fileDataVect[currentPossition + 0x10] == filteredFile) &&
				(fileDataVect[currentPossition + 0x10 + 0x01] == filteredChannel))
				fillBufferXA_ADPCM();
		}
		else
			fillBufferXA_ADPCM();

	storedAmm = amm;
	storedAss = ass;
	storedAsect = asect;

	bufferCounter = sectorSize;

	if ((mode.elem.xa_adpcm) && (fileDataVect[currentPossition + 0x12] & 0x4))
		indexStatus.elem.adpbusy = 1;

	waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
	//waitClocksDataResponse += 0x36cd2 * 10 / 10;
	std::cout << "~[CD_DRIVE] Read sector: " << std::dec << (uint16_t)amm << std::dec << ":" << (uint16_t)ass << std::dec << ":" << (uint16_t)asect << '\n';

	if (!bPlay)
		if ((uint8_t)fileDataVect[currentPossition + 0x11] == 0xff)
			std::cout << "~[CD_DRIVE] unused sector" << '\n';
		else
			switch (((uint8_t)fileDataVect[currentPossition + 0x12] & 0x0f) >> 1 << 1) {
			case 0x0:
				std::cout << "~[CD_DRIVE] unused sector" << '\n';
				waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
				break;
			case 0x2:
				std::cout << "~[CD_DRIVE] video sector" << '\n';
				waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
				break;
			case 0x4:
				std::cout << "~[CD_DRIVE] audio sector" << '\n';
				waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
				break;
			case 0x8:
				std::cout << "~[CD_DRIVE] data sector" << '\n';
				break;
			default:
				std::cout << "[CD_DRIVE] EMULATION PAUSED! not recognized CD sector!" <<
					std::hex << " 0x" << (((uint32_t)fileDataVect[currentPossition + 0x12] & 0x0f) >> 1 << 1) << '\n';
				g_isEmulationPaused = true;
				break;
			}
	else
		std::cout << "~[CD_DRIVE] CDDA sector" << '\n';

	for (int i = 0; i < cdTracks.size(); i++) {
		if ((amm == cdTracks[i].trackEnd.amm) &&
			(ass == cdTracks[i].trackEnd.ass) &&
			(asect == cdTracks[i].trackEnd.asect)) {

			if (mode.elem.autoPause) { //Pause
				CancelReading();

				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				SetSecondResult(secondWaitClocks, INTERRUPT_4, secondResponse, 1);

				std::cout << "~[CD_DRIVE] auto pause" << '\n';
			}
			else if (i == cdTracks.size() - 1) { //Stop
				CancelReading();

				stat.elem.spindleMotor = 0;
				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				SetSecondResult(secondWaitClocks, INTERRUPT_4, secondResponse, 1);

				std::cout << "~[CD_DRIVE] auto stop end of the disc" << '\n';
			}

		}
	}

	for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].trackStart.amm * 60 * 75 + cdTracks[i].trackStart.ass * 75 + cdTracks[i].trackStart.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].trackEnd.amm * 60 * 75 + cdTracks[i].trackEnd.ass * 75 + cdTracks[i].trackEnd.asect))) {

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

void CdDrive::CancelReading() {
	isDataResponseActive = false;
	bPlay = false;

	stat.elem.read = 0;
	stat.elem.play = 0;
}

void CdDrive::PrintDriveMode() {
	std::cout << std::hex << "~[CD_DRIVE] mode: " << (uint16_t)mode.data << '\n';
	std::cout << std::hex << "~[CD_DRIVE] speed: " << (uint16_t)mode.elem.speed << '\n';
	std::cout << std::hex << "~[CD_DRIVE] XA-ADCPM: " << (uint16_t)mode.elem.xa_adpcm << '\n';
	std::cout << std::hex << "~[CD_DRIVE] sector size: " << (uint16_t)mode.elem.sectorSize << '\n';
	std::cout << std::hex << "~[CD_DRIVE] ignore bit: " << (uint16_t)mode.elem.ignoreBit << '\n';
	std::cout << std::hex << "~[CD_DRIVE] XA-Filter: " << (uint16_t)mode.elem.xa_filter << '\n';
	std::cout << std::hex << "~[CD_DRIVE] report: " << (uint16_t)mode.elem.report << '\n';
	std::cout << std::hex << "~[CD_DRIVE] autoPause: " << (uint16_t)mode.elem.autoPause << '\n';
	std::cout << std::hex << "~[CD_DRIVE] CDDA: " << (uint16_t)mode.elem.cdda << '\n';
}

//Cd Drive Commands

void CdDrive::sync() {
	uint8_t response[2] = { stat.data | STAT_ERROR, 0x40 };
	SetFirstResult(0xc4e1, INTERRUPT_5, response, 2);
}

void CdDrive::xxxxx() {
	std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled command!" << '\n';
	g_isEmulationPaused = true;
}

void CdDrive::getStat() {
	if (!isShellOpen)
		stat.elem.shellOpen = 0;

	uint8_t response[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, response, 1);
}

void CdDrive::play() {
	stat.elem.spindleMotor = 1;

	if (!bSetLocIssued && paramFifo.empty()) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		std::cout << "[CD_DRIVE] WARNING! EMULATION PAUSED! Issued Play without setLoc!\n";
		g_isEmulationPaused = true;
	}
	else if (bSetLocUnprocessed) {
		bSetLocUnprocessed = false;

		amm = unprocessedAmm;
		ass = unprocessedAss;
		asect = unprocessedAsect;

		stat.elem.seek = 1;
	}
	else if (!paramFifo.empty()) { //parameter track
		uint8_t trackBcd;
		GetParameters(&trackBcd, 1);
		track = ConvFromBcdToHex8(trackBcd);//??

		amm = cdTracks[track - 1].trackStart.amm;
		ass = cdTracks[track - 1].trackStart.ass;
		asect = cdTracks[track - 1].trackStart.asect;

		stat.elem.seek = 1;
	}

	bSetLocIssued = false;

	CancelReading();

	busPtr->spu.ammCDDA = amm;
	busPtr->spu.assCDDA = ass;
	busPtr->spu.asectCDDA = asect;
	busPtr->spu.counterCDDA = 0x930;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.seek = 0; //~ used for gran turismo 2
	stat.elem.play = 1;

	bPlay = true;
	isDataResponseActive = true;
	waitClocksDataResponse = mode.elem.speed ? 0x3ab2b : 0x72732;
}

void CdDrive::setLoc() {
	bSetLocIssued = true;
	bSetLocUnprocessed = true;

	uint8_t ammTemp;
	GetParameters(&ammTemp, 1);
	if (((ammTemp & 0x0f) > 0x9) || ((ammTemp >> 4) > 0x9) || (ammTemp >= 0x99)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock amm value 0x" << std::hex << (uint16_t)ammTemp << '\n';
		g_isEmulationPaused = true;
		ammTemp = 0;
	}
	unprocessedAmm = ConvFromBcdToHex8(ammTemp);
	std::cout << "~[CD_DRIVE] amm: " << std::dec << (uint16_t)unprocessedAmm << '\n';

	uint8_t assTemp;
	GetParameters(&assTemp, 1);
	if (((assTemp & 0x0f) > 0x9) || ((assTemp >> 4) > 0x5) || (assTemp >= 0x60)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock ass value 0x" << std::hex << (uint16_t)assTemp << '\n';
		g_isEmulationPaused = true;
		assTemp = 0;
	}
	unprocessedAss = ConvFromBcdToHex8(assTemp);
	std::cout << "~[CD_DRIVE] ass: " << std::dec << (uint16_t)unprocessedAss << '\n';

	uint8_t asectTemp;
	GetParameters(&asectTemp, 1);
	if (((asectTemp & 0x0f) > 0x9) || ((asectTemp >> 4) > 0x7) || (asectTemp >= 0x75)) {
		std::cout << "~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x" << std::hex << (uint16_t)asectTemp << '\n';
		g_isEmulationPaused = true;
		asectTemp = 0;
	}
	unprocessedAsect = ConvFromBcdToHex8(asectTemp);
	std::cout << "~[CD_DRIVE] asect: " << std::dec << (uint16_t)unprocessedAsect << '\n';

	uint8_t response[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, response, 1);
}

void CdDrive::readN() {
	stat.elem.spindleMotor = 1;

	waitClocksDataResponse = 0;

	//seeking 
	if (!bSetLocIssued) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		std::cout << "[CD_DRIVE] WARNING! EMULATION PAUSED! Issued ReadN without setLoc!\n";
		g_isEmulationPaused = true;
	}
	else {
		if (bSetLocUnprocessed) {
			bSetLocUnprocessed = false;

			amm = unprocessedAmm;
			ass = unprocessedAss;
			asect = unprocessedAsect;

			stat.elem.seek = 1;

			waitClocksDataResponse = (mode.elem.speed ? 0x3ab2b : 0x72732) * 5;
		}
	}

	bSetLocIssued = false;

	CancelReading();

	//if (mode.elem.xa_adpcm) {
	//	pBus->spu.ammXA = amm;
	//	pBus->spu.assXA = ass;
	//	pBus->spu.asectXA = asect;
	//	pBus->spu.currentSample = 28 * 2 * 4 * 18 / (pBus->spu.b8BitsPerSample ? 2 : 1);
	//	pBus->spu.submode.data = 0;
	//}

	uint8_t firstResponse[1] = { stat.data };
	uint32_t firstResponseClocks = 0xc4e1;

	SetFirstResult(firstResponseClocks, INTERRUPT_3, firstResponse, 1);

	//if (stat.elem.seek) { //?? break gran turismo 2, Ace Combat 2 (USA)
	//	/*std::cout << "[CD_DRIVE] EMULATION PAUSED! ReadN\n";
	//	g_emulationPaused = true;*/
	//	uint8_t secondResponse[1] = { stat.data };
	//	setSecondResponse(0x1c421, INT2, secondResponse, 1);
	//}

	isDataResponseActive = true;

	waitClocksDataResponse += mode.elem.speed ? 0x3ab2b : 0x72732;
}

void CdDrive::motorOn() {
	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	if (stat.elem.spindleMotor) {
		uint8_t secondResponse[2] = { stat.data, 0x20 };
		SetSecondResult(0x1c421, INTERRUPT_5, secondResponse, 2);
	}
	else {
		stat.elem.spindleMotor = 1;
		uint8_t secondResponse[1] = { stat.data };
		SetSecondResult(0x1c421, INTERRUPT_2, secondResponse, 1);
	}
}

void CdDrive::stop() {
	uint32_t secondWaitClocks;

	if (isDataResponseActive)
		secondWaitClocks = mode.elem.speed ? 0x18a6076 : 0x0d38aca;
	else
		secondWaitClocks = 0x0001d7b;

	CancelReading();

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.spindleMotor = 0;

	uint8_t secondResponse[1] = { stat.data };

	SetSecondResult(secondWaitClocks, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::pause() {
	uint32_t secondWaitClocks;

	if (isDataResponseActive)
		secondWaitClocks = mode.elem.speed ? 0x010bd93 : 0x021181c;
	else
		secondWaitClocks = 0x0001df2;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	CancelReading();

	uint8_t secondResponse[1] = { stat.data };

	SetSecondResult(secondWaitClocks, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::init() {
	stat.elem.spindleMotor = 1;

	mode.data = 0x20;
	sectorSize = mode.elem.sectorSize ? 0x924 : 0x800;
	if (mode.elem.xa_adpcm == 0)
		indexStatus.elem.adpbusy = 0;

	bMuted = false;

	PrintDriveMode();

	CancelReading();

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0x13cce, INTERRUPT_3, firstResponse, 1);

	uint8_t secondResponse[1] = { stat.data };
	SetSecondResult(0x1c421, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::mute() {
	bMuted = true;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::demute() {
	bMuted = false;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::setFilter() {
	bFilter = true;

	GetParameters(&filteredFile, 1);
	std::cout << std::hex << "~[CD_DRIVE] file: 0x" << (uint16_t)filteredFile << '\n';

	GetParameters(&filteredChannel, 1);
	std::cout << std::hex << "~[CD_DRIVE] channel: 0x" << (uint16_t)filteredChannel << '\n';

	uint8_t response[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, response, 1);
}

void CdDrive::setMode() {
	GetParameters(&mode.data, 1);

	sectorSize = mode.elem.sectorSize ? 0x924 : 0x800;
	if (mode.elem.xa_adpcm == 0)
		indexStatus.elem.adpbusy = 0;

	PrintDriveMode();

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::getParam() {
	const uint8_t NULL_BYTE = 0x00;
	uint8_t firstResponse[5] = { stat.data, mode.data, NULL_BYTE, filteredFile, filteredChannel };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 5);
}

void CdDrive::getLocL() {
	SetFirstResult(0xc4e1, INTERRUPT_3, (uint8_t*)storedSectorHeader, 8);
}

void CdDrive::getLocP() {
	uint8_t track = 0xff;
	uint8_t index = 0;
	uint8_t mm;
	uint8_t ss;
	uint8_t sect;

	for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].trackStart.amm * 60 * 75 + cdTracks[i].trackStart.ass * 75 + cdTracks[i].trackStart.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].trackEnd.amm * 60 * 75 + cdTracks[i].trackEnd.ass * 75 + cdTracks[i].trackEnd.asect))) {

			track = cdTracks[i].track;

			uint32_t diff = ((amm * 60 * 75 + ass * 75 + asect) -
				(cdTracks[i].trackStart.amm * 60 * 75 + cdTracks[i].trackStart.ass * 75 + cdTracks[i].trackStart.asect));

			mm = diff / (60 * 75);
			diff -= mm * (60 * 75);

			ss = diff / 75;
			diff -= ss * 75;

			sect = diff;

			index = 0;
			if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 0))
				index = 1;

			//Vib-Ribbon
			/*if (track == 1)
				index = 1;
			else if (track == 2) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (11 * 75 + 8))
					index = 1;
			}
			else if (track == 3) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 3))
					index = 1;
			}
			else if (track == 4) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 31))
					index = 1;
			}
			else if (track == 5) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 0))
					index = 1;
			}
			else if (track == 6) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 18))
					index = 1;
			}
			else if (track == 7) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 66))
					index = 1;
			}
			else if (track == 8) {
				if ((mm * 60 * 75 + ss * 75 + sect) >= (2 * 75 + 61))
					index = 1;
			}*/
		}
	}

	if (track == 0xff) {
		std::cout << std::hex << "~[CD_DRIVE] GetLocP incorrect track number: " << std::dec << (uint16_t)track << '\n';
		g_isEmulationPaused = true;
	}

	std::vector<uint8_t> firstResponse;
	firstResponse.resize(8);
	//if (bPlay)
	//	firstResponse = { fromHex8ToBCD(track), fromHex8ToBCD(index), 
	// fromHex8ToBCD(pBus->spu.ammCDDA), fromHex8ToBCD(pBus->spu.assCDDA), fromHex8ToBCD(pBus->spu.asectCDDA),
	//		fromHex8ToBCD(pBus->spu.ammCDDA), fromHex8ToBCD(pBus->spu.assCDDA), fromHex8ToBCD(pBus->spu.asectCDDA) };
	//else //tomb raider needs index 0
	//	//firstResponse = { fromHex8ToBCD(track), fromHex8ToBCD(index), 
	// fromHex8ToBCD(pBus->spu.ammXA), fromHex8ToBCD(pBus->spu.assXA), fromHex8ToBCD(pBus->spu.asectXA),
	//	//	fromHex8ToBCD(pBus->spu.ammXA), fromHex8ToBCD(pBus->spu.assXA), fromHex8ToBCD(pBus->spu.asectXA) }; //??
	firstResponse = { ConvFromHex8ToBcd(track), ConvFromHex8ToBcd(index), 
		ConvFromHex8ToBcd(mm), ConvFromHex8ToBcd(ss), ConvFromHex8ToBcd(sect),
		ConvFromHex8ToBcd(amm), ConvFromHex8ToBcd(ass), ConvFromHex8ToBcd(asect) };

	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
}

void CdDrive::getTN() {
	uint8_t firstTrack = 1;
	uint8_t lastTrack = (uint8_t)cdTracks.size();
	uint8_t firstResponse[3] = { stat.data, ConvFromHex8ToBcd(firstTrack), ConvFromHex8ToBcd(lastTrack) };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 3);
}

void CdDrive::getTD() {
	std::vector<uint8_t> firstResponse;
	uint8_t specifiedTrackBcd;
	GetParameters(&specifiedTrackBcd, 1);
	uint8_t specifiedTrack = ConvFromBcdToHex8(specifiedTrackBcd);

	if (specifiedTrack > cdTracks.size()) {
		uint8_t errorCode = 0x10;
		firstResponse.resize(1);
		firstResponse = { errorCode };
		SetFirstResult(0xc4e1, INTERRUPT_5, firstResponse.data(), firstResponse.size());

		std::cout << std::hex << "~[CD_DRIVE] GetTD incorrect track number: " << std::dec << (uint16_t)specifiedTrack << '\n';
		g_isEmulationPaused = true;
		return;
	}

	uint8_t asect;
	uint8_t ass;
	uint8_t amm;

	if (specifiedTrack == 0x00) {
		asect = cdTracks[cdTracks.size() - 1].trackEnd.asect;
		ass = cdTracks[cdTracks.size() - 1].trackEnd.ass;
		amm = cdTracks[cdTracks.size() - 1].trackEnd.amm;
	}
	else {
		asect = cdTracks[specifiedTrack - 1].trackStart.asect;
		ass = cdTracks[specifiedTrack - 1].trackStart.ass;
		amm = cdTracks[specifiedTrack - 1].trackStart.amm;
		if (specifiedTrack > 1) { //relative to index 1
			asect += 0;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}

		//Vib-Ribbon
		/*if (specifiedTrack == 2) {
			asect += 8;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 11;
		}
		else if (specifiedTrack == 3) {
			asect += 3;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}
		else if (specifiedTrack == 4) {
			asect += 31;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}
		else if (specifiedTrack == 5) {
			asect += 0;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}
		else if (specifiedTrack == 6) {
			asect += 18;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}
		else if (specifiedTrack == 7) {
			asect += 66;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}
		else if (specifiedTrack == 8) {
			asect += 61;
			if (asect >= 75) {
				asect -= 75;
				ass++;
			}
			ass += 2;
		}*/
	}

	firstResponse.resize(3);
	firstResponse = { stat.data, ConvFromHex8ToBcd(amm), ConvFromHex8ToBcd(ass) };

	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
}

void CdDrive::seekL() {
	stat.elem.spindleMotor = 1;

	CancelReading();

	if (bSetLocUnprocessed)
		bSetLocUnprocessed = false;
	else {
		std::cout << "[CD_DRIVE] WARNING! EMULATION PAUSED! Issued SeekL before setLoc!\n";
		g_isEmulationPaused = true;
	}

	amm = unprocessedAmm;
	ass = unprocessedAss;
	asect = unprocessedAsect;

	stat.elem.seek = 1;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.seek = 0;
	uint8_t secondResponse[1] = { stat.data };
	SetSecondResult(0x1c421, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::seekP() {
	stat.elem.spindleMotor = 1;

	CancelReading();

	if (bSetLocUnprocessed)
		bSetLocUnprocessed = false;
	else {
		std::cout << "[CD_DRIVE] WARNING! EMULATION PAUSED! Issued SeekP before setLoc!\n";
		g_isEmulationPaused = true;
	}

	amm = unprocessedAmm;
	ass = unprocessedAss;
	asect = unprocessedAsect;

	stat.elem.seek = 1;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.seek = 0;
	uint8_t secondResponse[1] = { stat.data };
	SetSecondResult(0x1c421, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::test() {
	uint8_t testParam;
	GetParameters(&testParam, 1);
	switch (testParam) {
	case 0x04: {
		stat.elem.spindleMotor = 1;
		std::vector<uint8_t> firstResponse = { stat.data };
		SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
		break;
	}
	case 0x05: {
		std::vector<uint8_t> firstResponse = { 0x20, 0x00 }; //crash bash antipiracy skip
		//std::vector<uint8_t> firstResponse = { 0x01, 0x01 };
		SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
		break;
	}
	case 0x20: {
		//std::vector<uint8_t> firstResponse = { 0x94, 0x09, 0x19, 0xc0 }; //oldest cd drive
		std::vector<uint8_t> firstResponse = { 0xa1, 0x03, 0x06, 0xc3 }; //newest cd drive
		SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
		break;
	}
	default:
		uint8_t subFunction;
		GetParameters(&subFunction, 1);
		std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled test drive sub function 0x" 
			<< std::hex << (uint16_t)subFunction << "\n";
		g_isEmulationPaused = true;
		break;
	}
}

void CdDrive::getID() {
	std::vector<uint8_t> firstResponse = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());

	std::vector<uint8_t> secondResponse;
	ResponseInterrupt secondINT;
	switch (driveStatus) {
	case NO_DISC:
		stat.elem.idError = 1;
		secondResponse = { stat.data, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INTERRUPT_5;
		break;
	case UNLICENCED_DISC_MODE_1:
		stat.elem.spindleMotor = 1;
		stat.elem.idError = 1;
		secondResponse = { stat.data, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		secondINT = INTERRUPT_5;
		break;
	case LICENCED_MODE_2:
		stat.elem.spindleMotor = 1; // can differ after stop command??
		secondResponse = { stat.data, 0x00, 0x20, 0x00, 0x53, 0x43, 0x45, 0x41 };
		secondINT = INTERRUPT_2;
		break;
	default:
		std::cout << "[CD_DRIVE] EMULATION PAUSED! unhandled drive status 0x"
			<< std::hex << driveStatus;
		g_isEmulationPaused = true;
		break;
	}

	SetSecondResult(0x0004a00, secondINT, secondResponse.data(), secondResponse.size());
}

void CdDrive::readS() {
	stat.elem.spindleMotor = 1;

	waitClocksDataResponse = 0;

	//seeking 
	if (!bSetLocIssued) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		std::cout << "[CD_DRIVE] WARNING! EMULATION PAUSED! Issued ReadS without setLoc!\n";
		g_isEmulationPaused = true;
	}
	else {
		if (bSetLocUnprocessed) {
			bSetLocUnprocessed = false;

			amm = unprocessedAmm;
			ass = unprocessedAss;
			asect = unprocessedAsect;

			stat.elem.seek = 1;

			waitClocksDataResponse = (mode.elem.speed ? 0x3ab2b : 0x72732) * 5;
		}
	}

	bSetLocIssued = false;

	CancelReading();

	//if (mode.elem.xa_adpcm) {
	//	pBus->spu.ammXA = amm;
	//	pBus->spu.assXA = ass;
	//	pBus->spu.asectXA = asect;
	//	pBus->spu.currentSample = 28 * 2 * 4 * 18 / (pBus->spu.b8BitsPerSample ? 2 : 1);
	//	pBus->spu.submode.data = 0;
	//}

	uint8_t firstResponse[1] = { stat.data };
	uint32_t firstResponseClocks = 0xc4e1;

	SetFirstResult(firstResponseClocks, INTERRUPT_3, firstResponse, 1);

	//if (stat.elem.seek) { //breaks gran turismo 1
	//	//std::cout << "[CD_DRIVE] EMULATION PAUSED! ReadS\n";
	//	//g_emulationPaused = true;
	//	uint8_t secondResponse[1] = { stat.data };
	//	setSecondResponse(0x1c421, INT2, secondResponse, 1);
	//}

	isDataResponseActive = true;

	waitClocksDataResponse += mode.elem.speed ? 0x3ab2b : 0x72732;
}

void CdDrive::readTOC() {
	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	uint8_t secondResponse[1] = { stat.data };
	SetSecondResult(44100 * 0x300, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::ReadXaAdpcmSector(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src) {
	bool bSectorFound = false;

	while (!bSectorFound) {
		if (fileDataVect.size() < (((amm * 60 + ass - 2) * 75 + asect) * 0x930 + 0x930)) {
			src = emptySectorPtr;
			std::cout << "[CD_DRIVE] Unable to find requested XA_ADPCM sector" << '\n';
			//g_emulationPaused = true;
			break; // fixes problem in SaGa Frontier (USA) but why
		}

		uint32_t currentPossition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;

		src = (uint8_t*)fileDataVect.data() + currentPossition;

		if ((fileDataVect[currentPossition + 0x0f] == 0x02) &&
			(fileDataVect[currentPossition + 0x10 + 0x02] & 0x04) &&
			(fileDataVect[currentPossition + 0x10 + 0x02] & 0x40))
			if (mode.elem.xa_filter)
				bSectorFound = (fileDataVect[currentPossition + 0x10] == filteredFile) &&
				(fileDataVect[currentPossition + 0x10 + 0x01] == filteredChannel);
			else
				bSectorFound = true;

		/*if (bSectorFound) {
			std::cout << "~[CD_DRIVE] Read sectorXA:" << '\n';
			std::cout << std::dec << "~[CD_DRIVE] ammXA: " << (uint16_t)amm << '\n';
			std::cout << std::dec << "~[CD_DRIVE] assXA: " << (uint16_t)ass << '\n';
			std::cout << std::dec << "~[CD_DRIVE] asectXA: " << (uint16_t)asect << '\n';
		}*/

		if (++asect == 75) {
			asect = 0;
			if (++ass == 60) {
				ass = 0;
				amm++;
			}
		}
	}
	busPtr->spu.submode.data = src[0x10 + 0x02];
	busPtr->spu.codinginfo.data = src[0x10 + 0x03];
	busPtr->spu.bStereo = busPtr->spu.codinginfo.elem.monoStereo;
	busPtr->spu.bSampleRate18900Hz = busPtr->spu.codinginfo.elem.sampleRate;
	busPtr->spu.b8BitsPerSample = busPtr->spu.codinginfo.elem.bitsPerSample;
}

void CdDrive::ReadCddaSector(uint8_t& amm, uint8_t& ass, uint8_t& asect, uint8_t*& src) {
	uint32_t currentPossition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;
	src = (uint8_t*)fileDataVect.data() + currentPossition;

	if (++asect == 75) {
		asect = 0;
		if (++ass == 60) {
			ass = 0;
			amm++;
		}
	}
}


//debug

void CdDrive::PrintStatus() {
	std::cout << "~[CD_DRIVE] stat: 0x" << std::hex << (uint16_t)stat.data << '\n';
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
		g_isEmulationPaused = true;
	}
}