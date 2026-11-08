#include "hc05_pux.h"
#include "../bus/bus_interface.h"

static uint8_t ConvBcdToHex8(const uint8_t& dec) {
	return (dec >> 4) * 10 + (dec & 0x0f);
}

static uint8_t ConvHex8ToBcd(const uint8_t& hex) {
	return (hex / 10) * 16 + (hex % 10);
}

CdDrive::CdDrive() {
	commandLookup.resize(20);
	commandLookup = {
		{ "Sync",           &CdDrive::Sync },{ "Getstat",     &CdDrive::GetStat },
		{ "Setloc",       &CdDrive::SetLoc },{ "Play",           &CdDrive::Play },
		{ "Forward",       &CdDrive::xxxxx },{ "Backward",      &CdDrive::xxxxx },
		{ "ReadN",         &CdDrive::ReadN },{ "MotorOn",     &CdDrive::MotorOn },
		{ "Stop",           &CdDrive::Stop },{ "Pause",         &CdDrive::Pause },
		{ "Init",           &CdDrive::Init },{ "Mute",           &CdDrive::Mute },
		{ "Demute",       &CdDrive::Demute },{ "Setfilter", &CdDrive::SetFilter },
		{ "Setmode",     &CdDrive::SetMode },{ "Getparam",   &CdDrive::GetParam },
		{ "GetlocL",     &CdDrive::GetLocL },{ "GetlocP",     &CdDrive::GetLocP },
		{ "SetSession",    &CdDrive::xxxxx },{ "GetTN",         &CdDrive::GetTN },
		{ "GetTD",         &CdDrive::GetTD },{ "SeekL",         &CdDrive::SeekL },
		{ "SeekP",         &CdDrive::SeekP },{ "SetClock",      &CdDrive::xxxxx },
		{ "GetClock",      &CdDrive::xxxxx },{ "Test",           &CdDrive::Test },
		{ "GetID",         &CdDrive::GetID },{ "ReadS",         &CdDrive::ReadS },
		{ "Reset",         &CdDrive::xxxxx },{ "GetQ",          &CdDrive::xxxxx },
		{ "ReadTOC",     &CdDrive::ReadTOC },{ "VideoCD",       &CdDrive::xxxxx },
	};
}

