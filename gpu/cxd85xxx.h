#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <thread>

#include "../gpu/misc.hpp"
#include "../defines/defines.h"
#include "../debug_utilities/debug_utilities.h"
#include "../dependencies/olcPixelGameEngine.h"

#define size_x 2048
#define size_x_hw 1024
#define size_y 512

class busInterface;

class cxd85xxx {
public:

	cxd85xxx();

	busInterface* pBus;

	void videoClock();
	uint32_t cpuFrameClocks = 0;
	uint32_t cpuScanlineClocks = 0;
	uint32_t scanline = 0;

	void readGpu32(const uint32_t& addr, uint32_t& data);
	void writeGpu32(const uint32_t& addr, const uint32_t& data);

	union gpuStat_t {
		uint32_t data;
		struct {
			uint32_t tex_page_x_base : 4;
			uint32_t tex_page_y_base : 1;
			uint32_t semi_transparency : 2;
			uint32_t tex_page_colors : 2;
			uint32_t dither_24_15 : 1;
			uint32_t drawing_to_display_area : 1;
			uint32_t set_mask_bit_while_drawing : 1;
			uint32_t draw_pixels : 1;
			uint32_t interlace_field : 1;
			uint32_t reverseflag : 1;
			uint32_t tex_dis : 1;
			uint32_t horizontal_res_2 : 1;
			uint32_t horizontal_res_1 : 2;
			uint32_t vertical_res : 1;
			uint32_t video_mode : 1;
			uint32_t display_area_color_depth : 1;
			uint32_t vertical_interlace : 1;
			uint32_t display_enable : 1;
			uint32_t irq1 : 1;
			uint32_t data_request : 1;
			uint32_t ready_cmd : 1;
			uint32_t ready_vram_cpu : 1;
			uint32_t ready_dma : 1;
			uint32_t dma_dir : 2;
			uint32_t drawing_even_odd_lines : 1;
		}elem;
	}gpuStat;

	struct gp0Commands {
		void (cxd85xxx::* command)(const uint32_t& commandColor);
		std::string name;
		uint8_t collectCount;
	};
	std::vector <gp0Commands> gp0Lookup;

	void nop(const uint32_t& commandColor);
	void unknown(const uint32_t& commandColor);

	void clearCache(const uint32_t& commandColor);
	void fillRectVram(const uint32_t& commandColor);
	void copyRectVramVram(const uint32_t& commandColor);
	void copyRectCpuVram(const uint32_t& commandColor);
	void copyRectVramCpu(const uint32_t& commandColor);

	void interruptRequest(const uint32_t& commandColor);

	void mono3PolyOpaq(const uint32_t& commandColor);
	void mono3PolySemiTransp(const uint32_t& commandColor);

	void mono4PolyOpaq(const uint32_t& commandColor);
	void mono4PolySemiTransp(const uint32_t& commandColor);

	void tex3PolyOpaqTexBlend(const uint32_t& commandColor);
	void tex3PolyOpaqRawTex(const uint32_t& commandColor);
	void tex3PolySemiTranspTexBlend(const uint32_t& commandColor);
	void tex3PolySemiTranspRawTex(const uint32_t& commandColor);

	void tex4PolyOpaqTexBlend(const uint32_t& commandColor);
	void tex4PolyOpaqRawTex(const uint32_t& commandColor);
	void tex4PolySemiTranspTexBlend(const uint32_t& commandColor);
	void tex4PolySemiTranspRawTex(const uint32_t& commandColor);

	void shaded3PolyOpaq(const uint32_t& commandColor);
	void shaded3PolySemiTransp(const uint32_t& commandColor);

	void shaded4PolyOpaq(const uint32_t& commandColor);
	void shaded4PolySemiTransp(const uint32_t& commandColor);

	void shadedTex3PolyOpaqTexBlend(const uint32_t& commandColor);
	void shadedTex3PolySemiTranspTexBlend(const uint32_t& commandColor);

