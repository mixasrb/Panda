#include <vector>

#include "cxdxxxxx.h"
#include "../bus/bus_interface.h"

//TO DO 
//Define option for old and new GPU 

extern bool g_emulationPaused;
extern bool g_GpuCommandsLogged;
extern int g_mousePossitionX;
extern int g_mousePossitionY;
extern bool g_logPixel;
extern bool g_manualInterruptDMA;

Gpu::Gpu() {

	fifo.Resize(size_x * size_y);

	outputFifo.Resize(size_x * size_y);

	gp0Lookup.resize(0xef);
	gp0Lookup = {
		//0x00
		{"NOP (?)", 1},
		{"Clear Cache", 1},
		{"Fill Rectangle in VRAM", 3},
		{"Unknown?", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		//0x10
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Mirror of NOP (?)", 1},
		{"Interrupt Request (IRQ1)", 1},
		//0x20
		{"Monochrome three-point polygon, opaque", 4},
		{"Undocumented/Nonsense", 4},
		{"Monochrome three - point polygon, semi - transparent", 4},
		{"Undocumented/Nonsense", 4},
		{"Textured three-point polygon, opaque, texture-blending", 7},
		{"Textured three-point polygon, opaque, raw-texture", 7},
		{"Textured three-point polygon, semi-transparent, texture-blending", 7},
		{"Textured three-point polygon, semi-transparent, raw-texture", 7},
		{"Monochrome four-point polygon, opaque", 5},
		{"Undocumented/Nonsense", 5},
		{"Monochrome four-point polygon, semi-transparent", 5},
		{"Undocumented/Nonsense", 5},
		{"Textured four-point polygon, opaque, texture-blending", 9},
		{"Textured four-point polygon, opaque, raw-texture", 9},
		{"Textured four-point polygon, semi-transparent, texture-blending", 9},
		{"Textured four-point polygon, semi-transparent, raw-texture", 9},
		//0x30
		{"Shaded three-point polygon, opaque", 6},
		{"Undocumented/Nonsense", 6},
		{"Shaded three-point polygon, semi-transparent", 6},
		{"Undocumented/Nonsense", 6},
		{"Shaded Textured three-point polygon, opaque, texture-blending", 9},
		{"Undocumented/Nonsense", 9},
		{"Shaded Textured three-point polygon, semi-transparent, tex-blend", 9},
		{"Undocumented/Nonsense", 9},
		{"Shaded four-point polygon, opaque", 8},
		{"Undocumented/Nonsense", 8},
		{"Shaded four-point polygon, semi-transparent", 8},
		{"Undocumented/Nonsense", 8},
		{"Shaded Textured four-point polygon, opaque, texture-blending", 12},
		{"Undocumented/Nonsense", 12},
		{"Shaded Textured four-point polygon, semi-transparent, tex-blend", 12},
		{"Undocumented/Nonsense", 12},
		//0x40
		{"Monochrome line, opaque", 3},
		{"Monochrome line, opaque", 3}, //???
		{"Monochrome line, semi-transparent", 3},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Monochrome Poly-line, opaque", 3},
		{"Undocumented/Nonsense", 1},
		{"Monochrome Poly-line, semi-transparent", 3},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 3},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 3},
		{"Undocumented/Nonsense", 1},
		//0x50
		{"Shaded line, opaque", 4},
		{"Undocumented/Nonsense", 4},
		{"Shaded line, semi-transparent", 4},
		{"Undocumented/Nonsense", 4},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 1},
		{"Shaded Poly-line, opaque", 4},
		{"Undocumented/Nonsense", 1},
		{"Shaded Poly-line, semi-transparent", 4},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 4},
		{"Undocumented/Nonsense", 1},
		{"Undocumented/Nonsense", 4},
		{"Undocumented/Nonsense", 1},
		//0x60
		{"Monochrome Rectangle (variable size) (opaque)", 3},
		{"Undocumented/Nonsense", 3},
		{"Monochrome Rectangle (variable size) (semi-transparent)", 3},
		{"Undocumented/Nonsense", 3},
		{"Textured Rectangle, variable size, opaque, texture-blending", 4},
		{"Textured Rectangle, variable size, opaque, raw-texture", 4},
		{"Textured Rectangle, variable size, semi-transp, texture-blending", 4},
		{"Textured Rectangle, variable size, semi-transp, raw-texture", 4},
		{"Monochrome Rectangle (1x1) (Dot) (opaque)", 2},
		{"Undocumented/Nonsense", 2},
		{"Monochrome Rectangle (1x1) (Dot) (semi-transparent)", 2},
		{"Undocumented/Nonsense", 2},
		{"Textured Rectangle, 1x1 (nonsense), opaque, texture-blending", 3},
		{"Textured Rectangle, 1x1 (nonsense), opaque, raw-texture", 3},
		{"Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending", 3},
		{"Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture", 3},
		//0x70
		{"Monochrome Rectangle (8x8) (opaque)", 2},
		{"Undocumented/Nonsense", 2},
		{"Monochrome Rectangle (8x8) (semi-transparent)", 2},
		{"Undocumented/Nonsense", 2},
		{"Textured Rectangle, 8x8, opaque, texture-blending", 3},
		{"Textured Rectangle, 8x8, opaque, raw-texture", 3},
		{"Textured Rectangle, 8x8, semi-transparent, texture-blending", 3},
		{"Textured Rectangle, 8x8, semi-transparent, raw-texture", 3},
		{"Monochrome Rectangle (16x16) (opaque)", 2},
		{"Undocumented/Nonsense", 2},
		{"Monochrome Rectangle (16x16) (semi-transparent)", 2},
		{"Undocumented/Nonsense", 2},
		{"Textured Rectangle, 16x16, opaque, texture-blending", 3},
		{"Textured Rectangle, 16x16, opaque, raw-texture", 3},
		{"Textured Rectangle, 16x16, semi-transparent, texture-blending", 3},
		{"Textured Rectangle, 16x16, semi-transparent, raw-texture", 3},
		//0x80
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		//0x90
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		{"Copy Rectangle (VRAM to VRAM)", 4},
		//0xa0
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		//0xb0										  			   
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		{"Copy Rectangle (CPU to VRAM)", 3},
		//0xc0
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		//0xd0										   			   
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		{"Copy Rectangle (VRAM to CPU)", 3},
		//0xe0
		{"NOP (?)", 1},
		{"Draw Mode setting (aka 'Texpage')", 1},
		{"Texture Window setting", 1},
		{"Set Drawing Area top left (X1,Y1)", 1},
		{"Set Drawing Area bottom right (X2,Y2)", 1},
		{"Set Drawing Offset (X,Y)", 1},
		{"Mask Bit Setting", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
		{"NOP (?)", 1},
	};


	cpuClocks = 0;
	cpuFrameClocks = 0;
	cpuScanlineClocks = 0;
	cpuClocksToHblank = CPU_CLOCKS_PER_SCANLINE_NTSC;
	cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
	scanlineNumber = SCANLINE_NUMBER_NTSC;

	scanlineCount = 0;
	gpuStat.data = 0x14000000;
	fifo.Reset();
	outputFifo.Reset();
}

