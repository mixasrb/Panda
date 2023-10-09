#include "mdec.h"

extern bool g_emulationPaused;

mdec::mdec() {
	commandLookup.resize(7);
	commandLookup = {
		{&mdec::noFunction,"No function"},
		{&mdec::decodeMacroblock,"Decode Macroblock(s)"},
		{&mdec::setQuantTable,"Set Quant Table(s)"},
		{&mdec::setScaleTable,"Set Scale Table"},
		{&mdec::invalid,"Invalid"},
		{&mdec::invalid,"Invalid"},
		{&mdec::invalid,"Invalid"},
		{&mdec::invalid,"Invalid"}
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

void mdec::readMdec32(const uint32_t& addr, uint32_t& data) {
	if (addr == 0x1f801824) {
		std::cout << "[MDEC] read32 addr 0x" << addr << std::endl;
		data = statusRegister.data;
	}
	else if (addr == 0x1f801820) {
		data = rand();
		/*if (dst_num == 0) {
			data = rand();
		}
		else {
			dst_num--;
			data = dst[dst_num];
		}*/
	}
	else {
		std::cout << "[MDEC] EMULATION PAUSED! unhandled read32 addr 0x" << addr << std::endl;
		g_emulationPaused = true;
	}
}

void mdec::writeMdec32(const uint32_t& addr, const uint32_t& data) {
	if (addr == 0x1f801820) {
		/*	std::cout << "[MDEC] EMULATION PAUSED! unhandled write32 addr 0x" << addr
				<< " data 0x" << data << std::endl;*/
		if (recievedData32Count) {
			recievedData32.push_back(data);
			recievedData32Count--;
		}
		else {
			command = data;

			switch (command >> 29) {
			case 1:
				recievedData32Count = command & 0x0000ffff;
				break;
			case 2:
				if (command & 0x1)
					recievedData32Count = 128 / 4;
				else
					recievedData32Count = 64 / 4;
				break;
			case 3:
				recievedData32Count = 64 / 2;
				break;
			default:
				std::cout << "[MDEC] EMULATION PAUSED! unhandled command 0x"
					<< command << std::endl;
				g_emulationPaused = true;
				break;
			}

			statusRegister.elem.dataInFifoFull = 0;
			if (bEnabledDma0)
				statusRegister.elem.dataInRequest = 1;
			statusRegister.elem.dataOutRequest = 0;
			//statusRegister.elem.commandBusy = 1;
		}

		if (recievedData32Count == 0) {
			std::cout << "~[MDEC] command executed " << commandLookup[command >> 29].name << std::endl;
			statusRegister.elem.dataInFifoFull = 1;
			(this->*commandLookup[command >> 29].command)(command);
			if (bEnabledDma1)
				statusRegister.elem.dataOutRequest = 1;
			recievedData32.clear();
		}
	}
	else if (addr == 0x1f801824) {
		std::cout << "[MDEC] EMULATION PAUSED! unhandled write32 addr 0x" << addr
			<< " data 0x" << data << std::endl;
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

void mdec::clock() {
}

void mdec::decodeMacroblock(uint32_t command) {
	statusRegister.elem.dataOutputDepth = (command & 0x30000000) >> 28;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;

	src = (uint16_t*)recievedData32.data();
	/*if ((statusRegister.elem.dataOutputDepth == 2) ||
		(statusRegister.elem.dataOutputDepth == 3)) {
		dst_num = (8 + 8) * 16;
		rl_decode_block(Crblk, src, (uint16_t*)quantTableUV);
		rl_decode_block(Cbblk, src, (uint16_t*)quantTableUV);
		rl_decode_block(Yblk, src, (uint16_t*)quantTableY); yuv_to_rgb(0, 0);
		rl_decode_block(Yblk, src, (uint16_t*)quantTableY); yuv_to_rgb(0, 8);
		rl_decode_block(Yblk, src, (uint16_t*)quantTableY); yuv_to_rgb(8, 0);
		rl_decode_block(Yblk, src, (uint16_t*)quantTableY), yuv_to_rgb(8, 8);
	}*/
}

void mdec::setQuantTable(uint32_t command) {
	statusRegister.elem.dataOutputDepth = (command & 0x30000000) >> 28;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;

	/*for (uint32_t i = 0; i < 16; i++) {
		for (uint32_t j = 0; j < 4; j++) {
			quantTableY[i * 4 + j] = (uint8_t)(recievedData32[i] >> (8 * j));
		}
	}
	if (command & 0x1) {
		for (uint32_t i = 16; i < 32; i++) {
			for (uint32_t j = 0; j < 4; j++) {
				quantTableUV[i * 4 + j] = (uint8_t)(recievedData32[i] >> (8 * j));
			}
		}
	}*/
}

void mdec::setScaleTable(uint32_t command) {
	statusRegister.elem.dataOutputDepth = (command & 0x30000000) >> 28;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;

	/*for (uint32_t i = 0; i < 32; i++) {
		for (uint32_t j = 0; j < 2; j++) {
			scaleTable[i * 2 + j] = (uint16_t)(recievedData32[i] >> (16 * j));
		}
	}*/
}

void mdec::noFunction(uint32_t command) {
	statusRegister.elem.dataOutputDepth = (command & 0x30000000) >> 28;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;
}

void mdec::invalid(uint32_t command) {
}

static uint16_t singned10bit(uint16_t val) {
	return (val & 0x200) ? val | 0xfc00 : val;
}

static uint16_t minmax(int16_t val, int16_t min, int16_t max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void mdec::rl_decode_block(uint16_t* blk, uint16_t*& src, uint16_t* qt) {
	uint16_t n = 0;
	int k = 0;
	uint16_t q_scale = 0;
	uint16_t val = 0;
	for (int i = 0; i < 64; i++) {
		blk[i] = 0;
	}
skip:
	n = *src; src++; k = 0;
	if (n == 0xfe00) goto skip;
	q_scale = (n >> 10) & 0x3f;
	val = singned10bit(n & 0x3ff) * qt[k];
lop:
	if (q_scale == 0) val = singned10bit(n & 0x3ff) * 2;
	val = minmax(val, -0x400, 0x3ff);
	if (q_scale > 0) blk[zagzig[k]] = val;
	if (q_scale == 0) blk[k] = val;
	n = *src; src++;
	k = k + ((n >> 10) & 0x3f) + 1;
	val = (singned10bit(n & 0x3ff) * qt[k] * q_scale + 4) / 8;
	if (k <= 63) goto lop;
	real_idct_core(blk);
	return;
}

static void swap(uint16_t*& a, uint16_t*& b) {
	uint16_t* t = b;
	b = a;
	a = t;
}

void mdec::real_idct_core(uint16_t* blk) {
	uint16_t* src = blk; uint16_t* dst = temb_buffer;
	for (int pass = 0; pass <= 1; pass++) {
		for (int x = 0; x <= 7; x++) {
			for (int y = 0; y <= 7; y++) {
				uint32_t sum = 0;
				for (int z = 0; z <= 7; z++) {
					sum = sum + src[y + z * 8] * (scaleTable[x + z * 8] / 8);
				}
				dst[x + y * 8] = (sum + 0xfff) / 0x2000;
			}
		}
		swap(src, dst);
	}
}

static float MinMax(float min, float max, float val) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void mdec::yuv_to_rgb(int xx, int yy) {
	for (int x = 0; x <= 7; x++) {
		for (int y = 0; y <= 7; y++) {
			float R = *(Crblk + ((x + xx) / 2) + ((y + yy) / 2) * 8);
			float B = *(Cbblk + ((x + xx) / 2) + ((y + yy) / 2) * 8);
			float G = (-0.3437 * B) + (-0.7143 * R);
			R = (1.402 * R); B = (1.772 * B);
			float Y = *(Yblk + (x)+(y) * 8);
			R = MinMax(-128., 127., (Y + R));
			G = MinMax(-128., 127., (Y + G));
			B = MinMax(-128., 127., (Y + B));
			uint32_t BGR = (((int8_t)B) << 16) | (((int8_t)G) << 8) | ((int8_t)B);
			if (statusRegister.elem.dataOutputSigned == 0) BGR = BGR ^ 0x808080;
			dst[(x + xx) + (y + yy) * 16] = BGR;
		}
	}
	statusRegister.elem.dataOutFifoEmpty = 0;
}
