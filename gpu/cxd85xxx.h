#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <thread>
#include <algorithm>

#include "../gpu/utils.h"
#include "../defines/defines.h"
#include "../debug_utilities/debug_utilities.h"
#include "../dependencies/olcPixelGameEngine.h"

#define size_x_b 2048
#define size_x 1024 //in half words
#define size_y 512

class busInterface_t;

class gpu_t {
public:

	gpu_t();

	busInterface_t* pBus;

	void readGpu32(const uint32_t& addr, uint32_t& data);
	void writeGpu32(const uint32_t& addr, const uint32_t& data);

	void videoClock();

	const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
	const uint32_t CPU_CLOCKS_PER_FRAME = ((float)CPU_CLOCK / 59.29f);
	const uint32_t CPU_CLOCKS_PER_SCANLINE = CPU_CLOCKS_PER_FRAME / 263;
	const uint32_t SCANLINE_NUMBER_NTSC = 263;
	const uint32_t SCANLINE_NUMBER_PAL = 314;

	uint32_t cpuClocks = 0;
	uint32_t cpuFrameClocks = 0;
	uint32_t cpuScanlineClocks = 0;
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
			uint32_t dataRequest : 1;
			uint32_t readyCommand : 1;
			uint32_t readyVramCpu : 1;
			uint32_t readyDma : 1;
			uint32_t dmaDir : 2;
			uint32_t drawingEvenOddLines : 1;
		}elem;
	}gpuStat;

	typedef struct {
		uint8_t x;
		uint8_t y;
	}texWinOffset_t, texWinMask_t, Coord;

	texWinMask_t texWinMask;
	texWinOffset_t texWinOffset;
	Coord display_start;

	typedef struct {
		uint16_t x;
		uint16_t y;
	}drawArea_t;

	drawArea_t drawAreaTopLeft;
	drawArea_t drawAreaBottomRight;

	typedef struct {
		int16_t x;
		int16_t y;
	}drawOffset_t;

	drawOffset_t drawOffset;

	uint16_t x1;
	uint16_t x2;
	uint16_t y1;
	uint16_t y2;

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

	union psxColor24_t {
		psxColor24_t() : data(0) {
		}
		psxColor24_t(uint32_t input) : data(input << 8 >> 8) {
		}
		uint32_t data;
		struct {
			uint32_t red : 8;
			uint32_t green : 8;
			uint32_t blue : 8;
		}component;
	};

	uint16_t format24to16Color(const uint32_t& color_24);
	uint32_t format16to24Color(const uint16_t& color_16);

	struct vertex_t {
		vertex_t() {
		}
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

	template <typename T>
	struct fifo_t { //have some bugs and unintuitives
		fifo_t() {
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

	fifo_t<uint32_t> fifo;

	enum primitiveType_t {
		POLYGON,
		RECT,
		LINE,
		FILL,
		MEMORY_TRANSFER,
		OTHER
	};

	union texcoordType_t {
		texcoordType_t() {
		}
		texcoordType_t(uint32_t data) : data(data) {
		}
		uint32_t data;
		struct {
			uint32_t x : 8;
			uint32_t y : 8;
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
		param_t() {
		}
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

	union clut_t {
		clut_t(uint16_t data) : data(data) {
		}
		uint16_t data;
		struct {
			uint16_t xCoord : 6;
			uint16_t yCoord : 9;
			uint16_t : 1;
		}elem;
	};


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

	enum triangleOrientation_t {
		PSX_CLOCKWISE,
		PSX_ANTI_CLOCKWISE
	};

	void drawPolygon(param_t* p_param);

	void drawTexPolygon(param_t* p_param);

	void drawLine(const param_t& param);
	bool bCollectPolylineParam = false;

	void drawMonochromeRect(const param_t& param);
	void drawTexRect15Bit(const param_t& param);
	void drawTexRect8Bit(const param_t& param);
	void drawTexRect4Bit(const param_t& param);

	void copyRectVramVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight);
	void copyRectCpuVram(const uint32_t& dest_coord, const uint32_t& width_height);
	void copyRectVramCpu(const uint32_t& source_coord, const uint32_t& width_height);
	fifo_t<uint32_t> outputFifo;

	bool bCollectCopyCpuVramParam = false;
	uint32_t widthHeight;
	uint32_t destCoord;
	uint32_t sourceCoord;

	void decodeExecuteGp1Command(const uint32_t& command);

	uint32_t gp1Command;

	uint32_t gp1Result = 0;
	uint32_t gp1TextureWindowSetting = 0;
	uint32_t gp1TopLeft = 0;
	uint32_t gp1BottomRight = 0;
	uint32_t gp1DrawOffset = 0;

	uint8_t* vram = new uint8_t[size_x_b * size_y];

	void writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea = false);
	void readVram(const vertex_t& position, uint16_t& data, const bool bIgnoreDrawingArea = false);

	olc::Sprite* getVramSprite();
	olc::Sprite vramView = olc::Sprite(1024, 512);
	void updateVramView(const vertex_t& possition, const psxColor16_t& color16);

	uint32_t fifoCount = 0;
	uint32_t fifoClocks = 0;

	//debug
	debugUtilities* p_debugger;
};