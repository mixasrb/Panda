#include "mdec.h"

extern bool g_emulationPaused;

mdec_t::mdec_t() {
	commandLookup.resize(7);
	commandLookup = {
		{&mdec_t::noFunction,"No function"},
		{&mdec_t::decodeMacroblock,"Decode Macroblock(s)"},
		{&mdec_t::setQuantTable,"Set Quant Table(s)"},
		{&mdec_t::setScaleTable,"Set Scale Table"},
		{&mdec_t::invalid,"Invalid"},
		{&mdec_t::invalid,"Invalid"},
		{&mdec_t::invalid,"Invalid"},
		{&mdec_t::invalid,"Invalid"}
	};

	zigzag.resize(64);
	zigzag = {
		0, 1, 5, 6, 14, 15, 27, 28,
		2, 4, 7, 13, 16, 26, 29, 42,
		3, 8, 12, 17, 25, 30, 41, 43,
		9, 11, 18, 24, 31, 40, 44, 53,
		10, 19, 23, 32, 39, 45, 52, 54,
		20, 22, 33, 38, 46, 51, 55, 60,
		21, 34, 37, 47, 50, 56, 59, 61,
		35, 36, 48, 49, 57, 58, 62, 63
	};

	zagzig.resize(64);
	for (int i = 0; i < 64; i++) {
		zagzig[zigzag[i]] = i;
	}

}

union psxColor16_t {
	psxColor16_t() {
		data = 0;
	}
	psxColor16_t(uint16_t input) : data(input) {
	}
	uint16_t data;
	struct {
		uint16_t red : 5;
		uint16_t green : 5;
		uint16_t blue : 5;
		uint16_t semiTransparencyFlag : 1;
	}component;
	psxColor16_t operator* (float a) {
		psxColor16_t v;
		v.component.red = (uint16_t)(a * (float)component.red);
		v.component.green = (uint16_t)(a * (float)component.green);
		v.component.blue = (uint16_t)(a * (float)component.blue);
		return v;
	}
	psxColor16_t operator/ (float a) {
		psxColor16_t v;
		v.component.red = (uint16_t)(a / (float)component.red);
		v.component.green = (uint16_t)(a / (float)component.green);
		v.component.blue = (uint16_t)(a / (float)component.blue);
		return v;
	}
	psxColor16_t operator+ (psxColor16_t v1) {
		psxColor16_t v2;
		v2.component.red = v1.component.red + component.red;
		v2.component.green = v1.component.green + component.green;
		v2.component.blue = v1.component.blue + component.blue;
		return v2;
	}
	psxColor16_t operator* (psxColor16_t v1) {
		psxColor16_t v2;
		v2.component.red = v1.component.red + component.red;
		v2.component.green = v1.component.green + component.green;
		v2.component.blue = v1.component.blue + component.blue;
		return v2;
	}
};

void mdec_t::readMdec32(const uint32_t& addr, uint32_t& data) {
	if (addr == 0x1f801820) {

		//data = rand();
		//if (i >= dst_num) {
		//	/*psxColor16_t color1;
		//	color1.component.blue = 0x1f;
		//	psxColor16_t color2;
		//	color2.component.red = 0x1f;
		//	data = 0xffffffff;*/
		//	statusRegister.elem.dataOutFifoEmpty = 1;
		//}
		//else {

		if (i >= dstBlock) {
			std::cout << "[MDEC] EMULATION PAUSED! data out fifo overun 0x" << i << std::endl;
			data = rand();
			return;
			//g_emulationPaused = true;
		}

		data = dst[i];
		i++;
		if (i >= dstBlock)
			statusRegister.elem.dataOutFifoEmpty = 1;

		//	}
	}
	else if (addr == 0x1f801824) {
		//std::cout << "[MDEC] read32 addr 0x" << addr << std::endl;
		data = statusRegister.data;
	}
	else {
		std::cout << "[MDEC] EMULATION PAUSED! unhandled read32 addr 0x" << addr << std::endl;
		g_emulationPaused = true;
	}
}