	void shadedTex4PolyOpaqTexBlend(const uint32_t& commandColor);
	void shadedTex4PolySemiTranspTexBlend(const uint32_t& commandColor);

	void undocumented(const uint32_t& commandColor);

	void monoLineOpaq(const uint32_t& commandColor);
	void monoLineSemiTransp(const uint32_t& commandColor);

	void monoPolyLineOpaq(const uint32_t& commandColor);
	void monoPolyLineSemiTransp(const uint32_t& commandColor);

	void shadedLineOpaq(const uint32_t& commandColor);
	void shadedLineSemiTransp(const uint32_t& commandColor);

	void shadedPolyLineOpaq(const uint32_t& commandColor);
	void shadedPolyLineSemiTransp(const uint32_t& commandColor);

	void monoRectVarOpaq(const uint32_t& commandColor);
	void monoRectVarSemiTransp(const uint32_t& commandColor);

	void monoRect1x1Opaq(const uint32_t& commandColor);
	void monoRect1x1SemiTransp(const uint32_t& commandColor);

	void monoRect8x8Opaq(const uint32_t& commandColor);
	void monoRect8x8SemiTransp(const uint32_t& commandColor);

	void monoRect16x16Opaq(const uint32_t& commandColor);
	void monoRect16x16SemiTransp(const uint32_t& commandColor);

	void texRectVarOpaqTexBlend(const uint32_t& commandColor);
	void texRectVarOpaqRawTex(const uint32_t& commandColor);
	void texRectVarSemiTranspTexBlend(const uint32_t& commandColor);
	void texRectVarSemiTranspRawTex(const uint32_t& commandColor);

	void texRect1x1OpaqTexBlend(const uint32_t& commandColor);
	void texRect1x1OpaqRawTex(const uint32_t& commandColor);
	void texRect1x1SemiTranspTexBlend(const uint32_t& commandColor);
	void texRect1x1SemiTranspRawTex(const uint32_t& commandColor);

	void texRect8x8OpaqTexBlend(const uint32_t& commandColor);
	void texRect8x8OpaqRawTex(const uint32_t& commandColor);
	void texRect8x8SemiTranspTexBlend(const uint32_t& commandColor);
	void texRect8x8SemiTranspRawTex(const uint32_t& commandColor);

	void texRect16x16OpaqTexBlend(const uint32_t& commandColor);
	void texRect16x16OpaqRawTex(const uint32_t& commandColor);
	void texRect16x16SemiTranspTexBlend(const uint32_t& commandColor);
	void texRect16x16SemiTranspRawTex(const uint32_t& commandColor);

	void drawModeSetting(const uint32_t& commandColor);
	void texWindowSetting(const uint32_t& commandColor);
	void setDrawAreaTopLeft(const uint32_t& commandColor);
	void setDrawAreaBottomRight(const uint32_t& commandColor);
	void setDrawOffset(const uint32_t& commandColor);
	void maskBitSetting(const uint32_t& commandColor);

	typedef struct {
		uint8_t x;
		uint8_t y;
	}texWinOffset_t, texWinMask_t, Coord;

	texWinMask_t tex_win_mask;
	texWinOffset_t tex_win_offset;
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