void CdDrive::LoadRom(const uint8_t newDriveStatus, std::string newPath) {

	driveStatus = static_cast<DriveStatus>(newDriveStatus);

	if (driveStatus == NO_DISC)
		return;

	std::ifstream file(newPath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		LogCritical(TAG, "Failed to open rom: {}", newPath);
		return;
	}

	size_t fileSize = file.tellg();
	file.seekg(0);
	fileDataVect.resize(fileSize);
	file.read(fileDataVect.data(), fileSize);
	file.close();

	LogInfo(TAG, "Loaded rom: {} ({:.1f} MB)",
		newPath, (float)fileSize / 1024.0 / 1024.0);

	uint16_t i = 1;
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

	size_t found1 = newPath.find(ss1.str());
	size_t found2 = newPath.find(ss2.str());
	if ((found1 != std::string::npos) || (found2 != std::string::npos)) {
		int eraseCount;
		if (found1 != std::string::npos)
			eraseCount = 14;
		else
			eraseCount = 13;

		newPath.erase(newPath.end() - eraseCount, newPath.end());

		do {
			i++;
			std::string iStr = std::to_string(i);

			std::stringstream ss;
			std::ifstream file;
			if (found1 != std::string::npos) {
				ss << newPath.c_str() << "(Track " << ((i < 10) ? "0" : "") << iStr.c_str() << ").bin";
				file.open(ss.str(), std::ios::binary | std::ios::ate);
			}
			else {
				ss << newPath.c_str() << "(Track " << iStr.c_str() << ").bin";
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

				LogInfo(TAG, "Loaded track: {} ({:.1f} MB)",
					ss.str(), (float)fileSizeTemp / 1024.0 / 1024.0);

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
			LogError(TAG, "Result fifo is empty");
			break;
		}

		data = resultFifo.front();
		resultFifo.pop();

		LogTrace(TAG, "Result read: {:#04x}", data);
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
			LogTrace(TAG, "Command received: {}", commandLookup[data].name);

			if (!commandFifo.empty())
				LogError(TAG, "Command received, previous one is not executed");

			if(indexStatus.elem.busysts)
				LogWarn(TAG, "Command received, while previous one is executing");

			if (!pendingInterrupts.empty())
				LogWarn(TAG, "Command received, interrupt {} is pending", (uint8_t)pendingInterrupts.front());

			if (irqFlagRead.elem.int_1_7)
				LogWarn(TAG, "Command received, interrupt {} is not acknowledged", (uint8_t)irqFlagRead.elem.int_1_7);

			commandFifo.push(data);
			break;
		case 1:
			LogError(TAG, "Unhandled 1F801801h.Index1 - Sound Map Data Out (W) {:#04x}", data);
			break;
		case 2:
			LogError(TAG, "Unhandled 1F801801h.Index2 - Sound Map Coding Info (W) {:#04x}", data);
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
			LogTrace(TAG, "Parameter received: {:#04x}", data);
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

			if (requestReg.elem.smen)
				LogError(TAG, "Unhandled SMEN 0x1f801803 bank 0");

			if (requestReg.elem.bfwr)
				LogError(TAG, "Unhandled BFWR 0x1f801803 bank 0");

			if (requestReg.elem.bfrd) {
				indexStatus.elem.drqsts = 1; //!?
				LogTrace(TAG, "Request to load data fifo");
			}

			break;
		case 1:

			if ((data & 0x7) == 0x7) {
				if (irqFlagRead.elem.int_1_7)
					LogTrace(TAG, "Acknowledged interrupt {}", std::to_string((uint16_t)irqFlagRead.elem.int_1_7));

				irqFlagRead.elem.int_1_7 = NO_INTERRUPT;
			}
			if ((0x0 < (data & 0x7)) && ((data & 0x7) < 0x7)) // 0 < (data & 0x7) < 7
				LogError(TAG, "Unhandled interrupt flag register (W), state: {:#04x}", data);

			if (data & 0x8) {
				if (irqFlagRead.elem.int_8)
					LogTrace(TAG, "Acknowledged interrupt 8");

				irqFlagRead.elem.int_8 = 0;
			}
			if (data & 0x10) {
				if (irqFlagRead.elem.int_10)
					LogTrace(TAG, "Acknowledged interrupt 10");

				irqFlagRead.elem.int_10 = 0;
			}
			if (data & 0x20)
				LogError(TAG, "Unhandled Clear sound map XA - ADPCM buffer");

			if (data & 0x40) {
				LogTrace(TAG, "Parameter fifo reseted, count: {}", paramFifo.size());

				while (!paramFifo.empty())
					paramFifo.pop();
			}
			if (data & 0x80)
				LogError(TAG, "Unhandled reseting decoder chip");

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
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPtr[i]);
		LogTrace(TAG, "First result sent: {:#04x}", resultsPtr[i]);
	}

	firstResponseInterrupt = responseINT;
}

void CdDrive::SetSecondResult(const uint32_t& waitClocks, const ResponseInterrupt& responseINT,
	uint8_t* resultsPtr, const uint8_t& resultsCount) {
	isSecondResponsePending = true;
	waitClocksSecondResponse = waitClocks;

	//debug
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPtr[i]);
		LogTrace(TAG, "Second result sent: {:#04x}", resultsPtr[i]);
	}

	secondResponseInterrupt = responseINT;
}

