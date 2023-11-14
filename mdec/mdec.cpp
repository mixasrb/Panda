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

		if (i >= dst_num) {
			std::cout << "[MDEC] EMULATION PAUSED! data out fifo overun 0x" << i << std::endl;
			data = rand();
			return;
			//g_emulationPaused = true;
		}

		data = dst[i];
		i++;
		if (i >= dst_num)
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

			std::cout << "[MDEC] recieved data 0x" << (uint16_t)data << std::endl;
			std::cout << "[MDEC] recieved data 0x" << (uint16_t)(data >> 16) << std::endl;

			recievedData32.push_back(data);
			recievedData32Count--;

			statusRegister.elem.numberOfParam = (uint16_t)recievedData32Count - 1;
		}
		else {
			command = data;

			switch (command >> 29) {
			case 1:
				recievedData32Count = command & 0x0000ffff;
				clocks = 360000;
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


			std::cout << "~[MDEC] recievedData16 count: 0x"
				<< std::hex << recievedData16.size() << std::endl;
			std::cout << "~[MDEC] recievedData32 count: 0x"
				<< std::hex << recievedData32.size() << std::endl;
			std::cout << "~[MDEC] command executed " << commandLookup[command >> 29].name << std::endl;
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

		if (clocks == 0)
			statusRegister.elem.commandBusy = 0;
		if (bEnabledDma1)
			statusRegister.elem.dataOutRequest = 1;
	}
}

void mdec_t::decodeMacroblock(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;
	i = 0;
	dst_num = 0;
	src = (uint16_t*)recievedData16.data();
	srcTracker = 0;
	blockCount = 0;
	for (; srcTracker < recievedData16.size();) {
		if ((statusRegister.elem.dataOutputDepth == 2) ||
			(statusRegister.elem.dataOutputDepth == 3)) {
			statusRegister.elem.currentBlock = 5;
			rl_decode_block((uint16_t*)Crblk, src, quantTableUV);
			if (srcTracker >= recievedData16.size())
				return;
			rl_decode_block((uint16_t*)Cbblk, src, quantTableUV);
			rl_decode_block((uint16_t*)Yblk, src, quantTableY); yuv_to_rgb(0, 0);
			rl_decode_block((uint16_t*)Yblk, src, quantTableY); yuv_to_rgb(0, 8);
			rl_decode_block((uint16_t*)Yblk, src, quantTableY); yuv_to_rgb(8, 0);
			rl_decode_block((uint16_t*)Yblk, src, quantTableY), yuv_to_rgb(8, 8);
		}
		else {
			statusRegister.elem.currentBlock = 4;
			rl_decode_block((uint16_t*)Yblk, src, quantTableY); y_to_mono();
		}
		//g_emulationPaused = true;
	}
}

void mdec_t::setQuantTable(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;

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
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;

	for (uint32_t i = 0; i < 32; i++) {
		for (uint32_t j = 0; j < 2; j++) {
			scaleTable[i * 2 + j] = (uint16_t)(recievedData32[i] >> (16 * j));
		}
	}
}

