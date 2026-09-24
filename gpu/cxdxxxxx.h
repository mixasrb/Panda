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

inline uint16_t Signed11bit(uint16_t val) {
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

	void Resize(uint32_t newSize) {
		ptr = new T[newSize];
	}

	void Reset() {
		currentPossitionWrite = 0;
		currentPossitionRead = 0;
	}

	void PushBack(T data) {
		ptr[currentPossitionWrite] = data;
		currentPossitionWrite++;
	}

	void PopBack() {
		currentPossitionWrite--;
	}

	uint32_t GetSize() {
		return currentPossitionWrite;
	}

	bool Empty() {
		return (currentPossitionRead >= currentPossitionWrite) ? true : false;
	}

	T GetNext() {
		return ptr[currentPossitionRead++];
	}

	T operator[](uint32_t i) {
		return ptr[i];
	}
};

class BusInterface;

class Gpu {
public:

	Gpu();

	BusInterface* Bus;

	void ReadGpu32(const uint32_t& addr, uint32_t& data);
	void WriteGpu32(const uint32_t& addr, const uint32_t& data);

	void VideoClock();

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

	union GpuStat {
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
	}TexWinOffset, TexWinMask;

	TexWinMask texWinMask;
	TexWinOffset texWinOffset;

	typedef struct {
		uint16_t x;
		uint16_t y;
	}DrawArea, Coord;

	DrawArea drawAreaTopLeft;
	DrawArea drawAreaBottomRight;
	Coord displayStart;

	uint16_t screenX1;
	uint16_t screenX2;
	uint16_t screenY1;
	uint16_t screenY2;

	union PsxColor16 {
		PsxColor16() : data(0) {}
		PsxColor16(uint16_t input) : data(input) {}
		uint16_t data;
		struct {
			uint16_t red : 5;
			uint16_t green : 5;
			uint16_t blue : 5;
			uint16_t semiTransparencyFlag : 1;
		}component;
		PsxColor16 operator* (float a) {
			PsxColor16 v;
			v.component.red = (uint16_t)(a * (float)component.red);
			v.component.green = (uint16_t)(a * (float)component.green);
			v.component.blue = (uint16_t)(a * (float)component.blue);
			return v;
		}
		PsxColor16 operator/ (float a) {
			PsxColor16 v;
			v.component.red = (uint16_t)(a / (float)component.red);
			v.component.green = (uint16_t)(a / (float)component.green);
			v.component.blue = (uint16_t)(a / (float)component.blue);
			return v;
		}
		PsxColor16 operator+ (PsxColor16 v1) {
			PsxColor16 v2;
			v2.component.red = v1.component.red + component.red;
			v2.component.green = v1.component.green + component.green;
			v2.component.blue = v1.component.blue + component.blue;
			return v2;
		}
	};

	union PsxColor24 {
		PsxColor24() : data(0) {}
		PsxColor24(uint32_t input) : data(input << 8 >> 8) {}
		uint32_t data;
		struct {
			uint32_t red : 8;
			uint32_t green : 8;
			uint32_t blue : 8;
		}component;
	};

	uint16_t format24to16Color(const uint32_t& input) {
		PsxColor24 color24 = (PsxColor24)(input & 0x00ffffff);
		PsxColor16 color16 = 0;
		color16.component.red = color24.component.red >> 3;
		color16.component.green = color24.component.green >> 3;
		color16.component.blue = color24.component.blue >> 3;
		return color16.data;
	}

	uint32_t format16to24Color(const uint16_t& input) {
		PsxColor24 color24 = 0;
		PsxColor16 color16 = (PsxColor16)(input & 0x7fff);
		color24.component.red = color16.component.red << 3;
		color24.component.green = color16.component.green << 3;
		color24.component.blue = color16.component.blue << 3;
		return color24.data;
	}

	struct Vector2D {
		Vector2D() : x(0), y(0) {}
		Vector2D(uint32_t xTemp, uint32_t yTemp) {
			*(&x) = uint16_t(xTemp);
			*(&y) = uint16_t(yTemp);
		}
		Vector2D(uint32_t coord) {
			*(&x) = Signed11bit(coord & 0x7ff);
			*(&y) = Signed11bit((coord >> 16) & 0x7ff);
		}

		int16_t x;
		int16_t y;

		Vector2D operator+(Vector2D a) {
			return Vector2D(x + a.x, y + a.y);
		}
		Vector2D operator+=(Vector2D a) {
			return Vector2D(x + a.x, y + a.y);
		}
		Vector2D operator-(Vector2D a) {
			return Vector2D(x - a.x, y - a.y);
		}
		Vector2D operator*(int c) {
			return Vector2D(c * x, c * y);
		}
	};

	Vector2D drawOffset;

	Fifo<uint32_t> fifo;
	Fifo<uint32_t> outputFifo;

	enum PrimitiveType {
		UNDEFINED,
		POLYGON,
		RECT,
		LINE,
		FILL,
		MEMORY_TRANSFER,
		OTHER
	};