void mdec_t::writeMdec32(const uint32_t& addr, const uint32_t& data) {
	if (addr == 0x1f801820) {
		if (recievedData32Count) {
			rle_t temp_rle;
			temp_rle.data = (uint16_t)data;
			recievedData16.push_back(temp_rle);
			temp_rle.data = (uint16_t)(data >> 16);
			recievedData16.push_back(temp_rle);

			/*	std::cout << "[MDEC] recieved data 0x" << (uint16_t)data << std::endl;
				std::cout << "[MDEC] recieved data 0x" << (uint16_t)(data >> 16) << std::endl;*/

			recievedData32.push_back(data);
			recievedData32Count--;

			statusRegister.elem.numberOfParam = (uint16_t)recievedData32Count - 1;
		}
		else {
			command = data;

			switch (command >> 29) {
			case 1:
				recievedData32Count = command & 0x0000ffff;
				clocks = 3000000/1000;
				break;
			case 2:
				if (command & 0x1)
					recievedData32Count = 128 / 4;
				else
					recievedData32Count = 64 / 4;
				clocks = 0x30;
				break;
			case 3:
				recievedData32Count = 64 / 2;
				clocks = 0x30;
				break;
			default:
				std::cout << "[MDEC] EMULATION PAUSED! unhandled command 0x"
					<< command << std::endl;
				g_emulationPaused = true;
				break;
			}

			statusRegister.elem.numberOfParam = (uint16_t)recievedData32Count - 1;

			statusRegister.elem.dataInFifoFull = 0;

			if (bEnabledDma0)
				statusRegister.elem.dataInRequest = 1;

			statusRegister.elem.dataOutRequest = 0;//??

			statusRegister.elem.commandBusy = 1;
		}

		if (recievedData32Count == 0) {
			statusRegister.elem.dataInFifoFull = 1;

			(this->*commandLookup[command >> 29].command)(command);


			/*	std::cout << "~[MDEC] recievedData16 count: 0x"
					<< std::hex << recievedData16.size() << std::endl;
				std::cout << "~[MDEC] recievedData32 count: 0x"
					<< std::hex << recievedData32.size() << std::endl;
				std::cout << "~[MDEC] command executed " << commandLookup[command >> 29].name << std::endl;*/
				//g_emulationPaused = true;

			recievedData16.clear();
			recievedData32.clear();
		}
	}
	else if (addr == 0x1f801824) {
		/*	std::cout << "[MDEC] EMULATION PAUSED! unhandled write32 addr 0x" << addr
				<< " data 0x" << data << std::endl;*/

		controlRegister.data = data;
		if (controlRegister.elem.resetMdec) {
			controlRegister.elem.resetMdec = 0;
			statusRegister.data = 0x80040000;
		}
		bEnabledDma0 = controlRegister.elem.dataInRequest;
		bEnabledDma1 = controlRegister.elem.dataOutRequest;
	}
	else {
		std::cout << "[MDEC] EMULATION PAUSED! unhandled write32 addr 0x" << addr
			<< " data 0x" << data << std::endl;
		g_emulationPaused = true;
	}
}

void mdec_t::clock() {
	if (clocks) {


		clocks--;

		if ((statusRegister.elem.dataOutputDepth == 2) ||
			(statusRegister.elem.dataOutputDepth == 3)) {
			if (clocks % 200 == 0)
				statusRegister.elem.currentBlock = (statusRegister.elem.currentBlock - 1) % 6;//??
		}
		else
			statusRegister.elem.currentBlock = 0x4;

		if (clocks == 0) {
			if (bEnabledDma1)
				statusRegister.elem.dataOutRequest = 1;
			statusRegister.elem.commandBusy = 0;
		}
	}
}