void Gpu::ReadGpu32(const uint32_t& addr, uint32_t& data) {
	data = 0;

	if (addr == 0x1f801810) {
		if (gpuStat.elem.readyVramCpu) {
			data = outputFifo.GetNext();
			if (outputFifo.Empty()) {
				gpuStat.elem.readyVramCpu = 0;
				outputFifo.Reset();
			}
			return;
		}

		data = gp1Result;
		return;
	}

	if (addr == 0x1f801814) {
		switch (gpuStat.elem.directionDMA) {
		case 0:
			gpuStat.elem.dataRequestDMA = 0;
			break;
		case 1:
			gpuStat.elem.dataRequestDMA = 1;
			break;
		case 2:
			gpuStat.elem.dataRequestDMA = gpuStat.elem.readyBlockDMA;
			break;
		case 3:
			gpuStat.elem.dataRequestDMA = gpuStat.elem.readyVramCpu;
			break;
		}
		data = gpuStat.data;
	}
}

void Gpu::WriteGpu32(const uint32_t& addr, const uint32_t& data) {
	if (addr == 0x1f801810) {
		if (fifoCollectCount) {
			fifo.PushBack(data);
			fifoCollectCount--;
			if ((fifoCollectCount == 0) && (collectPolylineParam))
				if ((data & 0xf000f000) == 0x50005000)
					collectPolylineParam = false;
				else
					fifoCollectCount = 1;
		}
		else {
			gp0Command = data;

			if (!fifo.Empty()) {
				std::cout << "[GPU] EMULATION PAUSED! fifo not empty 0x" << std::hex << fifo.GetSize() << '\n';
				g_emulationPaused = true;
			}

			fifo.Reset();

			fifoCollectCount = gp0Lookup[gp0Command >> 24].collectPramCount - 1;

			if (((gp0Command >> 24) >= 0xa0) && ((gp0Command >> 24) < 0xc0))
				collectCopyCpuToVramData = true;
			else if ((gp0Command >> 29 == 0b010) && (gp0Command & 0x08000000))
				collectPolylineParam = true;
		}

		if (fifoCollectCount == 0) {

			if (g_GpuCommandsLogged)
				std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << gp0Command
				<< " " << gp0Lookup[gp0Command >> 24].name
				<< " memAdrTemp: 0x" << '\n';

			DecodeExecuteGp0Command(gp0Command);
		}
		return;
	}

	if (addr == 0x1f801814) {
		gp1Command = data;

		if (g_GpuCommandsLogged)
			std::cout << "~[GPU] GPU1 command executed: 0x" << std::hex << gp1Command << '\n';

		DecodeExecuteGp1Command(gp1Command);
	}
}