void mdec_t::noFunction(uint32_t command) {
	statusRegister.elem.dataOutputDepth = command << 3 >> 3 >> 27;
	statusRegister.elem.dataOutputSigned = (command & 0x08000000) ? 1 : 0;
	statusRegister.elem.dataOutputBit15 = (command & 0x04000000) ? 1 : 0;
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

void mdec_t::rl_decode_block(uint16_t* blk, uint16_t*& src, uint8_t* qt) {
	uint16_t n = 0;
	int k = 0;
	uint16_t q_scale = 0;
	int16_t val = 0;

	for (int i = 0; i < 64; i++) {
		blk[i] = 0;
	}

	if (srcTracker >= recievedData16.size())
		return;
skip:
	n = *src; src++; k = 0;
	srcTracker++;

	if (srcTracker >= recievedData16.size())
		return;

	if (n == 0xfe00) goto skip;

	q_scale = (n >> 10) & 0x3f;
	val = singned10bit(n & 0x3ff) * qt[k];
lop:
	if (q_scale == 0)
		val = singned10bit(n & 0x3ff) * 2;

	val = minmax(val, -0x400, 0x3ff);

	if (q_scale > 0)
		blk[zagzig[k]] = val;

	if (q_scale == 0)
		blk[k] = val;

	n = *src; src ++;
	srcTracker++;
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
	int i = 0;
	uint32_t oldBGR = 0;
	for (int y = 0; y <= 7; y++) {
		for (int x = 0; x <= 7; x++) {
			float R = *(int16_t*)(Crblk + ((x + xx) / 2) + ((y + yy) / 2) * 8);
			float B = *(int16_t*)(Cbblk + ((x + xx) / 2) + ((y + yy) / 2) * 8);
			float G = (-0.3437 * B) + (-0.7143 * R);
			R = (1.402 * R); B = (1.772 * B);
			float Y = *(int16_t*)(Yblk + (x)+(y) * 8);
			R = MinMax(-128.f, 127.f, (Y + R));
			G = MinMax(-128.f, 127.f, (Y + G));
			B = MinMax(-128.f, 127.f, (Y + B));

			if (statusRegister.elem.dataOutputDepth == 3) {
				psxColor16_t color1;
				color1.component.blue = (uint32_t)B >> 3;
				color1.component.green = (uint32_t)G >> 3;
				color1.component.red = (uint32_t)R >> 3;

				if (statusRegister.elem.dataOutputSigned == 0)
					color1.data ^= 0x101010;

				if (statusRegister.elem.dataOutputBit15)
					color1.data |= 0x800000;

				if (x % 2 == 0)
					dst[dst_num + i / 2] = color1.data;
				else
					dst[dst_num + i / 2] =
					((uint32_t)dst[dst_num + i / 2] << 16) | color1.data;
				i++;
			}
			else {
				uint32_t BGR = (((int8_t)B) << 16) | (((int8_t)G) << 8) | ((int8_t)R);
				if (statusRegister.elem.dataOutputSigned == 0)
					BGR = BGR ^ 0x808080;

				switch (i % 3) {
				case 0:
					if (i != 0)
						dst[dst_num + i - 1] = dst[dst_num + i - 1] | oldBGR;

					dst[dst_num + i] = BGR << 8;
					i++;
					break;
				case 1:
					dst[dst_num + i - 1] = dst[dst_num + i - 1] | (BGR >> 16);
					dst[dst_num + i] = BGR << 16;
					i++;
					break;
				case 2:
					dst[dst_num + i - 1] = dst[dst_num + i - 1] | (BGR >> 8);
					dst[dst_num + i] = BGR << 24;
					i++;
					break;
				}

				oldBGR = BGR;

				//dst[dst_num + (x + xx) + (y + yy) * 16] = BGR;
			}
		}
	}

	if (statusRegister.elem.dataOutputDepth == 3)
		dst_num += 8 * 2 / 4 * 8;
	else
		dst_num += 8 * 3 / 4 * 8;

	statusRegister.elem.dataOutFifoEmpty = 0;

	if (xx + yy == 16)
		blockCount++;
}

void mdec_t::y_to_mono() {
	/*if (src >= recievedData16.data() + recievedData16.size())
		return;*/

	for (int i = 0; i < 64; i++) {
		dst[dst_num + i] = 0;
	}
	for (int i = 0; i < 64; i++) {
		int16_t Y = *(Yblk + i);
		//Y = Y & 0x1ff; //??
		Y = MinMax((int16_t)-128, (int16_t)127, Y);
		//Y >>= 1; //??
		/*if (statusRegister.elem.dataOutputSigned == 0)
			Y = Y ^ 0x80;*/
		dst[dst_num + i / 4] = dst[dst_num + i / 4] | ((uint32_t)(uint8_t)Y << (i % 4) * 8);
	}

	dst_num += 8 / 4 * 8;
	statusRegister.elem.dataOutFifoEmpty = 0;
	blockCount++;
}