void CdDrive::SetDataResult(const ResponseInterrupt& responseINT,
	uint8_t* resultsPrt, const uint8_t& resultsCount) {

	//debug
	PrintStatus();

	for (uint8_t i = 0; i < resultsCount; i++) {
		resultFifo.push(resultsPrt[i]);
		LogTrace(TAG, "Data result sent: {:#04x}", resultsPrt[i]);
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
			LogTrace(TAG, "Sent interrupt {}", (uint8_t)irqFlagRead.elem.int_1_7);
		if (irqFlagRead.elem.int_8)
			LogError(TAG, "Unhandled sending interrupt 8");
		if (irqFlagRead.elem.int_10)
			LogError(TAG, "Unhandled sending interrupt 10");

		if (interruptEnable.data & irqFlagRead.data)
			busPtr->pCpu->cp0.InterruptHandler(_IRQ_CD_ROM);

		return;
	}

	if (isFirstResponsePending
		&& clocks >= waitClocksFirstResponse) {
		isFirstResponsePending = false;
		clocks = 0;

		SetResponseInterrupt(firstResponseInterrupt);

		if (!isSecondResponsePending)
			indexStatus.elem.busysts = 0;
	}
	else if (!isFirstResponsePending && isSecondResponsePending
		&& clocks >= waitClocksSecondResponse) {
		isSecondResponsePending = false;
		clocks = 0;

		SetResponseInterrupt(secondResponseInterrupt);

		indexStatus.elem.busysts = 0;
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
						dataResponse = { stat.data,  ConvHex8ToBcd(track), index,
						ConvHex8ToBcd(mm), ConvHex8ToBcd(ss + 80), ConvHex8ToBcd(sect),
						0x7f, 0x00 };//??
					else
						dataResponse = { stat.data, ConvHex8ToBcd(track), index,
						ConvHex8ToBcd(storedAmm), ConvHex8ToBcd(storedAss),ConvHex8ToBcd(storedAsect),
						0x7f, 0x00 };//??

					SetDataResult(INTERRUPT_1, dataResponse.data(), 8);
					SetResponseInterrupt(dataResponseInterrupt);
				}
			}
		}
	}

	if (!isFirstResponsePending && !isSecondResponsePending &&
		!commandFifo.empty() && pendingInterrupts.empty()) {
		clocks = 0;

		ExecuteNextCommand();
	}
}

void CdDrive::GetParameters(uint8_t* paramPtr, const uint8_t& paramCount) {
	for (uint8_t i = 0; i < paramCount; i++) {
		if (paramFifo.empty()) {
			LogError(TAG, "Parameters requested before they are available");
			break;
		}

		paramPtr[i] = paramFifo.front();
		paramFifo.pop();
		LogTrace(TAG, "Parameter read: {:#04x}", paramPtr[i]);
	}
}