	std::vector<uint32_t> fifo;

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
		}component;
		psxColor16_t operator* (float a) {
			psxColor16_t v;
			v.component.red = (uint16_t)(a * (float)component.red);
			v.component.green = (uint16_t)(a * (float)component.green);
			v.component.blue = (uint16_t)(a * (float)component.blue);
			return v;
		}
		psxColor16_t operator* (psxColor16_t v1) {
			psxColor16_t v2;
			v2.component.red = v1.component.red + component.red;
			v2.component.green = v1.component.green + component.green;
			v2.component.blue = v1.component.blue + component.blue;
			return v2;
		}
		psxColor16_t operator+ (psxColor16_t v1) {
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

	uint32_t gp1;

	uint32_t command;
	uint32_t collect = 0;
	std::vector<uint32_t> collect_list;

	struct vertex_t
	{
		vertex_t() {
		}
		vertex_t(uint32_t xt, uint32_t yt) {
			*(&x) = uint16_t(xt);
			*(&y) = uint16_t(yt);
		}
		vertex_t(uint32_t coord) {
			*(&x) = uint16_t(coord << 16 >> 16);
			*(&y) = uint16_t(coord >> 16);
		}

		int16_t x;
		int16_t y;

		vertex_t operator+(vertex_t a) {
			return vertex_t(x + a.x, y + a.y);
		}
		vertex_t operator-(vertex_t a) {
			return vertex_t(x - a.x, y - a.y);
		}
	};

	enum type_t {
		TRIANGLE_PARAM,
		RECT_PARAM,
		SHADED_3_POINT_PARAM,
		SHADED_4_POINT_PARAM,
		TEXTURED_3_POINT_OPAQUE_BLENDING_PARAM,
		TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM,
		TEXTURED_4_POINT_OPAQUE_RAW_PARAM,
		TEXTURED_RECT_OPAQUE_RAW_PARAM,
		MONOCHROME_LINE_PARAM
	};

	union texcoordData_t {
		texcoordData_t() {
		}
		texcoordData_t(uint32_t data) : data(data) {
		}
		uint32_t data;
		struct {
			uint32_t x : 8;
			uint32_t y : 8;
			uint32_t attribute : 16;
		}elem;
	};

	struct param_t {
		param_t() {
		}
		type_t type;
		uint32_t command_color1;
		vertex_t vert1;
		texcoordData_t texCoord1Palette;
		psxColor24_t color2;
		vertex_t vert2;
		texcoordData_t texCoord2TexPage;
		psxColor24_t color3;
		vertex_t vert3;
		texcoordData_t texCoord3;
		psxColor24_t color4;
		vertex_t vert4;
		texcoordData_t texCoord4;
	};

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

	void drawMonochromeRect(const param_t param);
	void drawMonochromeLine(param_t param);
	void drawTexRect15Bit(param_t param);
	void drawTexRect8Bit(param_t param);
	void drawTexRect4Bit(param_t param);

	void copyRectCpuVram(const uint32_t dest_coord, const uint32_t width_height);
	bool b_collect_param = false;
	uint32_t width_height;
	uint32_t dest_coord;
	uint32_t source_coord;
	void copyRectVramCpu(const uint32_t source_coord, const uint32_t width_height);
	bool b_trans_command = false;

	uint8_t* vram = new uint8_t[size_x * size_y];

	void writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea = false);
	void readVram(const vertex_t& position, uint16_t& data);

	float edgeFunc(vertex_t v1, vertex_t v2, vertex_t p);

	enum RasterizationModes {
		PSX_MONOCHROME,
		PSX_SHADED,
		PSX_OPAQUE,
		PSX_SEMI_TRANSPARENT
	};

	void CheckDrawingOffset(vertex_t& v1, vertex_t& v2, vertex_t& v3, vertex_t& v4);
	void rasterization(param_t* p_param, const RasterizationModes& color_mode, const RasterizationModes& transparency_mode,
		const uint8_t& vertex_count, vertex_t v1, vertex_t v2, vertex_t v3, vertex_t v4 = (0, 0));

	olc::Sprite* getVram();
	olc::Sprite vramView = olc::Sprite(1024, 512);
	void updateVramView(const vertex_t& possition, const psxColor16_t& color16);


	bool b_gp1Result = false;
	uint32_t gp1Result = 0;
	uint32_t gp1_top_left = 0;
	uint32_t gp1_bottom_right = 0;
	uint32_t gp1_draw_offset = 0;

	//debug
	debugUtilities* p_debugger;
};