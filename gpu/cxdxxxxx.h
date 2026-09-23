#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>

#include "../defines/defines.h"
#include "../debug_utilities/debug_utilities.h"
#include "../dependencies/olcPixelGameEngine.h"

#define size_x_b 2048
#define size_x 1024 //in half words
#define size_y 512

static uint16_t signed11bit(uint16_t val) {
	return ((int16_t)(val << 5)) >> 5;
}

template <typename T>
struct Fifo { //have some bugs and quarks
	Fifo() {
		ptr = nullptr;
		currentPossitionWrite = 0;
		currentPossitionRead = 0;
	};

	T* ptr;
	uint32_t currentPossitionWrite;
	uint32_t currentPossitionRead;

	void resize(uint32_t newSize) {
		ptr = new T[newSize];
	}

	void reset() {
		currentPossitionWrite = 0;
		currentPossitionRead = 0;
	}

	void pushBack(T data) {
		ptr[currentPossitionWrite] = data;
		currentPossitionWrite++;
	}

	void popBack() {
		currentPossitionWrite--;
	}

	uint32_t size() {
		return currentPossitionWrite;
	}

	bool empty() {
		return (currentPossitionRead >= currentPossitionWrite) ? true : false;
	}

	T getNext() {
		return ptr[currentPossitionRead++];
	}

	T operator[](uint32_t i) {
		return ptr[i];
	}
};

class busInterface_t;

class gpu {
public:

	gpu();

	busInterface_t* pBus;

	void readGpu32(const uint32_t& addr, uint32_t& data);
	void writeGpu32(const uint32_t& addr, const uint32_t& data);

	void videoClock();

	const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
	const uint32_t CPU_CLOCKS_PER_FRAME_NTSC = ((float)CPU_CLOCK / 59.29f);
	const uint32_t CPU_CLOCKS_PER_FRAME_PAL = ((float)CPU_CLOCK / 49.76f);
	const uint32_t SCANLINE_NUMBER_NTSC = 263;
	const uint32_t SCANLINE_NUMBER_PAL = 314;
	const uint32_t CPU_CLOCKS_PER_SCANLINE_NTSC = CPU_CLOCKS_PER_FRAME_NTSC / SCANLINE_NUMBER_NTSC;
	const uint32_t CPU_CLOCKS_PER_SCANLINE_PAL = CPU_CLOCKS_PER_FRAME_PAL / SCANLINE_NUMBER_PAL;
	const uint32_t CPU_CLOCKS_TO_HBLANK_NTSC = CPU_CLOCKS_PER_SCANLINE_NTSC * 3 / 4;
	const uint32_t CPU_CLOCKS_TO_HBLANK_PAL = CPU_CLOCKS_PER_SCANLINE_PAL * 3 / 4;

	const uint32_t cpuDotClocks[5] = {
		CPU_CLOCK / 10,
		CPU_CLOCK / 8,
		CPU_CLOCK / 5,
		CPU_CLOCK / 4,
		CPU_CLOCK / 7
	};

	const uint32_t dotsPerScanline[5] = {
		341,
		426,
		682,
		853,
		487
	};

	uint8_t dotClockDevider = 4;
	uint16_t horizontalResolution = 640;

	uint32_t cpuClocks = 0;
	uint32_t cpuFrameClocks = 0;
	uint32_t cpuScanlineClocks = 0;
	uint32_t cpuClocksToHblank = CPU_CLOCKS_PER_SCANLINE_NTSC;
	uint32_t cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
	uint32_t scanlineNumber = SCANLINE_NUMBER_NTSC;
	uint32_t scanlineCount = -1;

	union gpuStat_t {
		uint32_t data;
		struct {
			uint32_t texPageXBase : 4;
			uint32_t texPageYBase : 1;
			uint32_t semiTransparency : 2;
			uint32_t texPageColors : 2;
			uint32_t dither24To15 : 1;
			uint32_t drawingToDisplayArea : 1;
			uint32_t setMaskBitWhileDrawing : 1;
			uint32_t drawPixels : 1;
			uint32_t interlaceField : 1;
			uint32_t reverseflag : 1;
			uint32_t texDis : 1;
			uint32_t horizontalRes2 : 1;
			uint32_t horizontalRes1 : 2;
			uint32_t verticalRes : 1;
			uint32_t videoMode : 1;
			uint32_t displayAreaColorDepth : 1;
			uint32_t verticalInterlace : 1;
			uint32_t displayEnable : 1;
			uint32_t irq1 : 1;
			uint32_t dataRequestDMA : 1;
			uint32_t readyCommand : 1;
			uint32_t readyVramCpu : 1;
			uint32_t readyBlockDMA : 1;
			uint32_t directionDMA : 2;
			uint32_t drawingEvenOddLines : 1;
		}elem;
	}gpuStat;