void CdDrive::ExecuteNextCommand() {
	uint8_t command = commandFifo.front();
	commandFifo.pop();

	indexStatus.elem.busysts = 1;

	//debug
	LogTrace(TAG, "Command executed: {}", commandLookup[command].name);

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

	uint32_t currentPosition = ((amm * 60 + ass - 2) * 75 + asect) * 0x930;

	if ((ass < 2) && (amm == 0)) {
		LogError(TAG, "Inaccessible CD sector (ass < 2)");
		return;
	}

	if (fileDataVect.size() < (currentPosition + 0x930)) {
		LogError(TAG, "Inaccessible CD sector (position out of bounds)");
		return;
	}

	sendINT1 = false;
	if ((bFilter && mode.elem.xa_adpcm && mode.elem.xa_filter
		&& (fileDataVect[currentPosition + 0x12] & 0x04))
		|| (stat.elem.play && (mode.elem.report == 0)))
		sendINT1 = false;
	else if ((!bPlay) ||
		(bPlay && mode.elem.report)) {
		sendINT1 = true;
		dataBufferPtr = (uint8_t*)fileDataVect.data() + (currentPosition + (mode.elem.sectorSize ? 0x0c : 0x18));
	}

	if (!bPlay)
		for (int i = 0; i < 8; i++)
			storedSectorHeader[i] = fileDataVect[currentPosition + 0x0c + i];

	auto fillBufferXA_ADPCM = [&]() {
		busPtr->spu.pBufferXA_ADPCM = (uint8_t*)fileDataVect.data() + currentPosition;
		busPtr->spu.decodeSectorXA_ADPCM(busPtr->spu.pBufferXA_ADPCM);
		busPtr->spu.currentSample = 0;
		};

	if (mode.elem.xa_adpcm &&
		(fileDataVect[currentPosition + 0x0f] == 0x02) &&
		(fileDataVect[currentPosition + 0x10 + 0x02] & 0x04) &&
		(fileDataVect[currentPosition + 0x10 + 0x02] & 0x40))
		if (mode.elem.xa_filter) {
			if ((fileDataVect[currentPosition + 0x10] == filteredFile) &&
				(fileDataVect[currentPosition + 0x10 + 0x01] == filteredChannel))
				fillBufferXA_ADPCM();
		}
		else
			fillBufferXA_ADPCM();

	storedAmm = amm;
	storedAss = ass;
	storedAsect = asect;

	bufferCounter = sectorSize;

	if ((mode.elem.xa_adpcm) && (fileDataVect[currentPosition + 0x12] & 0x4))
		indexStatus.elem.adpbusy = 1;

	waitClocksDataResponse = (mode.elem.speed ? 0x37200 : 0x6e400);
	//waitClocksDataResponse += 0x36cd2 * 10 / 10;

	LogTrace(TAG, "Read sector: {}:{}:{}", amm, ass, asect);

	if (!bPlay)
		if ((uint8_t)fileDataVect[currentPosition + 0x11] == 0xff)
			LogTrace(TAG, "Unused sector"); //?
		else
			switch (((uint8_t)fileDataVect[currentPosition + 0x12] & 0x0f) >> 1 << 1) {
			case 0x0:
				LogTrace(TAG, "Unused sector"); //?
				break;
			case 0x2:
				LogTrace(TAG, "Video sector");
				break;
			case 0x4:
				LogTrace(TAG, "Audio sector");
				break;
			case 0x8:
				LogTrace(TAG, "Data sector");
				break;
			default:
				LogError(TAG, "Unknown sector");
				g_isEmulationPaused = true;
				break;
			}
	else
		LogTrace(TAG, "CD-DA sector");

	for (int i = 0; i < cdTracks.size(); i++) {
		if ((amm == cdTracks[i].trackEnd.amm) &&
			(ass == cdTracks[i].trackEnd.ass) &&
			(asect == cdTracks[i].trackEnd.asect)) {

			if (mode.elem.autoPause) { //Pause
				CancelReading();

				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				SetSecondResult(secondWaitClocks, INTERRUPT_4, secondResponse, 1);

				LogTrace(TAG, "Auto pause");
			}
			else if (i == cdTracks.size() - 1) { //Stop
				CancelReading();

				stat.elem.spindleMotor = 0;
				uint8_t secondResponse[1] = { stat.data };
				uint32_t secondWaitClocks = mode.elem.speed ? 0x10bd93 : 0x21181c;
				SetSecondResult(secondWaitClocks, INTERRUPT_4, secondResponse, 1);

				LogTrace(TAG, "Auto stop, CD end");
			}

		}
	}

	for (int i = 0; i < cdTracks.size(); i++) {
		if (((amm * 60 * 75 + ass * 75 + asect) >=
			(cdTracks[i].trackStart.amm * 60 * 75
				+ cdTracks[i].trackStart.ass * 75
				+ cdTracks[i].trackStart.asect)) &&
			((amm * 60 * 75 + ass * 75 + asect) <=
				(cdTracks[i].trackEnd.amm * 60 * 75
					+ cdTracks[i].trackEnd.ass * 75
					+ cdTracks[i].trackEnd.asect))) {

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
	LogTrace(TAG, "Mode: {:#04x}", mode.data);
	LogTrace(TAG, "	Speed: {}", mode.elem.speed);
	LogTrace(TAG, "	XA-ADCPM: {}", mode.elem.xa_adpcm);
	LogTrace(TAG, "	Sector size: {}", mode.elem.sectorSize);
	LogTrace(TAG, "	Ignore bit: {}", mode.elem.ignoreBit);
	LogTrace(TAG, "	XA-Filter: {}", mode.elem.xa_filter);
	LogTrace(TAG, "	Report: {}", mode.elem.report);
	LogTrace(TAG, "	Auto pause: {}", mode.elem.autoPause);
	LogTrace(TAG, "	CD-DA: {}", mode.elem.cdda);
}

//Cd Drive Commands

void CdDrive::Sync() {
	uint8_t results[2] = { stat.data | STAT_ERROR, 0x40 };
	SetFirstResult(0xc4e1, INTERRUPT_5, results, 2);
}

void CdDrive::xxxxx() {
	LogError(TAG, "Unhandled command");
}

void CdDrive::GetStat() {
	if (!isShellOpen)
		stat.elem.shellOpen = 0;

	uint8_t results[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, results, 1);
}

void CdDrive::Play() {
	stat.elem.spindleMotor = 1;

	if (!bSetLocIssued && paramFifo.empty()) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		LogWarn(TAG, "Play command issued without setLoc or parameter track");
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
		track = ConvBcdToHex8(trackBcd);//??

		amm = cdTracks[track - 1].trackStart.amm;
		ass = cdTracks[track - 1].trackStart.ass;
		asect = cdTracks[track - 1].trackStart.asect;

		stat.elem.seek = 1;
	}

	bSetLocIssued = false;

	CancelReading();

	busPtr->spu.ammCdda = amm;
	busPtr->spu.assCdda = ass;
	busPtr->spu.asectCdda = asect;
	busPtr->spu.counterCdda = 0x930;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.seek = 0; //~ used for gran turismo 2
	stat.elem.play = 1;

	bPlay = true;
	isDataResponseActive = true;
	waitClocksDataResponse = mode.elem.speed ? 0x3ab2b : 0x72732;
}

void CdDrive::SetLoc() {
	bSetLocIssued = true;
	bSetLocUnprocessed = true;

	uint8_t ammTemp;
	uint8_t assTemp;
	uint8_t asectTemp;

	GetParameters(&ammTemp, 1);
	GetParameters(&assTemp, 1);
	GetParameters(&asectTemp, 1);

	if (((ammTemp & 0x0f) > 0x9) || ((ammTemp >> 4) > 0x9) || (ammTemp >= 0x99)) {
		LogError(TAG, "Invalid setlock amm value {}", ammTemp);
		ammTemp = 0;
	}
	if (((assTemp & 0x0f) > 0x9) || ((assTemp >> 4) > 0x5) || (assTemp >= 0x60)) {
		LogError(TAG, "Invalid setlock ass value {}", assTemp);
		assTemp = 0;
	}
	if (((asectTemp & 0x0f) > 0x9) || ((asectTemp >> 4) > 0x7) || (asectTemp >= 0x75)) {
		LogError(TAG, "Invalid setlock asect value {}", asectTemp);
		asectTemp = 0;
	}

	unprocessedAmm = ConvBcdToHex8(ammTemp);
	unprocessedAss = ConvBcdToHex8(assTemp);
	unprocessedAsect = ConvBcdToHex8(asectTemp);

	LogTrace(TAG, "Position: {}:{}:{}",
		unprocessedAmm, unprocessedAss, unprocessedAsect);

	uint8_t results[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, results, 1);
}

void CdDrive::ReadN() {
	stat.elem.spindleMotor = 1;

	waitClocksDataResponse = 0;

	//seeking 
	if (!bSetLocIssued) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		LogError(TAG, "ReadN command issued without setLoc or parameter track");
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

void CdDrive::MotorOn() {
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

void CdDrive::Stop() {
	uint32_t secondWaitClocks;

	if (isDataResponseActive)
		secondWaitClocks = mode.elem.speed ? 0x18a6076 : 0x0d38aca;
	else
		secondWaitClocks = 0x0001d7b;

	CancelReading();

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0x1c4e1, INTERRUPT_3, firstResponse, 1);

	stat.elem.spindleMotor = 0;

	uint8_t secondResponse[1] = { stat.data };

	SetSecondResult(secondWaitClocks, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::Pause() {
	uint32_t secondWaitClocks;

	if (isDataResponseActive)
		secondWaitClocks = mode.elem.speed ? 0x010bd93 : 0x021181c;
	else
		secondWaitClocks = 0x0001df2;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0x1c4e1, INTERRUPT_3, firstResponse, 1);

	CancelReading();

	uint8_t secondResponse[1] = { stat.data };

	SetSecondResult(secondWaitClocks, INTERRUPT_2, secondResponse, 1);
}

void CdDrive::Init() {
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

void CdDrive::Mute() {
	bMuted = true;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::Demute() {
	bMuted = false;

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::SetFilter() {
	bFilter = true;

	GetParameters(&filteredFile, 1);
	LogTrace(TAG, "Filtered file: {:#04x}", filteredFile);

	GetParameters(&filteredChannel, 1);
	LogTrace(TAG, "Filtered channel: {:#04x}", filteredChannel);

	uint8_t response[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, response, 1);
}

void CdDrive::SetMode() {
	GetParameters(&mode.data, 1);

	sectorSize = mode.elem.sectorSize ? 0x924 : 0x800;
	if (mode.elem.xa_adpcm == 0)
		indexStatus.elem.adpbusy = 0;

	PrintDriveMode();

	uint8_t firstResponse[1] = { stat.data };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 1);
}

void CdDrive::GetParam() {
	const uint8_t NULL_BYTE = 0x00;
	uint8_t firstResponse[5] = { stat.data, mode.data, NULL_BYTE, filteredFile, filteredChannel };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 5);
}

void CdDrive::GetLocL() {
	SetFirstResult(0xc4e1, INTERRUPT_3, (uint8_t*)storedSectorHeader, 8);
}

void CdDrive::GetLocP() {
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

	if (track == 0xff) { //?
		LogError(TAG, "GetLocP incorrect track number: {}", track);
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
	firstResponse = { ConvHex8ToBcd(track), ConvHex8ToBcd(index),
		ConvHex8ToBcd(mm), ConvHex8ToBcd(ss), ConvHex8ToBcd(sect),
		ConvHex8ToBcd(amm), ConvHex8ToBcd(ass), ConvHex8ToBcd(asect) };

	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
}

void CdDrive::GetTN() {
	uint8_t firstTrack = 1;
	uint8_t lastTrack = (uint8_t)cdTracks.size();
	uint8_t firstResponse[3] = { stat.data, ConvHex8ToBcd(firstTrack), ConvHex8ToBcd(lastTrack) };
	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse, 3);
}

void CdDrive::GetTD() {
	std::vector<uint8_t> firstResponse;
	uint8_t specifiedTrackBcd;
	GetParameters(&specifiedTrackBcd, 1);
	uint8_t specifiedTrack = ConvBcdToHex8(specifiedTrackBcd);

	if (specifiedTrack > cdTracks.size()) {
		uint8_t errorCode = 0x10;
		firstResponse.resize(1);
		firstResponse = { errorCode };
		SetFirstResult(0xc4e1, INTERRUPT_5, firstResponse.data(), firstResponse.size());

		LogError(TAG, "GetTD incorrect track number: {}", specifiedTrack);
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
	firstResponse = { stat.data, ConvHex8ToBcd(amm), ConvHex8ToBcd(ass) };

	SetFirstResult(0xc4e1, INTERRUPT_3, firstResponse.data(), firstResponse.size());
}

void CdDrive::SeekL() {
	stat.elem.spindleMotor = 1;

	CancelReading();

	if (bSetLocUnprocessed)
		bSetLocUnprocessed = false;
	else
		LogWarn(TAG, "SeekL command issued without SetLoc");

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

void CdDrive::SeekP() {
	stat.elem.spindleMotor = 1;

	CancelReading();

	if (bSetLocUnprocessed)
		bSetLocUnprocessed = false;
	else
		LogWarn(TAG, "SeekP command issued without SetLoc");

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

void CdDrive::Test() {
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
		LogError(TAG, "Unhandled test drive sub function {:02x}", subFunction);
		break;
	}
}

void CdDrive::GetID() {
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
		LogError(TAG, "Unhandled drive status {}", (uint8_t)driveStatus);
		break;
	}

	SetSecondResult(0x0004a00, secondINT, secondResponse.data(), secondResponse.size());
}

void CdDrive::ReadS() {
	stat.elem.spindleMotor = 1;

	waitClocksDataResponse = 0;

	//seeking 
	if (!bSetLocIssued) {
		amm = storedAmm;
		ass = storedAss;
		asect = storedAsect;

		LogWarn(TAG, "ReadS command issued without SetLoc or parameter track");
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

void CdDrive::ReadTOC() {
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
			LogError(TAG, "Unable to find requested XA_ADPCM sector");
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
	LogTrace(TAG, "Status: {:#04x}", stat.data);

	if (stat.elem.play)
		LogTrace(TAG, "  Playing CD-DA");
	if (stat.elem.seek)
		LogTrace(TAG, "  Seeking");
	if (stat.elem.read)
		LogTrace(TAG, "  Reading data sectors");
	if (stat.elem.shellOpen)
		LogTrace(TAG, "	Shell is/was open");
	if (stat.elem.idError)
		LogTrace(TAG, "	GetID denied");
	if (stat.elem.seekError)
		LogTrace(TAG, "	Seek error");
	if (stat.elem.spindleMotor)
		LogTrace(TAG, "  Motor on");
	else
		LogTrace(TAG, "	Motor off, or in spin-up phase");

	if (stat.elem.error)
		LogError(TAG, "	Invalid command or command parameters");

	if ((stat.elem.read && stat.elem.seek) ||
		(stat.elem.read && stat.elem.play) ||
		(stat.elem.seek && stat.elem.play)) {
		LogError(TAG, "Play/Seek/Read status bits set at the same time!");
	}
}