	union TexCoord {
		TexCoord() : data(0) {}
		TexCoord(uint32_t data) : data(data) {}
		uint32_t data;
		struct {
			uint32_t u : 8;
			uint32_t v : 8;
			uint32_t attribute : 16;
		}elem;
	};

	enum ShadingType {
		UNDEFINED_SHADING,
		FLAT_SHADING,
		GOURAUD_SHADING
	};

	enum TranspanencyType {
		UNDEFINED_TRANSPARENCY,
		SEMI_TRANSPARENT,
		NOT_TRANSPARENT
	};

	enum ModulationType {
		UNDEFINED_MODULATION,
		RAW_TEXTURE,
		TEXTURE_BLENDING
	};

	struct Param {
		Param() : primitiveType(UNDEFINED), shadingType(UNDEFINED_SHADING),
			rectSize(0), sizeYX(0), verticesNumber(0), isPolyline(false), isTextured(false),
			transpanencyType(UNDEFINED_TRANSPARENCY), modulationType(UNDEFINED_MODULATION),
			commandColor1(0), vert1(0, 0), texCoord1Palette(0),
			color2(0), vert2(0, 0), texCoord2TexPage(0),
			color3(0), vert3(0, 0), texCoord3(0),
			color4(0), vert4(0, 0), texCoord4(0) {}

		PrimitiveType primitiveType;
		ShadingType shadingType;
		uint32_t rectSize;
		uint32_t sizeYX;
		uint8_t verticesNumber;
		bool isPolyline;
		bool isTextured;
		TranspanencyType transpanencyType;
		ModulationType modulationType;
		uint32_t commandColor1;
		Vector2D vert1;
		TexCoord texCoord1Palette;
		PsxColor24 color2;
		Vector2D vert2;
		TexCoord texCoord2TexPage;
		PsxColor24 color3;
		Vector2D vert3;
		TexCoord texCoord3;
		PsxColor24 color4;
		Vector2D vert4;
		TexCoord texCoord4;
	};

	struct GP0Commands {
		std::string name;
		uint8_t collectPramCount;
	};
	std::vector <GP0Commands> gp0Lookup;

	uint32_t gp0Command;
	uint32_t collectCount = 0;

	void DecodeExecuteGp0Command(const uint32_t& command);

	void CalculateTransparency(const Vector2D& p, PsxColor16& frontPixelColor);

	void FillRectVram(const Param& param);

	void DrawPolygon(const Param& param);

	void DrawTexPolygon(const Param& p_param);

	void DrawLine(const Param& param);
	bool collectPolylineParam = false;

	void DrawMonochromeRect(const Param& param);
	void DrawTexRect15(const Param& param);
	void DrawTexRect8(const Param& param);
	void DrawTexRect4Bit(const Param& param);

	void CopyRectVramToVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight);
	void CopyRectCpuToVram(const uint32_t& dest_coord, const uint32_t& width_height);
	void CopyRectVramToCpu(const uint32_t& source_coord, const uint32_t& width_height);

	bool collectCopyCpuToVramData = false;

	uint32_t widthHeight;
	uint32_t destCoord;
	uint32_t sourceCoord;

	int16_t ditheringLookup[4 * 4] = {
		-4, +0, -3, +1,
		+2, -2, +3, -1,
		-3, +1, -4, +0,
		+3, -1, +2, -2
	};

	void DecodeExecuteGp1Command(const uint32_t& command);

	uint32_t gp1Command;

	uint32_t gp1Result = 0;
	uint32_t gp1TextureWindowSetting = 0;
	uint32_t gp1TopLeft = 0;
	uint32_t gp1BottomRight = 0;
	uint32_t gp1DrawOffset = 0;

	uint8_t* vram = new uint8_t[size_x_b * size_y];

	void WriteVram(const Vector2D& position, const uint16_t& data, const bool bIgnoreDrawingArea = false, const bool bDithering = false);
	void ReadVram(const Vector2D& position, uint16_t& data, const bool bIgnoreDrawingArea = false);

	olc::Sprite* GetVramSprite();
	olc::Sprite vramView = olc::Sprite(1024, 512);
	olc::Sprite* vramViewPtr = new olc::Sprite(1024, 512);
	void UpdateVramView(const Vector2D& position, const PsxColor16& color16);

	struct ClutCache {
		bool isValid = false;
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
			}elemV2;
		}clut;
		uint16_t cacheData[256] = {};

		ClutCache() : isValid(false), clut(0) {}

		void RebuildCache(Gpu* gpuPtr, const uint16_t& newClut) {

			if (clut.data != newClut)
				isValid = false;

			if (!isValid) {
				clut.data = newClut;
				for (int i = 0; i < 256; i++) {
					Vector2D entry((clut.elem.x << 4) + i, clut.elem.y);
					gpuPtr->ReadVram(entry, cacheData[i], true);
				}
				isValid = true;
			}
		}

		void Invalidate() {
			isValid = false;
		}

		uint16_t GetColor(int i) {
			return cacheData[i];
		}
	}clutCache;

	//debug
	debugUtilities* p_debugger;
};