	typedef struct {
		uint8_t x;
		uint8_t y;
	}texWinOffset_t, texWinMask_t;

	texWinMask_t texWinMask;
	texWinOffset_t texWinOffset;

	typedef struct {
		uint16_t x;
		uint16_t y;
	}drawArea_t, Coord;

	drawArea_t drawAreaTopLeft;
	drawArea_t drawAreaBottomRight;
	Coord displayStart;

	typedef struct {
		int16_t x;
		int16_t y;
	}drawOffset_t;

	drawOffset_t drawOffset;

	uint16_t screenX1;
	uint16_t screenX2;
	uint16_t screenY1;
	uint16_t screenY2;

	union psxColor16_t {
		psxColor16_t() {
			data = 0;
		}
		psxColor16_t(uint16_t input) : data(input) {}
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

	union psxColor24_t {
		psxColor24_t() : data(0) {}
		psxColor24_t(uint32_t input) : data(input << 8 >> 8) {}
		uint32_t data;
		struct {
			uint32_t red : 8;
			uint32_t green : 8;
			uint32_t blue : 8;
		}component;
	};

	uint16_t format24to16Color(const uint32_t& input) {
		psxColor24_t color24 = (psxColor24_t)(input & 0x00ffffff);
		psxColor16_t color16 = 0;
		color16.component.red = color24.component.red >> 3;
		color16.component.green = color24.component.green >> 3;
		color16.component.blue = color24.component.blue >> 3;
		return color16.data;
	}

	uint32_t format16to24Color(const uint16_t& input) {
		psxColor24_t color24 = 0;
		psxColor16_t color16 = (psxColor16_t)(input & 0x7fff);
		color24.component.red = color16.component.red << 3;
		color24.component.green = color16.component.green << 3;
		color24.component.blue = color16.component.blue << 3;
		return color24.data;
	}

	struct vertex_t {
		vertex_t() {}
		vertex_t(uint32_t xt, uint32_t yt) {
			*(&x) = uint16_t(xt);
			*(&y) = uint16_t(yt);
		}
		vertex_t(uint32_t coord) {
			*(&x) = signed11bit(uint16_t(coord & 0x7ff));
			*(&y) = signed11bit(uint16_t((coord >> 16) & 0x7ff));
		}

		uint16_t signed11bit(uint16_t val) {
			return ((int16_t)(val << 5)) >> 5;
		}

		int16_t x;
		int16_t y;

		vertex_t operator+(vertex_t a) {
			return vertex_t(x + a.x, y + a.y);
		}
		vertex_t operator-(vertex_t a) {
			return vertex_t(x - a.x, y - a.y);
		}
		vertex_t operator*(int c) {
			return vertex_t(c * x, c * y);
		}
	};

	Fifo<uint32_t> fifo;
	Fifo<uint32_t> fifoTemp;
	Fifo<uint32_t> outputFifo;

	enum primitiveType_t {
		POLYGON,
		RECT,
		LINE,
		FILL,
		MEMORY_TRANSFER,
		OTHER
	};

	union texcoordType_t {
		texcoordType_t() {}
		texcoordType_t(uint32_t data) : data(data) {}
		uint32_t data;
		struct {
			uint32_t u : 8;
			uint32_t v : 8;
			uint32_t attribute : 16;
		}elem;
	};

	enum shadingType_t {
		FLAT_SHADING,
		GOURAUD_SHADING
	};

	enum transpanencyType_t {
		SEMI_TRANSPARENT,
		NOT_TRANSPARENT
	};

	enum modulationType_t {
		RAW_TEXTURE,
		TEXTURE_BLENDING
	};