void Gpu::VideoClock() {
	if (cpuScanlineClocks == cpuClocksToHblank) {
		//Hblank clocking timer 1
		BusPtr->timer0.bHblank = true;
		BusPtr->timer0.clock(_HBLANK_CLOCK);
		BusPtr->timer1.clock(_HBLANK_CLOCK);
	}

	if (cpuScanlineClocks == cpuScanlineClocksNumber) {

		cpuScanlineClocks = 0;
		scanlineCount++;
		BusPtr->timer0.bHblank = false;

		if (gpuStat.elem.verticalInterlace)
			if (!gpuStat.elem.verticalRes)
				gpuStat.elem.drawingEvenOddLines = (scanlineCount < screenY2) ? scanlineCount & 0x1 : 0;

		if (scanlineCount == screenY2) {
			BusPtr->timer1.bVblank = true;

			if (g_GpuCommandsLogged)
				std::cout << "~[GPU] GPU VBlank" << '\n';

			//V-Sync interrupt is naturally edge trigered
			BusPtr->pCpu->cp0.interruptHandler(_IRQ_VBLANK_);


			//Vblank synchronization timer 1
			BusPtr->timer1.clock(_VBLANK_CLOCK);
		}
		else if (scanlineCount == scanlineNumber) {

			scanlineCount = 0;
			BusPtr->timer1.bVblank = false;

			if (g_GpuCommandsLogged)
				std::cout << "~[GPU] GPU non VBlank" << '\n';

			if (gpuStat.elem.verticalInterlace)
				if (gpuStat.elem.verticalRes)
					gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;
		}
	}

	if ((cpuClocks % dotClockDevider) == 0)
		BusPtr->timer0.clock(_DOT_CLOCK);

	if (cpuClocks == CPU_CLOCK)
		cpuClocks = 0;
	else
		cpuClocks++;

	cpuScanlineClocks++;
}