void mdec_t::decodeMacroblock(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x04000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x02000000) ? 1 : 0;
	i = 0;
	dstBlock = 0;
	src = (uint16_t*)recievedData16.data();
	srcTracker = 0;
	blockCount = 0;


	for (; srcTracker < recievedData16.size() - 1;) {
		if ((statusRegister.elem.dataOutputDepth == 2) ||
			(statusRegister.elem.dataOutputDepth == 3)) {
			statusRegister.elem.currentBlock = 5;

			rl_decode_block((uint16_t*)Crblk, src, quantTableUV);
			if (srcTracker >= recievedData16.size())
				return;
			rl_decode_block((uint16_t*)Cbblk, src, quantTableUV);

			rl_decode_block((uint16_t*)Yblk, src, quantTableY);
			yuv_to_rgb(0, 0);

			rl_decode_block((uint16_t*)Yblk, src, quantTableY);
			yuv_to_rgb(8, 0);

			rl_decode_block((uint16_t*)Yblk, src, quantTableY);
			yuv_to_rgb(0, 8);

			rl_decode_block((uint16_t*)Yblk, src, quantTableY);
			yuv_to_rgb(8, 8);

			if (statusRegister.elem.dataOutputDepth == 3)
				dstBlock += 4 * 8 * 2 / 4 * 8;
			else
				dstBlock += 4 * 8 * 3 / 4 * 8;
		}
		else {
			statusRegister.elem.currentBlock = 4;
			rl_decode_block((uint16_t*)Yblk, src, quantTableY);
			y_to_mono();
		}
		//g_emulationPaused = true;
	}
}

void mdec_t::setQuantTable(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x04000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x02000000) ? 1 : 0;

	for (uint32_t i = 0; i < 16; i++) {
		for (uint32_t j = 0; j < 4; j++) {
			quantTableY[i * 4 + j] = (uint8_t)(recievedData32[i] >> (8 * j));
		}
	}
	if (command & 0x1) {
		for (uint32_t i = 16; i < 32; i++) {
			for (uint32_t j = 0; j < 4; j++) {
				quantTableUV[(i - 16) * 4 + j] = (uint8_t)(recievedData32[i] >> (8 * j));
			}
		}
	}
}

void mdec_t::setScaleTable(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x04000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x02000000) ? 1 : 0;

	for (uint32_t i = 0; i < 32; i++) {
		for (uint32_t j = 0; j < 2; j++) {
			scaleTable[i * 2 + j] = (uint16_t)(recievedData32[i] >> (16 * j));
		}
	}
}

void mdec_t::noFunction(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x04000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x02000000) ? 1 : 0;
}

void mdec_t::invalid(uint32_t command) {
}

static uint16_t singned10bit(uint16_t val) {
	return ((int16_t)(val << 6)) >> 6;
}