	struct param_t {
		param_t() {}
		primitiveType_t primitiveType;
		shadingType_t shadingType;
		uint32_t rectSize;
		uint32_t size_y_x;
		uint8_t verticesNumber;
		bool bPolyline;
		bool bTextured;
		transpanencyType_t transpanencyType;
		modulationType_t modulationType;
		uint32_t commandColor1 = 0;
		vertex_t vert1 = 0;
		texcoordType_t texCoord1Palette = 0;
		psxColor24_t color2 = 0;
		vertex_t vert2 = 0;
		texcoordType_t texCoord2TexPage = 0;
		psxColor24_t color3 = 0;
		vertex_t vert3 = 0;
		texcoordType_t texCoord3 = 0;
		psxColor24_t color4 = 0;
		vertex_t vert4 = 0;
		texcoordType_t texCoord4 = 0;
		bool bFlipHorizontally = false;
		bool bFlipVertically = false;
	};

	param_t param;

	struct gp0Commands_t {
		std::string name;
		uint8_t collectCount;
	};
	std::vector <gp0Commands_t> gp0Lookup;

	uint32_t gp0Command;
	uint32_t collectCount = 0;

	void decodeExecuteGp0Command(const uint32_t& command);

	void calculateTransparency(const vertex_t& p, psxColor16_t& frontPixelColor);

	void fillRectVram(const param_t& param);

	void drawPolygon(const param_t& param);

	void drawTexPolygon(const param_t& p_param);

	void drawLine(const param_t& param);
	bool bCollectPolylineParam = false;

	void drawMonochromeRect(const param_t& param);
	void drawTexRect15Bit(const param_t& param);
	void drawTexRect8Bit(const param_t& param);
	void drawTexRect4Bit(const param_t& param);

	void copyRectVramVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight);
	void copyRectCpuVram(const uint32_t& dest_coord, const uint32_t& width_height);
	void copyRectVramCpu(const uint32_t& source_coord, const uint32_t& width_height);

	bool bCollectCopyCpuVramParam = false;
	uint32_t widthHeight;
	uint32_t destCoord;
	uint32_t sourceCoord;

	int16_t ditheringLookup[4 * 4] = {
		-4, +0, -3, +1,
		+2, -2, +3, -1,
		-3, +1, -4, +0,
		+3, -1, +2, -2
	};

	void decodeExecuteGp1Command(const uint32_t& command);

	uint32_t gp1Command;

	uint32_t gp1Result = 0;
	uint32_t gp1TextureWindowSetting = 0;
	uint32_t gp1TopLeft = 0;
	uint32_t gp1BottomRight = 0;
	uint32_t gp1DrawOffset = 0;

	uint8_t* vram = new uint8_t[size_x_b * size_y];

	void writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea = false, const bool bDithering = false);
	void readVram(const vertex_t& position, uint16_t& data, const bool bIgnoreDrawingArea = false);

	olc::Sprite* getVramSprite();
	olc::Sprite vramView = olc::Sprite(1024, 512);
	olc::Sprite* pVramViewBuffered = new olc::Sprite(1024, 512);
	void updateVramView(const vertex_t& possition, const psxColor16_t& color16);

	bool bWasDMAActive = false;

	struct clutCache_t {
		bool bIsValid = false;
		union clut_t {
			uint16_t data;
			struct {
				uint16_t x : 6;
				uint16_t y : 9;
				uint16_t : 1;
			}elem;
			struct {
				uint16_t x : 6;
				uint16_t y : 10;
			}elem_v2;
		}clut;
		uint16_t cacheData[256] = {};

		clutCache_t() : bIsValid(false), clut(0) {}

		void rebuildCache(gpu* pGpu, const uint16_t& newClut) {

			if (clut.data != newClut)
				bIsValid = false;

			if (!bIsValid) {
				clut.data = newClut;
				for (int i = 0; i < 256; i++) {
					vertex_t entry((clut.elem.x << 4) + i, clut.elem.y);
					pGpu->readVram(entry, cacheData[i], true);
				}
				bIsValid = true;
			}
		}

		void invalidate() {
			bIsValid = false;
		}

		uint16_t getColor(int i) {
			return cacheData[i];
		}
	}clutCache;

	//debug
	debugUtilities* p_debugger;
};