void Gpu::DecodeExecuteGp0Command(const uint32_t& command) {

	auto maskPosFill = [](uint16_t& xpos, uint16_t& ypos) {
		xpos = xpos & 0x3f0;
		ypos = ypos & 0x1ff;
		};

	auto maskSizFill = [](uint16_t& xsiz, uint16_t& ysiz) {
		xsiz = ((xsiz & 0x3ff) + 0x0f) & (~0x0f);
		ysiz = ysiz & 0x1ff;
		};

	auto maskPosCopy = [](uint16_t& xpos, uint16_t& ypos) {
		xpos = xpos & 0x3ff;
		ypos = ypos & 0x1ff;
		};

	auto maskSizCopy = [](uint16_t& xsiz, uint16_t& ysiz) {
		xsiz = ((xsiz - 1) & 0x3ff) + 1;
		ysiz = ((ysiz - 1) & 0x1ff) + 1;
		};

	switch (command >> 29) {
	case 0b000: //Misc commands
		if ((gp0Command >> 24) == 0x02) {
			Param param;

			param.primitiveType = FILL;
			param.commandColor1 = gp0Command;

			uint32_t pos = fifo.GetNext();
			uint16_t xpos = pos & 0xffff;
			uint16_t ypos = pos >> 16;
			maskPosFill(xpos, ypos);
			param.vert1 = Vector2D(xpos, ypos);

			uint32_t size_x_y = fifo.GetNext();
			uint16_t xsiz = size_x_y & 0xffff;
			uint16_t ysiz = size_x_y >> 16;
			maskSizFill(xsiz, ysiz);
			param.vert4 = param.vert1 + Vector2D(xsiz, ysiz);

			FillRectVram(param);
		}
		else if ((gp0Command >> 24) == 0x01) {
			//clear texture cache unhandled??

			clutCache.Invalidate();
		}
		else {
			if (gp0Command >> 24 == 0x1f) {
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x"
					<< std::hex << command << " " << gp0Lookup[(command >> 24)].name << "\n";
				g_emulationPaused = true;
			}
		}
		break;
	case 0b001: {

		Param param;
		param.primitiveType = POLYGON;
		param.shadingType = (command & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.verticesNumber = (command & 0x08000000) ? 4 : 3;
		param.isTextured = (command & 0x04000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (command & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		param.commandColor1 = command;

		param.vert1 = fifo.GetNext();
		param.vert1 += drawOffset;

		if (param.isTextured)
			param.texCoord1Palette = fifo.GetNext();

		if (param.shadingType == GOURAUD_SHADING)
			param.color2 = fifo.GetNext();

		param.vert2 = fifo.GetNext();
		param.vert2 += drawOffset;

		if (param.isTextured)
			param.texCoord2TexPage = fifo.GetNext();

		if (param.shadingType == GOURAUD_SHADING)
			param.color3 = fifo.GetNext();

		param.vert3 = fifo.GetNext();
		param.vert3 += drawOffset;

		if (param.isTextured)
			param.texCoord3 = fifo.GetNext();

		param.vert4 = (0, 0);//Can it cause problems??
		if (param.verticesNumber == 4) {
			if (param.shadingType == GOURAUD_SHADING)
				param.color4 = fifo.GetNext();

			param.vert4 = fifo.GetNext();
			param.vert4 += drawOffset;

			if (param.isTextured)
				param.texCoord4 = fifo.GetNext();
		}

		if (param.isTextured) {
			gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
			gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
		}

		if (param.shadingType == FLAT_SHADING) {
			param.color2 = param.commandColor1 & 0x00ffffff;
			param.color3 = param.commandColor1 & 0x00ffffff;
			if (param.verticesNumber == 4)
				param.color4 = param.commandColor1 & 0x00ffffff;
		}

		// I am not 100% sure for this
		if ((abs(param.vert1.x - param.vert2.x) > 1024) ||
			(abs(param.vert1.x - param.vert3.x) > 1024) ||
			((param.verticesNumber == 4) && (abs(param.vert1.x - param.vert4.x) > 1024)) ||
			(abs(param.vert2.x - param.vert3.x) > 1024) ||
			((param.verticesNumber == 4) && (abs(param.vert2.x - param.vert4.x) > 1024)) ||
			((param.verticesNumber == 4) && (abs(param.vert3.x - param.vert4.x) > 1024)) ||
			(abs(param.vert1.y - param.vert2.y) > 512) ||
			(abs(param.vert1.y - param.vert3.y) > 512) ||
			((param.verticesNumber == 4) && (abs(param.vert1.y - param.vert4.y) > 512)) ||
			(abs(param.vert2.y - param.vert3.y) > 512) ||
			((param.verticesNumber == 4) && (abs(param.vert2.y - param.vert4.y) > 512)) ||
			((param.verticesNumber == 4) && (abs(param.vert3.y - param.vert4.y) > 512)))
			return;

		if (param.isTextured)
			DrawTexPolygon(param);
		else
			DrawPolygon(param);
	}

			  break;

	case 0b010: {

		Param param;
		param.primitiveType = LINE;
		param.shadingType = (command & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.isPolyline = (command & 0x08000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;

		if (param.isPolyline)
			fifo.PopBack(); // pop 0x50005000

		param.commandColor1 = command;
		if (param.shadingType == FLAT_SHADING) {
			for (int i = 0; i < fifo.GetSize() - 1; i++) {
				param.vert1 = fifo[i];
				param.vert1 += drawOffset;
				param.vert2 = fifo[i + 1];
				param.vert2 += drawOffset;

				DrawLine(param);
			}
		}
		else {
			for (int i = 0; i < fifo.GetSize() - 2; i += 2) {
				param.vert1 = fifo[i];
				param.vert1 += drawOffset;
				param.color2 = fifo[i + 1];
				param.vert2 = fifo[i + 2];
				param.vert2 += drawOffset;

				if (fifo[0] == fifo[i + 2]) //!!?????
					param.color2 = command & 0x00ffffff;

				DrawLine(param);

				param.commandColor1 = (param.commandColor1 & 0xff000000) | param.color2.data;

			}
		}

		fifo.Reset(); //??
	}

			  break;

	case 0b011: { //Rectangle primitive

		Param param;
		param.primitiveType = RECT;
		param.rectSize = command << 3 >> 30;
		param.isTextured = (command & 0x04000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (command & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		param.commandColor1 = gp0Command;

		param.vert1 = fifo.GetNext();
		param.vert1 += drawOffset;

		if (param.isTextured)
			param.texCoord1Palette = fifo.GetNext();

		switch (param.rectSize) {
		case 0b00:
			param.sizeYX = fifo.GetNext();
			break;
		case 0b01:
			param.sizeYX = 0x00010001;
			break;
		case 0b10:
			param.sizeYX = 0x00080008;
			break;
		case 0b11:
			param.sizeYX = 0x00100010;
			break;
		}

		if (param.isTextured)
			DrawTexRect(param);
		else
			DrawMonochromeRect(param);
		break;
	}
	case 0b100: { //VRAM-to-VRAM blit
		uint32_t sourceCoord = fifo.GetNext();
		uint16_t xpos = sourceCoord & 0xffff;
		uint16_t ypos = sourceCoord >> 16;
		maskPosCopy(xpos, ypos);
		sourceCoord = ((uint32_t)ypos << 16) | xpos;

		uint32_t destCoord = fifo.GetNext();
		xpos = destCoord & 0xffff;
		ypos = destCoord >> 16;
		maskPosCopy(xpos, ypos);
		destCoord = ((uint32_t)ypos << 16) | xpos;

		uint32_t widthHeight = fifo.GetNext();
		uint16_t xsiz = widthHeight & 0xffff;
		uint16_t ysiz = widthHeight >> 16;
		maskSizCopy(xsiz, ysiz);
		widthHeight = ((uint32_t)ysiz << 16) | xsiz;

		CopyRectVramToVram(sourceCoord, destCoord, widthHeight);
		break;
	}
	case 0b101: //CPU-to-VRAM blit
		if (collectCopyCpuToVramData) {
			collectCopyCpuToVramData = false;

			destCoord = fifo.GetNext();
			uint16_t xpos = destCoord & 0xffff;
			uint16_t ypos = destCoord >> 16;
			maskPosCopy(xpos, ypos);
			destCoord = ((uint32_t)ypos << 16) | xpos;

			widthHeight = fifo.GetNext();
			uint16_t xsiz = widthHeight & 0xffff;
			uint16_t ysiz = widthHeight >> 16;
			maskSizCopy(xsiz, ysiz);
			widthHeight = ((uint32_t)ysiz << 16) | xsiz;

			fifoCollectCount = xsiz * ysiz / 2;
			if ((fifoCollectCount * 2) < xsiz * ysiz)
				fifoCollectCount++;
		}
		else
			CopyRectCpuToVram(destCoord, widthHeight);

		break;

	case 0b110: { //VRAM-to-CPU blit
		sourceCoord = fifo.GetNext();
		uint16_t xpos = sourceCoord & 0xffff;
		uint16_t ypos = sourceCoord >> 16;
		maskPosCopy(xpos, ypos);
		sourceCoord = ((uint32_t)ypos << 16) | xpos;

		widthHeight = fifo.GetNext();
		uint16_t xsiz = widthHeight & 0xffff;
		uint16_t ysiz = widthHeight >> 16;
		maskSizCopy(xsiz, ysiz);
		widthHeight = ((uint32_t)ysiz << 16) | xsiz;

		CopyRectVramToCpu(sourceCoord, widthHeight);
		gpuStat.elem.readyVramCpu = 1;
		break;
	}
	case 0b111:

		switch (command >> 24) {
		case 0xe1:
			gpuStat.data &= 0xffff7800;
			gpuStat.data |= command & 0x000087ff;
			gpuStat.elem.texDis = command & 0x00008000 ? 1 : 0;
			break;

		case 0xe2:
			texWinMask.x = command << 27 >> 27;
			texWinMask.y = command << 22 >> 27;
			texWinOffset.x = command << 17 >> 27;
			texWinOffset.y = command << 12 >> 27;
			gp1TextureWindowSetting = command & 0x000fffff;
			break;

		case 0xe3:
			drawAreaTopLeft.x = command << 22 >> 22;
			drawAreaTopLeft.y = command << 13 >> 23;
			gp1TopLeft = command & 0x0007ffff;
			break;

		case 0xe4:
			drawAreaBottomRight.x = command << 22 >> 22;
			drawAreaBottomRight.y = command << 13 >> 23;
			gp1BottomRight = command & 0x0007ffff;
			break;

		case 0xe5:
			drawOffset.x = Signed11bit(command & 0x7ff);
			drawOffset.y = Signed11bit((command >> 11) & 0x7ff);
			gp1DrawOffset = command & 0x003fffff;
			break;

		case 0xe6:
			gpuStat.elem.setMaskBitWhileDrawing = command & 0x1;
			gpuStat.elem.drawPixels = (command & 0x2 ? 1 : 0);
			break;

		default:
			std::cout << "~[GPU] EMULATION PAUSED! Unhandled environment command 0x"
				<< std::hex << command << '\n';
			g_emulationPaused = true;
			break;

			break;
		}
	}
}

void Gpu::DecodeExecuteGp1Command(const uint32_t& command) {

	switch (command >> 24) {
	case 0x0:
		//clear fifo

		gpuStat.elem.irq1 = 0;

		gpuStat.elem.displayEnable = 0;

		gpuStat.elem.directionDMA = 0;

		displayStart.x = 0;
		displayStart.y = 0;

		screenX1 = 0x200;
		screenX2 = 0x200 + 256 * 10;

		screenY1 = 0x010;
		screenY2 = 0x010 + 240;

		gpuStat.elem.horizontalRes1 = 1;
		gpuStat.elem.verticalRes = 0;
		gpuStat.elem.videoMode = 0;
		gpuStat.elem.displayAreaColorDepth = 0;
		gpuStat.elem.verticalInterlace = 0;
		gpuStat.elem.horizontalRes2 = 0;
		gpuStat.elem.reverseflag = 0;

		//Bug while reseting maybe for PAL Psx's video mode is reseted to value 1??
		/*if (!gpuStat.elem.videoMode) { //Breaks Disney's Donald Duck - Quack Attack (Europe) (En,Fr,De,It)
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
			scanlineNumber = SCANLINE_NUMBER_NTSC;
		}
		else {
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_PAL;
			scanlineNumber = SCANLINE_NUMBER_PAL;
		}*/

		if (!gpuStat.elem.verticalInterlace)
			gpuStat.elem.interlaceField = 1;
		else
			gpuStat.elem.interlaceField = 0;

		if (gpuStat.elem.horizontalRes2) {
			horizontalResolution = 368;
			dotClockDevider = 7;
		}
		else
			switch (gpuStat.elem.horizontalRes1) {
			case 0:
				horizontalResolution = 256;
				dotClockDevider = 10;
				break;
			case 1:
				horizontalResolution = 320;
				dotClockDevider = 8;
				break;
			case 2:
				horizontalResolution = 512;
				dotClockDevider = 5;
				break;
			case 3:
				horizontalResolution = 640;
				dotClockDevider = 4;
				break;
			}

		gpuStat.data &= 0xffff7800;
		gpuStat.data |= 0 & 0x000087ff;
		gpuStat.elem.texDis = 0 & 0x00008000 ? 1 : 0;

		texWinMask.x = 0 << 27 >> 27;
		texWinMask.y = 0 << 22 >> 27;
		texWinOffset.x = 0 << 17 >> 27;
		texWinOffset.y = 0 << 12 >> 27;
		gp1TextureWindowSetting = 0 & 0x000fffff;

		drawAreaTopLeft.x = 0 << 22 >> 22;
		drawAreaTopLeft.y = 0 << 13 >> 23;
		gp1TopLeft = 0 & 0x0007ffff;

		drawAreaBottomRight.x = 0 << 22 >> 22;
		drawAreaBottomRight.y = 0 << 13 >> 23;
		gp1BottomRight = 0 & 0x0007ffff;

		drawOffset.x = Signed11bit(0 & 0x7ff);
		drawOffset.y = Signed11bit((0 >> 11) & 0x7ff);
		gp1DrawOffset = 0 & 0x003fffff;;

		gpuStat.elem.setMaskBitWhileDrawing = 0 & 0x1;
		gpuStat.elem.drawPixels = (0 & 0x2 ? 1 : 0);

		gpuStat.data = 0x14802000;
		break;
	case 0x1:
		std::cout << "~[GPU] Unhandled GPU1 command: Reset Command Buffer" << '\n';
		break;
	case 0x2:
		gpuStat.elem.irq1 = 0;
		break;
	case 0x3:
		gpuStat.elem.displayEnable = command & 0x1;
		break;
	case 0x4:
		gpuStat.elem.directionDMA = command & 0x3;
		if (gpuStat.elem.directionDMA == 1) {
			std::cout << "[GPU] EMULATION PAUSED! DMA Direction FIFO" << '\n';
			g_emulationPaused = true;
		}
		break;
	case 0x5:
		displayStart.x = command & 0x3ff;
		displayStart.y = (command >> 10) & 0x1ff;
		break;
	case 0x6:
		screenX1 = command & 0xfff;
		screenX2 = (command >> 12) & 0xfff;
		break;
	case 0x7:
		screenY1 = command << 22 >> 22;
		screenY2 = command << 12 >> 22;
		break;
	case 0x8:
		gpuStat.elem.horizontalRes1 = command & 0x3;
		gpuStat.elem.verticalRes = (command & 0x4 ? 1 : 0);
		gpuStat.elem.videoMode = (command & 0x8 ? 1 : 0);
		gpuStat.elem.displayAreaColorDepth = (command & 0x10 ? 1 : 0);
		gpuStat.elem.verticalInterlace = (command & 0x20 ? 1 : 0);
		gpuStat.elem.horizontalRes2 = (command & 0x40 ? 1 : 0);
		gpuStat.elem.reverseflag = (command & 0x80 ? 1 : 0);

		if (!gpuStat.elem.videoMode) {
			cpuClocksToHblank = CPU_CLOCKS_TO_HBLANK_NTSC;
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
			scanlineNumber = SCANLINE_NUMBER_NTSC;
		}
		else {
			cpuClocksToHblank = CPU_CLOCKS_TO_HBLANK_PAL;
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_PAL;
			scanlineNumber = SCANLINE_NUMBER_PAL;
		}

		if (!gpuStat.elem.verticalInterlace)
			gpuStat.elem.interlaceField = 1;
		else
			gpuStat.elem.interlaceField = 0;

		if (gpuStat.elem.horizontalRes2) {
			horizontalResolution = 368;
			dotClockDevider = 7;
		}
		else
			switch (gpuStat.elem.horizontalRes1) {
			case 0:
				horizontalResolution = 256;
				dotClockDevider = 10;
				break;
			case 1:
				horizontalResolution = 320;
				dotClockDevider = 8;
				break;
			case 2:
				horizontalResolution = 512;
				dotClockDevider = 5;
				break;
			case 3:
				horizontalResolution = 640;
				dotClockDevider = 4;
				break;
			}

		break;
	case 0x10:
		switch (command & 0x7) {
		case 2:
			gp1Result = gp1TextureWindowSetting;
			break;
		case 3:
			gp1Result = gp1TopLeft;
			break;
		case 4:
			gp1Result = gp1BottomRight;
			break;
		case 5:
			gp1Result = gp1DrawOffset;
			break;
		case 7:
			//std::cout << "~[GPU] Old 160pin GPU" << '\n';

			std::cout << "~[GPU] New 208pin GPU v2" << '\n';
			gp1Result = 0x2;
			break;
		default:
			std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x"
				<< std::hex << command << '\n';
			g_emulationPaused = true;
			break;
		}
		break;
	default:
		std::cout << "~[GPU] unhandled GPU1 command: 0x" << std::hex << command << '\n';
		g_emulationPaused = true;
		break;
	}
	return;
}

void Gpu::UpdateVramView(const Vector2D& possition, const PsxColor16& color16) {
	auto ConvertPSXtoPC_RGB_Values = [=](const PsxColor16& color16) {
		olc::Pixel pixel;
		pixel.r = color16.component.red << 3;
		pixel.g = color16.component.green << 3;
		pixel.b = color16.component.blue << 3;
		return pixel;
		};

	vramView->SetPixel(possition.x, possition.y, ConvertPSXtoPC_RGB_Values(color16));

	/*if (g_logPixel && (possition.x == g_mousePossitionX) && (possition.y == g_mousePossitionY)) {
		vramView->SetPixel(possition.x, possition.y, olc::YELLOW);
		std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << gp0Command
		<< " " << gp0Lookup[gp0Command >> 24].name << '\n';
		g_emulationPaused = true;
	}*/
}

void Gpu::WriteVram(const Vector2D& position, const uint16_t& data,
	const bool bIgnoreDrawingArea, const bool bDithering) {

	if (bIgnoreDrawingArea) {
		/*if ((position.x < 0) || (position.y < 0)) {
			std::cout << "~[GPU] EMULATION PAUSED! Write Vram out of bounds!" << std::dec
			<< " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			g_emulationPaused = true;
			return;
		}*/

		uint16_t backPixel;
		ReadVram(position, backPixel, bIgnoreDrawingArea);

		if ((gpuStat.elem.drawPixels &&
			((backPixel & 0x8000) == 0x0000)) || (gpuStat.elem.drawPixels == 0)) {

			*((uint16_t*)(vram.get() + ((position.x & 1023) << 1) + ((position.y & 511) << 11))) =
				data | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x8000 : 0);
			UpdateVramView(position, data);
		}
		return;
	}

	if (((position.x) >= drawAreaTopLeft.x) &&
		((position.y) >= drawAreaTopLeft.y) &&
		((position.x) <= drawAreaBottomRight.x) &&
		((position.y) <= drawAreaBottomRight.y)) {

		PsxColor16 pixelColor16;
		pixelColor16.data = data;

#if ENABLE_DITHERING //not finished some artifacts are present as I remember.
		if (bDithering) {
			int16_t ditheringOffset = ditheringLookup[position.x & 0x3 + ((position.y & 0x3) << 2)];
			int16_t red = ((uint16_t)pixelColor16.component.red << 3) + ditheringOffset;
			int16_t green = ((uint16_t)pixelColor16.component.green << 3) + ditheringOffset;
			int16_t blue = ((uint16_t)pixelColor16.component.blue << 3) + ditheringOffset;

			pixelColor16.component.red = ((red < 0) ? 0 : ((red > 0xff) ? 0xff : red)) >> 3;
			pixelColor16.component.green = ((green < 0) ? 0 : ((green > 0xff) ? 0xff : green)) >> 3;
			pixelColor16.component.blue = ((blue < 0) ? 0 : ((blue > 0xff) ? 0xff : blue)) >> 3;
		}
#endif

		uint16_t data = pixelColor16.data;

		uint16_t backPixel;
		ReadVram(position, backPixel, bIgnoreDrawingArea);
		if ((gpuStat.elem.drawPixels && ((backPixel & 0x8000) == 0x0000)) || (gpuStat.elem.drawPixels == 0)) {
			*((uint16_t*)(vram.get() + ((position.x & 1023) << 1) + ((position.y & 511) << 11))) =
				data | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x8000 : 0);
			UpdateVramView(position, data);
		}
	}
}

void Gpu::ReadVram(const Vector2D& position, uint16_t& data,
	const bool bIgnoreDrawingArea) const {

	if (bIgnoreDrawingArea) {
		/*if ((position.x < 0) || (position.y < 0)) {
			std::cout << "~[GPU] EMULATION PAUSED! Read Vram out of bounds!" << std::dec
			<< " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			g_emulationPaused = true;
			return;
		}*/

		data = *((uint16_t*)(vram.get() + ((position.x & 1023) << 1) + ((position.y & 511) << 11)));
		return;
	}

	if ((position.x >= drawAreaTopLeft.x) &&
		(position.y >= drawAreaTopLeft.y) &&
		(position.x <= drawAreaBottomRight.x) &&
		(position.y <= drawAreaBottomRight.y)) {

		data = *((uint16_t*)(vram.get() + ((position.x & 1023) << 1) + ((position.y & 511) << 11)));
	}
}

olc::Sprite* Gpu::GetVramSprite() {
	return vramView.get();
}