static uint16_t minmax(int16_t val, int16_t min, int16_t max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

//void mdec_t::rl_decode_block(uint16_t* blk, uint16_t*& src, uint8_t* qt) {
//	uint16_t n = 0;
//	int k = 0;
//	uint16_t q_scale = 0;
//	int16_t val = 0;
//
//	for (int i = 0; i < 64; i++) {
//		blk[i] = 0;
//	}
//
//	if (srcTracker >= recievedData16.size())
//		return;
//skip:
//	n = *src; src++; k = 0;
//	srcTracker++;
//
//	if (srcTracker >= recievedData16.size())
//		return;
//
//	if (n == 0xfe00) goto skip;
//
//	q_scale = (n >> 10) & 0x3f;
//	val = singned10bit(n & 0x3ff) * qt[k];
//lop:
//	if (q_scale == 0)
//		val = singned10bit(n & 0x3ff) * 2;
//
//	val = minmax(val, -0x400, 0x3ff);
//
//	if (q_scale > 0)
//		blk[zagzig[k]] = val;
//
//	if (q_scale == 0)
//		blk[k] = val;
//
//	n = *src; src++;
//	srcTracker++;
//	k = k + ((n >> 10) & 0x3f) + 1;
//	val = (singned10bit(n & 0x3ff) * qt[k] * q_scale + 4) / 8;
//
//	if (k <= 63) goto lop;
//
//
//	real_idct_core(blk);
//
//	return;
//}

void mdec_t::rl_decode_block(uint16_t* coefficientList, uint16_t*& src, uint8_t* qt) {

skip:
	if (srcTracker >= recievedData16.size() - 1)
		return;
	if (recievedData16[srcTracker].data == 0xfe00) {
		if (srcTracker >= recievedData16.size() - 1)
			return;
		srcTracker++;

		goto skip;
	}

	uint16_t quantizationScale = recievedData16[srcTracker].elem.factor;
	uint16_t dcCoefficient = recievedData16[srcTracker].elem.value;
	srcTracker++;
	//if (srcTracker >= recievedData16.size() - 1)
	//	return;

	for (int i = 0; i < 64; i++) {
		coefficientList[i] = 0;
	}

	coefficientList[0] = singned10bit(dcCoefficient);

	uint16_t runLenghtCode = recievedData16[srcTracker].data;
	srcTracker++;
	/*if (srcTracker >= recievedData16.size() - 1)
		return;*/

	int i = 1 + ((runLenghtCode >> 10) & 0x3f);
	while (runLenghtCode != 0xfe00) {
		coefficientList[i] = singned10bit(runLenghtCode & 0x3ff);
		runLenghtCode = recievedData16[srcTracker].data;
		i += 1 + ((runLenghtCode >> 10) & 0x3f);
		if (i > 63)
			break;
		srcTracker++;
		/*if (srcTracker >= recievedData16.size() - 1)
			break;*/
	}

	int16_t coefficientMatrix[64];

	for (int i = 0; i < 64; i++) {
		if (quantizationScale > 0)
			coefficientMatrix[i] = coefficientList[zigzag[i]];
		else
			coefficientMatrix[i] = coefficientList[i];
	}

	int16_t dequantizedMatrix[64];

	for (int i = 0; i < 64; i++) {
		if (i == 0)
			/* The DC coefficient is not multiplied
			   by the quantization scale */
			dequantizedMatrix[i] = minmax(coefficientMatrix[i] * qt[i],
				-0x400, 0x3ff);
		else
			if (quantizationScale > 0)
				dequantizedMatrix[i] = minmax((coefficientMatrix[i] * quantizationScale * qt[i] + 4) / 8,
					-0x400, 0x3ff);
			else
				dequantizedMatrix[i] = minmax(coefficientMatrix[i] * 2,
					-0x400, 0x3ff);
	}

	for (int i = 0; i < 64; i++) {
		coefficientList[i] = dequantizedMatrix[i];
	}

	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			float sum = 0;

			for (int dct_x = 0; dct_x < 8; dct_x++) {
				for (int dct_y = 0; dct_y < 8; dct_y++) {

					float Sub_Total = dequantizedMatrix[dct_x + dct_y * 8];

					if (dct_x == 0)
						Sub_Total *= sqrt(1.0f / 8.0f);
					else
						Sub_Total *= sqrt(2.0f / 8.0f);

					if (dct_y == 0)
						Sub_Total *= sqrt(1.0f / 8.0f);
					else
						Sub_Total *= sqrt(2.0f / 8.0f);

					Sub_Total *=
						cos(float(dct_x * 3.1412f * (2.0f * float(x) + 1.0f) / (2.0f * 8.0f)));
					Sub_Total *=
						cos(float(dct_y * 3.1412f * (2.0f * float(y) + 1.0f) / (2.0f * 8.0f)));

					sum += Sub_Total;
				}
			}

			coefficientList[x + y * 8] = (int16_t)sum;
		}
	}


	//real_idct_core(coefficientList);

	return;
}

static void swap(uint16_t*& a, uint16_t*& b) {
	uint16_t* t = b;
	b = a;
	a = t;
}

void mdec_t::real_idct_core(uint16_t* blk) {
	uint16_t* src = blk; uint16_t* dst = temb_buffer;
	for (int pass = 0; pass <= 1; pass++) {
		for (int x = 0; x <= 7; x++) {
			for (int y = 0; y <= 7; y++) {
				int32_t sum = 0;
				for (int z = 0; z <= 7; z++) {
					sum = sum + (int32_t)(int16_t)src[y + z * 8] * ((int32_t)(int16_t)scaleTable[x + z * 8] / 8);
				}
				dst[x + y * 8] = (sum + 0xfff) / 0x2000;
			}
		}
		swap(src, dst);
	}
}

template<typename T>
static T MinMax(T min, T max, T val) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void mdec_t::yuv_to_rgb(int xx, int yy) {
	//if (src >= recievedData16.data() + recievedData16.size())
	//	return;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {

			float R = (float)*(Crblk + (x + xx) / 2 + (y + yy) / 2 * 8);
			float B = (float)*(Cbblk + (x + xx) / 2 + (y + yy) / 2 * 8);
			float Y = (float)*(Yblk + x + y * 8);

			float G = Y - 0.3437 * B - 0.7143 * R;
			R = Y + 1.402 * R;
			B = Y + 1.772 * B;

			if (statusRegister.elem.dataOutputSigned == 0) {
				R = MinMax(0.0f, 255.0f, R + 128.0f);
				G = MinMax(0.0f, 255.0f, G + 128.0f);
				B = MinMax(0.0f, 255.0f, B + 128.0f);
			}
			else {
				R = MinMax(-128.0f, 127.0f, R);
				G = MinMax(-128.0f, 127.0f, G);
				B = MinMax(-128.0f, 127.0f, B);
			}

			if (statusRegister.elem.dataOutputDepth == 3) {
				psxColor16_t color1;
				color1.component.red = (uint32_t)R >> 3;
				color1.component.green = (uint32_t)G >> 3;
				color1.component.blue = (uint32_t)B >> 3;

				if (statusRegister.elem.dataOutputBit15)
					color1.data |= 0x8000;

				uint16_t destination = ((x + xx) + (y + yy) * 16) / 2;

				if (x % 2 == 0)
					dst[dstBlock + destination] = (uint32_t)color1.data;
				else
					dst[dstBlock + destination] =
					(dst[dstBlock + destination]) | ((uint32_t)color1.data << 16);
			}
			else {
				uint32_t BGR = (((uint32_t)(uint8_t)B) << 16) | (((uint32_t)(uint8_t)G) << 8) | ((uint32_t)(uint8_t)R);

				uint16_t destination = ((x + xx) + (y + yy) * 16) * 3 / 4;

				switch (x % 4) {
				case 0:
					dst[dstBlock + destination] = BGR;
					break;
				case 1:
					dst[dstBlock + destination] = dst[dstBlock + destination] | (BGR << 24);
					dst[dstBlock + destination + 1] = BGR >> 8;
					break;
				case 2:
					dst[dstBlock + destination] = dst[dstBlock + destination] | (BGR << 16);
					dst[dstBlock + destination + 1] = BGR >> 16;
					break;
				case 3:
					dst[dstBlock + destination] = dst[dstBlock + destination] | (BGR << 8);
					break;
				}
			}
		}
	}

	statusRegister.elem.dataOutFifoEmpty = 0;

	if (xx + yy == 16)
		blockCount++;
}

void mdec_t::y_to_mono() {
	/*if (src >= recievedData16.data() + recievedData16.size())
		return;*/

	for (int i = 0; i < 64; i++) {
		dst[dstBlock + i] = 0;
	}
	for (int i = 0; i < 64; i++) {
		int16_t Y = *(Yblk + i);
		//Y = Y & 0x1ff; //??
		Y = MinMax((int16_t)-128, (int16_t)127, Y);
		//Y >>= 1; //??
		if (statusRegister.elem.dataOutputSigned == 0)
			Y = Y ^ 0x80;
		dst[dstBlock + i / 4] = dst[dstBlock + i / 4] | ((uint32_t)(uint8_t)Y << (i % 4) * 8);
	}

	dstBlock += 8 / 4 * 8;
	statusRegister.elem.dataOutFifoEmpty = 0;
	blockCount++;
}
