#include "cxd85xxx.h"
#include "../bus/bus_interface.h"

#include "math.h"
#include "limits"

extern bool g_emulationPaused;
extern bool g_GpuCommandsLogged;

#undef DISABLE_SCALING
#define DISABLE_SCALING 0

gpu_t::gpu_t() {
	gpuStat.data = 0x1c000000;

	textureCache = new uint16_t * [256];
	for (int i = 0; i < 256; i++) {
		textureCache[i] = new uint16_t[256];
	}

	gp0Lookup.resize(0xef);
	gp0Lookup = {
		//0x00
		{"NOP (?)",1},
		{"Clear Cache",1},
		{"Fill Rectangle in VRAM",3},
		{"Unknown?",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		//0x10
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Mirror of NOP (?)",1},
		{"Interrupt Request (IRQ1)",1},
		//0x20
		{"Monochrome three-point polygon, opaque",4},
		{"Undocumented/Nonsense",4},
		{"Monochrome three - point polygon, semi - transparent",4},
		{"Undocumented/Nonsense",4},
		{"Textured three-point polygon, opaque, texture-blending",7},
		{"Textured three-point polygon, opaque, raw-texture",7},
		{"Textured three-point polygon, semi-transparent, texture-blending",7},
		{"Textured three-point polygon, semi-transparent, raw-texture",7},
		{"Monochrome four-point polygon, opaque",5},
		{"Undocumented/Nonsense",5},
		{"Monochrome four-point polygon, semi-transparent",5},
		{"Undocumented/Nonsense",5},
		{"Textured four-point polygon, opaque, texture-blending",9},
		{"Textured four-point polygon, opaque, raw-texture",9},
		{"Textured four-point polygon, semi-transparent, texture-blending",9},
		{"Textured four-point polygon, semi-transparent, raw-texture",9},
		//0x30
		{"Shaded three-point polygon, opaque",6},
		{"Undocumented/Nonsense",6},
		{"Shaded three-point polygon, semi-transparent",6},
		{"Undocumented/Nonsense",6},
		{"Shaded Textured three-point polygon, opaque, texture-blending",9},
		{"Undocumented/Nonsense",9},
		{"Shaded Textured three-point polygon, semi-transparent, tex-blend",9},
		{"Undocumented/Nonsense",9},
		{"Shaded four-point polygon, opaque",8},
		{"Undocumented/Nonsense",8},
		{"Shaded four-point polygon, semi-transparent",8},
		{"Undocumented/Nonsense",8},
		{"Shaded Textured four-point polygon, opaque, texture-blending",12},
		{"Undocumented/Nonsense",12},
		{"Shaded Textured four-point polygon, semi-transparent, tex-blend",12},
		{"Undocumented/Nonsense",12},
		//0x40
		{"Monochrome line, opaque",3},
		{"Monochrome line, opaque",3}, //???
		{"Monochrome line, semi-transparent",3},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Monochrome Poly-line, opaque",3},
		{"Undocumented/Nonsense",1},
		{"Monochrome Poly-line, semi-transparent",3},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",3},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",3},
		{"Undocumented/Nonsense",1},
		//0x50
		{"Shaded line, opaque",4},
		{"Undocumented/Nonsense",4},
		{"Shaded line, semi-transparent",4},
		{"Undocumented/Nonsense",4},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",1},
		{"Shaded Poly-line, opaque",4},
		{"Undocumented/Nonsense",1},
		{"Shaded Poly-line, semi-transparent",4},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",4},
		{"Undocumented/Nonsense",1},
		{"Undocumented/Nonsense",4},
		{"Undocumented/Nonsense",1},
		//0x60
		{"Monochrome Rectangle (variable size) (opaque)",3},
		{"Undocumented/Nonsense",1},
		{"Monochrome Rectangle (variable size) (semi-transparent)",3},
		{"Undocumented/Nonsense",3},
		{"Textured Rectangle, variable size, opaque, texture-blending",4},
		{"Textured Rectangle, variable size, opaque, raw-texture",4},
		{"Textured Rectangle, variable size, semi-transp, texture-blending",4},
		{"Textured Rectangle, variable size, semi-transp, raw-texture",4},
		{"Monochrome Rectangle (1x1) (Dot) (opaque)",2},
		{"Undocumented/Nonsense",1},
		{"Monochrome Rectangle (1x1) (Dot) (semi-transparent)",2},
		{"Undocumented/Nonsense",1},
		{"Textured Rectangle, 1x1 (nonsense), opaque, texture-blending",3},
		{"Textured Rectangle, 1x1 (nonsense), opaque, raw-texture",3},
		{"Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending",3},
		{"Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture",3},
		//0x70
		{"Monochrome Rectangle (8x8) (opaque)",2},
		{"Undocumented/Nonsense",1},
		{"Monochrome Rectangle (8x8) (semi-transparent)",2},
		{"Undocumented/Nonsense",1},
		{"Textured Rectangle, 8x8, opaque, texture-blending",3},
		{"Textured Rectangle, 8x8, opaque, raw-texture",3},
		{"Textured Rectangle, 8x8, semi-transparent, texture-blending",3},
		{"Textured Rectangle, 8x8, semi-transparent, raw-texture",3},
		{"Monochrome Rectangle (16x16) (opaque)",2},
		{"Undocumented/Nonsense",1},
		{"Monochrome Rectangle (16x16) (semi-transparent)",2},
		{"Undocumented/Nonsense",1},
		{"Textured Rectangle, 16x16, opaque, texture-blending",3},
		{"Textured Rectangle, 16x16, opaque, raw-texture",3},
		{"Textured Rectangle, 16x16, semi-transparent, texture-blending",3},
		{"Textured Rectangle, 16x16, semi-transparent, raw-texture",3},
		//0x80
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		//0x90
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		{"Copy Rectangle (VRAM to VRAM)",4},
		//0xa0
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		//0xb0										  			   
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		{"Copy Rectangle (CPU to VRAM)",3},
		//0xc0
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		//0xd0										   			   
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		{"Copy Rectangle (VRAM to CPU)",3},
		//0xe0
		{"NOP (?)",1},
		{"Draw Mode setting (aka 'Texpage')",1},
		{"Texture Window setting",1},
		{"Set Drawing Area top left (X1,Y1)",1},
		{"Set Drawing Area bottom right (X2,Y2)",1},
		{"Set Drawing Offset (X,Y)",1},
		{"Mask Bit Setting",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
		{"NOP (?)",1},
	};
}

void gpu_t::readGpu32(const uint32_t& addr, uint32_t& data) {
	data = 0;
	if (addr == 0x1f801810) {
		if (bTransferCommand) {
			data = fifo[0];
			fifo.erase(fifo.begin());
			if (fifo.empty())
				bTransferCommand = false;
			return;
		}

		data = gp1Result;
		return;
	}

	if (addr == 0x1f801814)
		data = gpuStat.data;
}

void gpu_t::writeGpu32(const uint32_t& addr, const uint32_t& data) {
	if (addr == 0x1f801810) {
		//std::cout << "0x" << data << std::endl;
		if (collectCount) {
			fifo.push_back(data);
			collectCount--;
			if ((collectCount == 0) && (bCollectPolylineParam))
				if ((data & 0xf000f000) == 0x50005000)
					bCollectPolylineParam = false;
				else
					collectCount = 1;
		}
		else {
			gp0Command = data;
			fifo.clear();//!!!!
			if (!fifo.empty()) {
				std::cout << "[GPU] EMULATION PAUSED! collect list not empty 0x" << std::hex << fifo.size() << std::endl;
				g_emulationPaused = true;
			}

			collectCount = gp0Lookup[gp0Command >> 24].collectCount - 1;
			if (((gp0Command >> 24) >= 0xa0) && ((gp0Command >> 24) < 0xc0))
				bCollectCopyCpuVramParam = true;
			if ((gp0Command >> 29 == 0b010) && (gp0Command & 0x08000000))
				bCollectPolylineParam = true;

			//??
			if (pBus->dma.dma2.channelControl.reg.startBusy)
				fifoCount += gp0Lookup[gp0Command >> 24].collectCount;
		}

		if (collectCount == 0) {
			if (g_GpuCommandsLogged)
				std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << gp0Command << " " << gp0Lookup[gp0Command >> 24].name << std::endl;

			decodeExecuteGp0Command(gp0Command);
		}
		return;
	}

	if (addr == 0x1f801814) {
		gp1Command = data;
		if (g_GpuCommandsLogged)
			std::cout << "~[GPU] GPU1 command executed: 0x" << std::hex << gp1Command << std::endl;
		switch (gp1Command >> 24) {
		case 0x0:
			gpuStat.data = 0x1c802000;
			break;
		case 0x1:
			//std::cout << "~[GPU] unhandled properly GPU1 command: Reset Command Buffer" << std::endl;
			break;
		case 0x2:
			gpuStat.elem.irq1 = 0;
			break;
		case 0x3:
			gpuStat.elem.displayEnable = data & 0x1;
			break;
		case 0x4:
			gpuStat.elem.dmaDir = gp1Command & 0x3;
			break;
		case 0x5:
			display_start.x = gp1Command << 22 >> 22;
			display_start.y = gp1Command << 13 >> 23;
			break;
		case 0x6:
			x1 = gp1Command << 20 >> 20;
			x2 = gp1Command << 12 >> 24;
			break;
		case 0x7:
			y1 = gp1Command << 22 >> 22;
			y2 = gp1Command << 12 >> 22;
			break;
		case 0x8:
			gpuStat.elem.horizontalRes1 = gp1Command & 0x3;
			gpuStat.elem.verticalRes = (gp1Command & 0x4 ? 1 : 0);
			gpuStat.elem.videoMode = (gp1Command & 0x8 ? 1 : 0);
			gpuStat.elem.displayAreaColorDepth = (gp1Command & 0x10 ? 1 : 0);
			gpuStat.elem.verticalInterlace = (gp1Command & 0x20 ? 1 : 0);
			gpuStat.elem.horizontalRes2 = gp1Command & 0x40;
			gpuStat.elem.reverseflag = (gp1Command & 0x80 ? 1 : 0);

			if (gpuStat.elem.verticalInterlace)
				gpuStat.elem.interlaceField = 1;
			else
				gpuStat.elem.interlaceField = 0;
			break;
		case 0x10:
			switch (gp1Command & 0x7) {
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
				std::cout << "~[GPU] Old 160pin GPU" << std::endl;
				break;
			default:
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x" << std::hex << data << std::endl;
				g_emulationPaused = true;
				break;
			}
			break;
		default:
			std::cout << "~[GPU] unhandled GPU1 command: 0x" << std::hex << data << std::endl;
			g_emulationPaused = true;
			break;
		}
		return;
	}
}

void gpu_t::videoClock() {
	constexpr uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
	constexpr uint32_t CPU_CLOCKS_PER_FRAME = ((float)CPU_CLOCK / 59.29f);
	constexpr uint32_t CPU_CLOCKS_PER_SCANLINE = CPU_CLOCKS_PER_FRAME / 263;

	if (cpuFrameClocks == CPU_CLOCKS_PER_FRAME) {
		cpuFrameClocks = 0;

		//V-Sync interrupt is naturally edge trigered
		pBus->pCpu->cp0.interruptHandler(_IRQ_V_SYNC_);

		if (gpuStat.elem.verticalRes && gpuStat.elem.verticalInterlace)
			gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;

		//Vblank synchronization timer 1
		pBus->timer1.clock(_VBLANK_CLOCK);
	}

	if (cpuScanlineClocks == CPU_CLOCKS_PER_SCANLINE) {
		cpuScanlineClocks = 0;
		if (!gpuStat.elem.verticalRes)
			gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;

		//Hblank clocking timer 1
		pBus->timer1.clock(_HBLANK_CLOCK);

	}

	if (cpuFrameClocks % 10 == 0) {
		pBus->timer0.clock(_DOT_CLOCK);
	}

	if (cpuFrameClocks % 1000000 == 0) {
		if (pBus->spuControlRegister.elem.irq9Enable) {
			pBus->pCpu->cp0.interruptHandler(_IRQ_SPU);
			pBus->spuStatusRegister.elem.irq9Flag = 1;
		}
	}

	if (cpuFrameClocks == CPU_CLOCKS_PER_FRAME / 5) {
		//HAHHAHA BYPASS SPU_DMA
		if (pBus->dma.interruptRegister.reg.IRQ_enable_dma4 && pBus->dma.interruptRegister.reg.IRQ_master_enable) {
			pBus->dma.interruptRegister.reg.IRQ_master_flag = 1;
			pBus->dma.interruptRegister.reg.IRQ_flag_dma4 = 1;
			pBus->pCpu->cp0.interruptHandler(_IRQ_DMA);
		}
	}

	cpuFrameClocks++;
	cpuScanlineClocks++;

	//DMA ACCESING FIX!? //not working ok in all games
	/*if (fifoCount >= 16) {
		fifoClocks++;
		pBus->dma.dma2.channelControl.reg.startBusy = 0;
		pBus->dma.isCpuStopped = false;
		gpuStat.elem.readyDma = 0;
		if (fifoClocks >= 0x76) {
			fifoClocks = 0;
			fifoCount = 0;
			pBus->dma.dma2.channelControl.reg.startBusy = 1;
			pBus->dma.isCpuStopped = true;
			gpuStat.elem.readyDma = 1;
		}
	}*/
}

void gpu_t::updateVramView(const vertex_t& possition, const psxColor16_t& color16) {
	auto ConvertPSXtoPC_RGB_Values = [=](const psxColor16_t& color16) {
		olc::Pixel pixel;
		pixel.r = color16.component.red << 3;
		pixel.g = color16.component.green << 3;
		pixel.b = color16.component.blue << 3;
		//color correction for PC???
		/*if (color16.component.red <= 8)
			pixel.r = color16.component.red * 2 * 256 / 32;
		else {
			float k = (31. - 16.) / (31. - 8.);
			pixel.r = (uint8_t)((((float)color16.component.red - 8.) * k + 16.) * 256 / 32);
		}
		if (color16.component.green <= 8)
			pixel.g = color16.component.green * 2 * 256 / 32;
		else {
			float k = (31. - 16.) / (31. - 8.);
			pixel.g = (uint8_t)((((float)color16.component.green - 8.) * k + 16.) * 256 / 32);
		}
		if (color16.component.blue <= 8)
			pixel.b = color16.component.blue * 2 * 256 / 32;
		else {
			float k = (31. - 16.) / (31. - 8.);
			pixel.b = (uint8_t)((((float)color16.component.blue - 8.) * k + 16.) * 256 / 32);
		}*/
		return pixel;
		};
	vramView.SetPixel(possition.x, possition.y, ConvertPSXtoPC_RGB_Values(color16));
}

void gpu_t::writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		if ((position.x < 0) || (position.y) < 0) {
			/*std::cout << "~[GPU] EMULATION PAUSED! Write Vram out of bounds!" << std::dec << " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			g_emulationPaused = true;*/
			return;
		}

		if (((gpuStat.elem.drawPixels == 1) && ((data & 0x8000) == 0x8000)) || (gpuStat.elem.drawPixels == 0)) {
			vram[(position.x & 1023) * 2 * size_y + (position.y & 511)] = data << 8 >> 8;
			vram[((position.x & 1023) * 2 + 1) * size_y + (position.y & 511)] = (data >> 8) | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x80 : 0);
			updateVramView(position, data);
		}
		return;
	}

	if (((position.x + (drawOffset.x >> 5)) >= drawAreaTopLeft.x) &&
		((position.x + (drawOffset.x >> 5)) <= drawAreaBottomRight.x) &&
		((position.y + (drawOffset.y >> 5)) >= drawAreaTopLeft.y) &&
		((position.y + (drawOffset.y >> 5)) <= drawAreaBottomRight.y)) {

		if ((position.x + (drawOffset.x >> 5) < 0) || (position.y + (drawOffset.y >> 5)) < 0) {
			/*std::cout << "~[GPU] EMULATION PAUSED! Write Vram out of bounds!" << std::dec << " (x,y) (" << position.x + (drawOffset.x >> 5) << "," <<
				position.y + (drawOffset.y >> 5) << ")" << "\n";
			g_emulationPaused = true;*/
			return;
		}

		if (((gpuStat.elem.drawPixels == 1) && ((data & 0x8000) == 0x8000)) || (gpuStat.elem.drawPixels == 0)) {
			vram[((position.x + (drawOffset.x >> 5)) & 1023) * 2 * size_y + ((position.y + (drawOffset.y >> 5)) & 511)] = data << 8 >> 8;
			vram[(((position.x + (drawOffset.x >> 5)) & 1023) * 2 + 1) * size_y + ((position.y + (drawOffset.y >> 5)) & 511)] = (data >> 8) | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x80 : 0);

			vertex_t p = { 0, 0 };
			p.x = position.x + (drawOffset.x >> 5);
			p.y = position.y + (drawOffset.y >> 5);
			updateVramView(p, data);
		}
	}
	//else {
	//	if ((position.x + (drawOffset.x >> 5) < 0) || (position.y + (drawOffset.y >> 5)) < 0) {
	//		/*std::cout << "~[GPU] EMULATION PAUSED! Write Vram out of bounds!" << std::dec << " (x,y) (" << position.x + (drawOffset.x >> 5) << "," <<
	//			position.y + (drawOffset.y >> 5) << ")" << "\n";
	//		g_emulationPaused = true;*/
	//		return;
	//	}

	//	vram[(((position.x + (drawOffset.x >> 5)) & 1023) - ((position.x + (drawOffset.x >> 5)) >= drawAreaBottomRight.x) ? drawAreaBottomRight.x/* - (drawOffset.x >> 5) */: 0) * 2 * size_y +
	//		(((position.y + (drawOffset.y >> 5)) & 511) - ((position.y + (drawOffset.y >> 5)) >= drawAreaBottomRight.y) ? drawAreaBottomRight.y /*- (drawOffset.y >> 5)*/ : 0)] = data << 8 >> 8;
	//	vram[((((position.x + (drawOffset.x >> 5)) & 1023) - ((position.x + (drawOffset.x >> 5)) >= drawAreaBottomRight.x) ? drawAreaBottomRight.x /*- (drawOffset.x >> 5)*/ : 0) * 2 + 1) * size_y +
	//		(((position.y + (drawOffset.y >> 5)) & 511) - ((position.y + (drawOffset.y >> 5)) >= drawAreaBottomRight.y) ? drawAreaBottomRight.y /*- (drawOffset.y >> 5)*/ : 0)] = data >> 8;

	//	vertex_t p = { 0, 0 };
	//	p.x = position.x + (drawOffset.x >> 5) - (((position.x + (drawOffset.x >> 5)) >= drawAreaBottomRight.x) ? drawAreaBottomRight.x /*- (drawOffset.x >> 5)*/ : 0);
	//	p.y = position.y + (drawOffset.y >> 5) - (((position.y + (drawOffset.y >> 5)) >= drawAreaBottomRight.y) ? drawAreaBottomRight.y /*- (drawOffset.y >> 5)*/ : 0);
	//	updateVramView(p, data);
	//}
}

void gpu_t::readVram(const vertex_t& position, uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		if ((position.x < 0) || (position.y) < 0) {
			std::cout << "~[GPU] EMULATION PAUSED! Read Vram out of bounds!" << std::dec << " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			//g_emulationPaused = true;
			return;
		}

		data = vram[(position.x & 1023) * 2 * size_y + (position.y & 511)];
		data |= vram[((position.x & 1023) * 2 + 1) * size_y + (position.y & 511)] << 8;
		return;
	}

	if (((position.x + (drawOffset.x >> 5)) >= drawAreaTopLeft.x) &&
		((position.x + (drawOffset.x >> 5)) <= drawAreaBottomRight.x) &&
		((position.y + (drawOffset.y >> 5)) >= drawAreaTopLeft.y) &&
		((position.y + (drawOffset.y >> 5)) <= drawAreaBottomRight.y)) {

		if ((position.x + (drawOffset.x >> 5) < 0) || (position.y + (drawOffset.y >> 5)) < 0) {
			/*std::cout << "~[GPU] EMULATION PAUSED! Read Vram out of bounds!" << std::dec << " (x,y) (" << position.x + (drawOffset.x >> 5) << "," <<
				position.y + (drawOffset.y >> 5) << ")" << "\n";
			g_emulationPaused = true;*/
			return;
		}

		data = vram[((position.x + (drawOffset.x >> 5)) & 1023) * 2 * size_y + ((position.y + (drawOffset.y >> 5)) & 511)];
		data |= vram[(((position.x + (drawOffset.x >> 5)) & 1023) * 2 + 1) * size_y + ((position.y + (drawOffset.y >> 5)) & 511)] << 8;
	}
}

olc::Sprite* gpu_t::getVramSprite() {
	return &vramView;
}

uint16_t gpu_t::format24to16Color(const uint32_t& input) {
	psxColor24_t color24 = (psxColor24_t)(input & 0x00ffffff);
	psxColor16_t color16 = 0;
	color16.component.red = color24.component.red >> 3;
	color16.component.green = color24.component.green >> 3;
	color16.component.blue = color24.component.blue >> 3;
	return color16.data;
}

uint32_t gpu_t::format16to24Color(const uint16_t& input) {
	psxColor24_t color24 = 0;
	psxColor16_t color16 = (psxColor16_t)(input & 0x7fff);
	color24.component.red = color16.component.red << 3;
	color24.component.green = color16.component.green << 3;
	color24.component.blue = color16.component.blue << 3;
	return color24.data;
}

float gpu_t::edgeFunc(const vertex_t& vert1, const vertex_t& vert2, const vertex_t& point) {
	return ((float)vert1.y - (float)vert2.y) * ((float)point.x - (float)vert1.x) +
		((float)vert1.x - (float)vert2.x) * ((float)vert1.y - (float)point.y);
}

void gpu_t::calculateTransparency(vertex_t p, psxColor16_t& frontPixelColor) {
	psxColor16_t backPixelColor;
	readVram(p, backPixelColor.data);
	switch (gpuStat.elem.semiTransparency) {
	case 0: //B/2+F/2
		frontPixelColor.component.red = backPixelColor.component.red / 2 + frontPixelColor.component.red / 2;
		frontPixelColor.component.green = backPixelColor.component.green / 2 + frontPixelColor.component.green / 2;
		frontPixelColor.component.blue = backPixelColor.component.blue / 2 + frontPixelColor.component.blue / 2;
		break;
	case 1: //B+F
		frontPixelColor.component.red = (backPixelColor.component.red + frontPixelColor.component.red) > 0x1f ? 0x1f : backPixelColor.component.red + frontPixelColor.component.red;
		frontPixelColor.component.green = (backPixelColor.component.green + frontPixelColor.component.green) > 0x1f ? 0x1f : backPixelColor.component.green + frontPixelColor.component.green;
		frontPixelColor.component.blue = (backPixelColor.component.blue + frontPixelColor.component.blue) > 0x1f ? 0x1f : backPixelColor.component.blue + frontPixelColor.component.blue;
		break;
	case 2: //B-F
		frontPixelColor.component.red = (backPixelColor.component.red < frontPixelColor.component.red) ? 0 : backPixelColor.component.red - frontPixelColor.component.red;
		frontPixelColor.component.green = (backPixelColor.component.green < frontPixelColor.component.green) ? 0 : backPixelColor.component.green - frontPixelColor.component.green;
		frontPixelColor.component.blue = (backPixelColor.component.blue < frontPixelColor.component.blue) ? 0 : backPixelColor.component.blue - frontPixelColor.component.blue;
		break;
	case 3: //B+F/4
		frontPixelColor.component.red = (backPixelColor.component.red + frontPixelColor.component.red / 4) > 0x1f ? 0x1f : backPixelColor.component.red + frontPixelColor.component.red / 4;
		frontPixelColor.component.green = (backPixelColor.component.green + frontPixelColor.component.green / 4) > 0x1f ? 0x1f : backPixelColor.component.green + frontPixelColor.component.green / 4;
		frontPixelColor.component.blue = (backPixelColor.component.blue + frontPixelColor.component.blue / 4) > 0x1f ? 0x1f : backPixelColor.component.blue + frontPixelColor.component.blue / 4;
		break;
	}
}

static void swap(int& a, int& b) {
	int c = b;
	b = a; a = c;
}

static void swap(uint32_t& a, uint32_t& b) {
	int c = b;
	b = a; a = c;
}

static void swap(float& a, float& b) {
	float c = b;
	b = a; a = c;
}

void gpu_t::drawPolygon(param_t* pParam) {

	const uint8_t vertexCount = pParam->verticesNumber;
	const shadingType_t shadingType = pParam->shadingType;
	const transpanencyType_t transparencyType = pParam->transpanencyType;

	vertex_t vert1 = pParam->vert1;
	vertex_t vert2 = pParam->vert2;
	vertex_t vert3 = pParam->vert3;
	vertex_t vert4 = pParam->vert4;

	psxColor24_t color1 = pParam->commandColor1;
	psxColor24_t color2 = pParam->color2;
	psxColor24_t color3 = pParam->color3;
	psxColor24_t color4 = pParam->color4;

	vertex_t p;
	psxColor24_t calculatedColor;

	int16_t arrayX[4] = { vert1.x, vert2.x, vert3.x, vert4.x };
	int16_t arrayY[4] = { vert1.y, vert2.y, vert3.y, vert4.y };

	int16_t xMin = utils::min(arrayX, vertexCount);
	const int16_t xMax = utils::max(arrayX, vertexCount);
	const int16_t yMin = utils::min(arrayY, vertexCount);
	const int16_t yMax = utils::max(arrayY, vertexCount);

	auto drawTriangle = [&]() {
		float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;

		triangleOrientation_t orientation = P123 > 0 ? PSX_CLOCKWISE : PSX_ANTI_CLOCKWISE;

		vertex_t e1 = vert3 - vert2;
		vertex_t e2 = vert1 - vert3;
		vertex_t e3 = vert2 - vert1;


		if (orientation == PSX_CLOCKWISE)
			for (int y = yMin; y <= yMax; y++) {
				p.y = y;
				bool flag = false;
				bool oldFlag = false;
				for (int x = xMin; x <= xMax; x++) {
					p.x = x;

					float e12 = edgeFunc(vert1, vert2, p);
					//float e23 = edgeFunc(vert2, vert3, p);
					//float e31 = edgeFunc(vert3, vert1, p);

					auto testEdgeAndWrite = [&]() {
						if ((e12 >= 0) /*|| ((e12 == 0) ? ((e3.y == 0) && (vert2.y < vert3.y)) || (vert2.y < vert1.y) : false)*/) {
							float e23 = edgeFunc(vert2, vert3, p);
							if ((e23 >= 0) /*|| ((e23 == 0) ? ((e1.y == 0) && (vert3.y < vert1.y)) || (vert3.y < vert2.y) : false)*/) {
								float e31 = edgeFunc(vert3, vert1, p);
								if ((e31 >= 0) /*|| ((e31 == 0) ? ((e2.y == 0) && (vert1.y < vert2.y)) || (vert1.y < vert3.y) : false)*/) {
									psxColor16_t frontPixelColor;

									if (pParam->shadingType == GOURAUD_SHADING) {
										float lamda1 = abs(e23 / 2.f / P123);
										float lamda2 = abs(e31 / 2.f / P123);
										float lamda3 = abs(e12 / 2.f / P123);
										calculatedColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
										calculatedColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
										calculatedColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
										frontPixelColor.data = format24to16Color(calculatedColor.data);
									}

									if (pParam->shadingType == FLAT_SHADING)
										frontPixelColor.data = format24to16Color(pParam->commandColor1 & 0x00ffffff);

									if (pParam->transpanencyType == SEMI_TRANSPARENT)
										calculateTransparency(p, frontPixelColor);


									writeVram(p, frontPixelColor.data);
									flag = true;
								}
							}
						}
						};

					oldFlag = flag;
					flag = false;
					testEdgeAndWrite();

					if ((oldFlag == true) && (flag == false))
						break;
				}
			}


		if (orientation == PSX_ANTI_CLOCKWISE) {
			P123 = -P123;
			for (int y = yMin; y <= yMax; y++) {
				p.y = y;
				bool flag = false;
				bool oldFlag = false;
				for (int x = xMin; x <= xMax; x++) {
					p.x = x;

					float e21 = edgeFunc(vert2, vert1, p);
					//float e32 = edgeFunc(vert3, vert2, p);
					//float e13 = edgeFunc(vert1, vert3, p);

					auto testEdgeAndWrite = [&]() {
						if ((e21 >= 0)/* || ((e21 == 0) ? ((e3.y == 0) && (vert2.y > vert3.y)) || (vert2.y > vert1.y) : false)*/) {
							float e32 = edgeFunc(vert3, vert2, p);
							if ((e32 >= 0) /*|| ((e32 == 0) ? ((e1.y == 0) && (vert3.y > vert1.y)) || (vert3.y > vert2.y) : false)*/) {
								float e13 = edgeFunc(vert1, vert3, p);
								if ((e13 >= 0) /*|| ((e13 == 0) ? ((e2.y == 0) && (vert1.y > vert2.y)) || (vert1.y > vert3.y) : false)*/) {

									psxColor16_t frontPixelColor;

									if (pParam->shadingType == GOURAUD_SHADING) {
										float lamda1 = abs(e32 / 2.f / P123);
										float lamda2 = abs(e13 / 2.f / P123);
										float lamda3 = abs(e21 / 2.f / P123);
										calculatedColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
										calculatedColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
										calculatedColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
										frontPixelColor.data = format24to16Color(calculatedColor.data);
									}

									if (pParam->shadingType == FLAT_SHADING)
										frontPixelColor.data = format24to16Color(pParam->commandColor1 & 0x00ffffff);

									if (pParam->transpanencyType == SEMI_TRANSPARENT)
										calculateTransparency(p, frontPixelColor);


									writeVram(p, frontPixelColor.data);
									flag = true;
								}
							}
						}
						};

					oldFlag = flag;
					flag = false;
					testEdgeAndWrite();

					if ((oldFlag == true) && (flag == false))
						break;
				}
			}
		}

		};

	drawTriangle();

	if (vertexCount == 4) {
		vert1 = vert2;
		vert2 = vert3;
		vert3 = vert4;
		color1 = color2;
		color2 = color3;
		color3 = color4;
		drawTriangle();
	}

	//const uint8_t vertexCount = pParam->verticesNumber;

	//psxColor24_t blendingColor;
	//psxColor16_t frontPixelColor;

	//vertex_t vert1 = pParam->vert1;
	//psxColor24_t color1 = pParam->commandColor1;
	//int x1 = pParam->vert1.x;
	//int y1 = pParam->vert1.y;

	//vertex_t vert2 = pParam->vert2;
	//psxColor24_t color2 = pParam->color2;
	//int x2 = pParam->vert2.x;
	//int y2 = pParam->vert2.y;

	//vertex_t vert3 = pParam->vert3;
	//psxColor24_t color3 = pParam->color3;
	//int x3 = pParam->vert3.x;
	//int y3 = pParam->vert3.y;


	//auto drawTriangle = [&]() {

	//	if (y2 < y1) {
	//		swap(y1, y2);
	//		swap(x1, x2);
	//	}

	//	if (y3 < y1) {
	//		swap(y1, y3);
	//		swap(x1, x3);
	//	}

	//	if (y3 < y2) {
	//		swap(y2, y3);
	//		swap(x2, x3);
	//	}

	//	int dy1 = y2 - y1;
	//	int dx1 = x2 - x1;

	//	int dy2 = y3 - y1;
	//	int dx2 = x3 - x1;

	//	float dax_step = 0, dbx_step = 0;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	if (dy1) {
	//		for (int i = y1; i <= y2; i++) {

	//			float ax = x1 + (float)(i - y1) * dax_step;
	//			float bx = x1 + (float)(i - y1) * dbx_step;

	//			if (ax > bx)
	//				swap(ax, bx);

	//			for (int j = round(ax); j < round(bx); j++) {

	//				vertex_t p(j, i);
	//				//if (frontPixelColor.data) {
	//					//if (pParam->shadingType == GOURAUD_SHADING) {
	//				float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
	//				float e12 = edgeFunc(vert1, vert2, p);
	//				float e23 = edgeFunc(vert2, vert3, p);
	//				float e13 = edgeFunc(vert1, vert3, p);
	//				float lamda1 = abs(e23 / 2.f / P123);
	//				float lamda2 = abs(e13 / 2.f / P123);
	//				float lamda3 = abs(e12 / 2.f / P123);
	//				blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
	//				blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
	//				blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
	//				//}

	//				//if (pParam->shadingType == FLAT_SHADING)
	//				//	blendingColor = pParam->commandColor1;

	//				//if (pParam->modulationType == TEXTURE_BLENDING) {
	//				//	float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
	//				//	float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
	//				//	float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
	//				//	frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
	//				//	frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
	//				//	frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
	//				//}

	//				//if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
	//				//	calculateTransparency(p, frontPixelColor);


	//				writeVram(p, format24to16Color(blendingColor.data));
	//				//}
	//			}

	//		}
	//	}

	//	dy1 = y3 - y2;
	//	dx1 = x3 - x2;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	if (dy1) {
	//		for (int i = y2; i <= y3; i++) {
	//			float ax = x2 + (float)(i - y2) * dax_step;
	//			float bx = x1 + (float)(i - y1) * dbx_step;

	//			if (ax > bx)
	//				swap(ax, bx);

	//			for (int j = round(ax); j < round(bx); j++) {

	//				vertex_t p(j, i);
	//				//if (frontPixelColor.data) {
	//					//if (pParam->shadingType == GOURAUD_SHADING) {
	//				float P123 = edgeFunc(vert1, vert2, vert3) / 2.0f;
	//				float e12 = edgeFunc(vert1, vert2, p);
	//				float e23 = edgeFunc(vert2, vert3, p);
	//				float e13 = edgeFunc(vert1, vert3, p);
	//				float lamda1 = abs(e23 / 2.0f / P123);
	//				float lamda2 = abs(e13 / 2.0f / P123);
	//				float lamda3 = abs(e12 / 2.0f / P123);
	//				blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
	//				blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
	//				blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
	//				//}

	//				//if (pParam->shadingType == FLAT_SHADING)
	//				//	blendingColor = pParam->commandColor1;

	//				//if (pParam->modulationType == TEXTURE_BLENDING) {
	//				//	float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
	//				//	float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
	//				//	float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
	//				//	frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
	//				//	frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
	//				//	frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
	//				//}

	//				//if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
	//				//	calculateTransparency(p, frontPixelColor);


	//				writeVram(p, format24to16Color(blendingColor.data));
	//				//}
	//			}
	//		}
	//	}

	//	};

	//drawTriangle();

	//if (vertexCount == 4) {
	//	vert1 = pParam->vert2;
	//	color1 = pParam->color2;
	//	x1 = pParam->vert2.x;
	//	y1 = pParam->vert2.y;

	//	vert2 = pParam->vert3;
	//	color2 = pParam->color3;
	//	x2 = pParam->vert3.x;
	//	y2 = pParam->vert3.y;

	//	vert3 = pParam->vert4;
	//	color3 = pParam->color4;
	//	x3 = pParam->vert4.x;
	//	y3 = pParam->vert4.y;

	//	drawTriangle();
	//}

}

void gpu_t::drawTexPolygon(param_t* pParam) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (pParam->texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (pParam->texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	struct {
		bool bit15;
		uint8_t devider;
		uint8_t firstAnder;
		uint8_t shifter;
		uint8_t secondAnder;
	}texBittnesVar;

	switch (gpuStat.elem.texPageColors) {
	case 0: //4bit
		texBittnesVar.bit15 = false;
		texBittnesVar.devider = 4;
		texBittnesVar.firstAnder = 0x3;
		texBittnesVar.shifter = 2;
		texBittnesVar.secondAnder = 0xf;
		break;
	case 1: //8bit
		texBittnesVar.bit15 = false;
		texBittnesVar.devider = 2;
		texBittnesVar.firstAnder = 0x1;
		texBittnesVar.shifter = 3;
		texBittnesVar.secondAnder = 0xff;
		break;
	case 2: //16bit
		texBittnesVar.bit15 = true;
		texBittnesVar.devider = 1;
		break;
	case 3: //Reserved
		throw std::runtime_error("texture page color mode RESERVED!");
		break;
	}

	clut_t clut = pParam->texCoord1Palette.elem.attribute;

	const uint8_t vertexCount = pParam->verticesNumber;

	vertex_t vert1 = pParam->vert1;
	psxColor24_t color1 = pParam->commandColor1;
	int x1 = pParam->vert1.x;
	int y1 = pParam->vert1.y;
	float u1 = pParam->texCoord1Palette.elem.x;
	float v1 = pParam->texCoord1Palette.elem.y;

	vertex_t vert2 = pParam->vert2;
	psxColor24_t color2 = pParam->color2;
	int x2 = pParam->vert2.x;
	int y2 = pParam->vert2.y;
	float u2 = pParam->texCoord2TexPage.elem.x;
	float v2 = pParam->texCoord2TexPage.elem.y;

	vertex_t vert3 = pParam->vert3;
	psxColor24_t color3 = pParam->color3;
	int x3 = pParam->vert3.x;
	int y3 = pParam->vert3.y;
	float u3 = pParam->texCoord3.elem.x;
	float v3 = pParam->texCoord3.elem.y;

	psxColor24_t blendingColor;
	psxColor16_t frontPixelColor;

	//for (int v = 0; v < 256; v++) {
	//	for (int u = 0; u < 256 / texBittnesVar.devider; u++) {
	//		uint16_t data;
	//		vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + u, (gpuStat.elem.texPageYBase << 8) + v);
	//		readVram(tex_p, data, true);

	//		if (texBittnesVar.bit15)
	//			textureCache[u * texBittnesVar.devider][v] = data;
	//		else {
	//			for (int i = 0; i < texBittnesVar.devider; i++) {
	//				vertex_t pClut;
	//				pClut.x = (clut.elem.xCoord << 4) +
	//					((data >> (i << texBittnesVar.shifter)) & texBittnesVar.secondAnder);
	//				pClut.y = clut.elem.yCoord;
	//				readVram(pClut, frontPixelColor.data, true);

	//				textureCache[u * texBittnesVar.devider + i][v] = frontPixelColor.data;
	//				vertex_t p(u * texBittnesVar.devider + i, v);
	//				psxColor16_t color16;
	//				color16.data = frontPixelColor.data;
	//				/*if (frontPixelColor.data)
	//					writeVram(p, frontPixelColor.data);*/
	//			}
	//		}
	//	}
	//}


	auto drawTexTriangle = [&]() {

		if (y2 < y1) {
			swap(y1, y2);
			swap(x1, x2);
			swap(u1, u2);
			swap(v1, v2);
		}

		if (y3 < y1) {
			swap(y1, y3);
			swap(x1, x3);
			swap(u1, u3);
			swap(v1, v3);
		}

		if (y3 < y2) {
			swap(y2, y3);
			swap(x2, x3);
			swap(u2, u3);
			swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);

		if (dy1) {
			for (int i = y1; i <= y2; i++) {
				float ax = x1 + (float)(i - y1) * dax_step;
				float bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				/*std::cout << "/////////////////" << std::endl;
				std::cout << ((float)(bx - ax)) << std::endl;
				std::cout << abs(tex_eu - tex_su) * 255 << std::endl;*/


				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {

					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					vertex_t pTex;
					pTex.y = gpuStat.elem.texPageYBase << 8;
					pTex.y += (int)round(tex_v) & 255;
					pTex.y = (pTex.y & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

					pTex.x = gpuStat.elem.texPageXBase << 6;
					pTex.x += (int)round(tex_u) & 255;
					pTex.x = (pTex.x & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + ((int)round(tex_u) & 255) / texBittnesVar.devider, (gpuStat.elem.texPageYBase << 8) + (int)round(tex_v));
					readVram(tex_p, data, true);

					if (texBittnesVar.bit15)
						frontPixelColor.data = data;
					else {
						vertex_t pClut;
						pClut.x = (clut.elem.xCoord << 4) +
							((data >> ((pTex.x & texBittnesVar.firstAnder) << texBittnesVar.shifter)) & texBittnesVar.secondAnder);
						pClut.y = clut.elem.yCoord;
						readVram(pClut, frontPixelColor.data, true);
					}

					//frontPixelColor.data = textureCache[(int)round(tex_u)][(int)round(tex_v)];


					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}


					t += tstep;
				}

			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy1) {
			for (int i = y2; i <= y3; i++) {
				float ax = x2 + (float)(i - y2) * dax_step;
				float bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				//std::cout << "/////////////////" << std::endl;
				//std::cout << ((float)(bx - ax)) << std::endl;
				//std::cout << abs(tex_eu - tex_su) << std::endl;

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {

					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					vertex_t pTex;
					pTex.y = gpuStat.elem.texPageYBase << 8;
					pTex.y += (int)round(tex_v) & 255;
					pTex.y = (pTex.y & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

					pTex.x = gpuStat.elem.texPageXBase << 6;
					pTex.x += (int)round(tex_u) & 255;
					pTex.x = (pTex.x & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + ((int)round(tex_u) & 255) / texBittnesVar.devider, (gpuStat.elem.texPageYBase << 8) + (int)round(tex_v));
					readVram(tex_p, data, true);

					if (texBittnesVar.bit15)
						frontPixelColor.data = data;
					else {
						vertex_t pClut;
						pClut.x = (clut.elem.xCoord << 4) +
							((data >> ((pTex.x & texBittnesVar.firstAnder) << texBittnesVar.shifter)) & texBittnesVar.secondAnder);
						pClut.y = clut.elem.yCoord;
						readVram(pClut, frontPixelColor.data, true);
					}

					//frontPixelColor.data = textureCache[(int)round(tex_u)][(int)round(tex_v)];

					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}


					t += tstep;
				}
			}
		}
		};

	drawTexTriangle();
	if (vertexCount == 4) {
		vert1 = pParam->vert2;
		color1 = pParam->color2;
		x1 = pParam->vert2.x;
		y1 = pParam->vert2.y;
		u1 = pParam->texCoord2TexPage.elem.x;
		v1 = pParam->texCoord2TexPage.elem.y;

		vert2 = pParam->vert3;
		color2 = pParam->color3;
		x2 = pParam->vert3.x;
		y2 = pParam->vert3.y;
		u2 = pParam->texCoord3.elem.x;
		v2 = pParam->texCoord3.elem.y;

		vert3 = pParam->vert4;
		color3 = pParam->color4;
		x3 = pParam->vert4.x;
		y3 = pParam->vert4.y;
		u3 = pParam->texCoord4.elem.x;
		v3 = pParam->texCoord4.elem.y;
		drawTexTriangle();
	}
}

void gpu_t::drawTexPolygon15Bit(param_t* pParam) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (pParam->texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (pParam->texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	const uint8_t vertexCount = pParam->verticesNumber;
	const shadingType_t shadingType = pParam->shadingType;
	const transpanencyType_t transparencyType = pParam->transpanencyType;

	vertex_t vert1 = pParam->vert1;
	psxColor24_t color1 = pParam->commandColor1;
	int x1 = pParam->vert1.x;
	int y1 = pParam->vert1.y;
	int u1 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord1Palette.elem.x;
	int v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord1Palette.elem.y;

	vertex_t vert2 = pParam->vert2;
	psxColor24_t color2 = pParam->color2;
	int x2 = pParam->vert2.x;
	int y2 = pParam->vert2.y;
	int u2 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord2TexPage.elem.x;
	int v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

	vertex_t vert3 = pParam->vert3;
	psxColor24_t color3 = pParam->color3;
	int x3 = pParam->vert3.x;
	int y3 = pParam->vert3.y;
	int u3 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord3.elem.x;
	int v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

	psxColor24_t blendingColor = pParam->commandColor1;
	psxColor16_t frontPixelColor;

	auto drawTexTriangle = [&]() {
		if (y2 < y1) {
			swap(y1, y2);
			swap(x1, x2);
			swap(u1, u2);
			swap(v1, v2);
		}

		if (y3 < y1) {
			swap(y1, y3);
			swap(x1, x3);
			swap(u1, u3);
			swap(v1, v3);
		}

		if (y3 < y2) {
			swap(y2, y3);
			swap(x2, x3);
			swap(u2, u3);
			swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		int dv1 = v2 - v1;
		int du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		int dv2 = v3 - v1;
		int du2 = u3 - u1;

		int tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);

		if (dy1) {
			for (int i = y1; i <= y2; i++) {
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				int tex_su = u1 + (float)(i - y1) * du1_step;
				int tex_sv = v1 + (float)(i - y1) * dv1_step;

				int tex_eu = u1 + (float)(i - y1) * du2_step;
				int tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p(tex_u, tex_v);
					readVram(tex_p, frontPixelColor.data, true);
					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
				}


			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy1) {
			for (int i = y2; i <= y3; i++) {
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				int tex_su = u2 + (float)(i - y2) * du1_step;
				int tex_sv = v2 + (float)(i - y2) * dv1_step;

				int tex_eu = u1 + (float)(i - y1) * du2_step;
				int tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p(tex_u, tex_v);
					readVram(tex_p, frontPixelColor.data, true);
					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
				}
			}
		}
		};

	drawTexTriangle();
	if (vertexCount == 4) {
		vert1 = pParam->vert2;
		color1 = pParam->color2;
		x1 = pParam->vert2.x;
		y1 = pParam->vert2.y;
		u1 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord2TexPage.elem.x;
		v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

		vert2 = pParam->vert3;
		color2 = pParam->color3;
		x2 = pParam->vert3.x;
		y2 = pParam->vert3.y;
		u2 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord3.elem.x;
		v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

		vert3 = pParam->vert4;
		color3 = pParam->color4;
		x3 = pParam->vert4.x;
		y3 = pParam->vert4.y;
		u3 = (gpuStat.elem.texPageXBase << 6) + pParam->texCoord4.elem.x;
		v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord4.elem.y;
		drawTexTriangle();
	}
}

void gpu_t::drawTexPolygon8Bit(param_t* pParam) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (pParam->texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (pParam->texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	clut_t clut = pParam->texCoord1Palette.elem.attribute;
	const uint8_t vertexCount = pParam->verticesNumber;
	const shadingType_t shadingType = pParam->shadingType;
	const transpanencyType_t transparencyType = pParam->transpanencyType;

	vertex_t vert1 = pParam->vert1;
	psxColor24_t color1 = pParam->commandColor1;
	int x1 = pParam->vert1.x;
	int y1 = pParam->vert1.y;
	int u1 = pParam->texCoord1Palette.elem.x;
	int v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord1Palette.elem.y;

	vertex_t vert2 = pParam->vert2;
	psxColor24_t color2 = pParam->color2;
	int x2 = pParam->vert2.x;
	int y2 = pParam->vert2.y;
	int u2 = pParam->texCoord2TexPage.elem.x;
	int v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

	vertex_t vert3 = pParam->vert3;
	psxColor24_t color3 = pParam->color3;
	int x3 = pParam->vert3.x;
	int y3 = pParam->vert3.y;
	int u3 = pParam->texCoord3.elem.x;
	int v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

	psxColor24_t blendingColor = pParam->commandColor1;
	psxColor16_t frontPixelColor;

	auto drawTexTriangle = [&]() {
		if (y2 < y1) {
			swap(y1, y2);
			swap(x1, x2);
			swap(u1, u2);
			swap(v1, v2);
		}

		if (y3 < y1) {
			swap(y1, y3);
			swap(x1, x3);
			swap(u1, u3);
			swap(v1, v3);
		}

		if (y3 < y2) {
			swap(y2, y3);
			swap(x2, x3);
			swap(u2, u3);
			swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		int dv1 = v2 - v1;
		int du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		int dv2 = v3 - v1;
		int du2 = u3 - u1;

		int tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);

		if (dy1) {
			for (int i = y1; i <= y2; i++) {
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				int tex_su = u1 + (float)(i - y1) * du1_step;
				int tex_sv = v1 + (float)(i - y1) * dv1_step;

				int tex_eu = u1 + (float)(i - y1) * du2_step;
				int tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				bool flag = false;
				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + tex_u / 2, tex_v);
					readVram(tex_p, data, true);
					vertex_t pClut;
					pClut.x = (clut.elem.xCoord << 4) + ((data >> ((tex_u & 0x1) << 3)) & 0xff);
					pClut.y = clut.elem.yCoord;
					readVram(pClut, frontPixelColor.data, true);
					vertex_t p(j, i);

					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
					flag = !flag;
				}


			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy1) {
			for (int i = y2; i <= y3; i++) {
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				int tex_su = u2 + (float)(i - y2) * du1_step;
				int tex_sv = v2 + (float)(i - y2) * dv1_step;

				int tex_eu = u1 + (float)(i - y1) * du2_step;
				int tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				bool flag = false;
				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + tex_u / 2, tex_v);
					readVram(tex_p, data, true);
					vertex_t pClut;
					pClut.x = clut.elem.xCoord * 16 + (flag ? data & 0xff : (data >> 8) & 0xff);
					pClut.y = clut.elem.yCoord;
					readVram(pClut, frontPixelColor.data, true);
					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
					flag = !flag;
				}
			}
		}
		};

	drawTexTriangle();
	if (vertexCount == 4) {
		vert1 = pParam->vert2;
		color1 = pParam->color2;
		x1 = pParam->vert2.x;
		y1 = pParam->vert2.y;
		u1 = pParam->texCoord2TexPage.elem.x;
		v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

		vert2 = pParam->vert3;
		color2 = pParam->color3;
		x2 = pParam->vert3.x;
		y2 = pParam->vert3.y;
		u2 = pParam->texCoord3.elem.x;
		v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

		vert3 = pParam->vert4;
		color3 = pParam->color4;
		x3 = pParam->vert4.x;
		y3 = pParam->vert4.y;
		u3 = pParam->texCoord4.elem.x;
		v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord4.elem.y;
		drawTexTriangle();
	}
}

void gpu_t::drawTexPolygon4Bit(param_t* pParam) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (pParam->texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (pParam->texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	clut_t clut = pParam->texCoord1Palette.elem.attribute;

	const uint8_t vertexCount = pParam->verticesNumber;
	const shadingType_t shadingType = pParam->shadingType;
	const transpanencyType_t transparencyType = pParam->transpanencyType;

	vertex_t vert1 = pParam->vert1;
	psxColor24_t color1 = pParam->commandColor1;
	int x1 = pParam->vert1.x;
	int y1 = pParam->vert1.y;
	int u1 = pParam->texCoord1Palette.elem.x;
	int v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord1Palette.elem.y;

	vertex_t vert2 = pParam->vert2;
	psxColor24_t color2 = pParam->color2;
	int x2 = pParam->vert2.x;
	int y2 = pParam->vert2.y;
	int u2 = pParam->texCoord2TexPage.elem.x;
	int v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

	vertex_t vert3 = pParam->vert3;
	psxColor24_t color3 = pParam->color3;
	int x3 = pParam->vert3.x;
	int y3 = pParam->vert3.y;
	int u3 = pParam->texCoord3.elem.x;
	int v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

	psxColor24_t blendingColor = pParam->commandColor1;
	psxColor16_t frontPixelColor;

	auto drawTexTriangle = [&]() {
		if (y2 < y1) {
			swap(y1, y2);
			swap(x1, x2);
			swap(u1, u2);
			swap(v1, v2);
		}

		if (y3 < y1) {
			swap(y1, y3);
			swap(x1, x3);
			swap(u1, u3);
			swap(v1, v3);
		}

		if (y3 < y2) {
			swap(y2, y3);
			swap(x2, x3);
			swap(u2, u3);
			swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		int dv1 = v2 - v1;
		int du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;

		float tex_u, tex_v;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);

		if (dy1) {
			for (int i = y1; i <= y2; i++) {
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + tex_u / 4, tex_v);
					readVram(tex_p, data, true);
					vertex_t pClut;
					pClut.x = (clut.elem.xCoord << 4) + ((data >> (((uint16_t)tex_u & 0x3) << 2)) & 0xf);
					pClut.y = clut.elem.yCoord;
					readVram(pClut, frontPixelColor.data, true);

					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
				}


			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				int tex_su = u2 + (float)(i - y2) * du1_step;
				int tex_sv = v2 + (float)(i - y2) * dv1_step;

				int tex_eu = u1 + (float)(i - y1) * du2_step;
				int tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx) {
					swap(ax, bx);
					swap(tex_su, tex_eu);
					swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					uint16_t data;
					vertex_t tex_p((gpuStat.elem.texPageXBase << 6) + tex_u / 4, tex_v);
					readVram(tex_p, data, true);
					vertex_t pClut;
					pClut.x = (clut.elem.xCoord << 4) + ((data >> (((uint16_t)tex_u & 0x3) << 2)) & 0xf);
					pClut.y = clut.elem.yCoord;
					readVram(pClut, frontPixelColor.data, true);
					vertex_t p(j, i);
					if (frontPixelColor.data) {
						if (pParam->shadingType == GOURAUD_SHADING) {
							float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
							float e12 = edgeFunc(vert1, vert2, p);
							float e23 = edgeFunc(vert2, vert3, p);
							float e13 = edgeFunc(vert1, vert3, p);
							float lamda1 = abs(e23 / 2.f / P123);
							float lamda2 = abs(e13 / 2.f / P123);
							float lamda3 = abs(e12 / 2.f / P123);
							blendingColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
							blendingColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
							blendingColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
						}

						if (pParam->shadingType == FLAT_SHADING)
							blendingColor = pParam->commandColor1;

						if (pParam->modulationType == TEXTURE_BLENDING) {
							float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
							float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
							float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
							frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
							frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
							frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
						}

						if ((pParam->transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
							calculateTransparency(p, frontPixelColor);


						writeVram(p, frontPixelColor.data);
					}

					t += tstep;
				}
			}
		}
		};

	drawTexTriangle();
	if (vertexCount == 4) {
		vert1 = pParam->vert2;
		color1 = pParam->color2;
		x1 = pParam->vert2.x;
		y1 = pParam->vert2.y;
		u1 = pParam->texCoord2TexPage.elem.x;
		v1 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord2TexPage.elem.y;

		vert2 = pParam->vert3;
		color2 = pParam->color3;
		x2 = pParam->vert3.x;
		y2 = pParam->vert3.y;
		u2 = pParam->texCoord3.elem.x;
		v2 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord3.elem.y;

		vert3 = pParam->vert4;
		color3 = pParam->color4;
		x3 = pParam->vert4.x;
		y3 = pParam->vert4.y;
		u3 = pParam->texCoord4.elem.x;
		v3 = (gpuStat.elem.texPageYBase << 8) + pParam->texCoord4.elem.y;
		drawTexTriangle();
	}
}

void gpu_t::drawMonochromeRect(param_t param) {
	for (int16_t y = param.vert1.y; y < param.vert4.y; y++)
		for (int16_t x = param.vert1.x; x < param.vert4.x; x++) {
			vertex_t p(x, y);

			psxColor16_t frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				calculateTransparency(p, frontPixelColor);

			writeVram(p, frontPixelColor.data);
		}
}

void gpu_t::drawLine(param_t param) {

	psxColor24_t color1 = param.commandColor1;
	int x1 = param.vert1.x;
	int y1 = param.vert1.y;

	psxColor24_t color2 = param.color2;
	int x2 = param.vert2.x;
	int y2 = param.vert2.y;

	psxColor16_t frontPixelColor;


	if (y2 < y1) {
		swap(y1, y2);
		swap(x1, x2);
		//swap(color1.data, color2.data);
	}

	int dy1 = y2 - y1;
	int dx1 = x2 - x1;

	float k = 0;

	if (dy1) k = dx1 / (float)abs(dy1);
	else k = std::numeric_limits<float>::max();

	if ((k >= 1.0f) || (k <= -1.0f)) {
		int ax = x1;
		int bx = x2;

		if (bx < ax)
			swap(ax, bx);

		for (int i = ax; i <= bx; i++) {
			vertex_t p(i, round(y1 + (float)(i - x1) * 1.0f / k));

			if (param.shadingType == GOURAUD_SHADING) {
				float lamdaAbs = sqrt(pow((param.vert1.x - param.vert2.x), 2) + pow((param.vert1.y - param.vert2.y), 2));
				float lamdaPointAbs = sqrt(pow((param.vert1.x - p.x), 2) + pow((param.vert1.y - p.y), 2));
				float lamda1 = 1 - lamdaPointAbs / lamdaAbs;
				float lamda2 = 1 - lamda1;

				psxColor24_t calculatedColor;

				calculatedColor.component.red = (((float)color1.component.red * lamda1 + (float)color2.component.red * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.red * lamda1 + (float)color2.component.red * lamda2;
				calculatedColor.component.green = (((float)color1.component.green * lamda1 + (float)color2.component.green * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.green * lamda1 + (float)color2.component.green * lamda2;
				calculatedColor.component.blue = (((float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2;

				frontPixelColor = format24to16Color(calculatedColor.data);
			}

			if (param.shadingType == FLAT_SHADING)
				frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				calculateTransparency(p, frontPixelColor);

			writeVram(p, frontPixelColor.data);
		}
	}
	else if ((k < 1.0f) && (k > -1.0f)) {
		for (int i = y1; i <= y2; i++) {
			vertex_t p(round(x1 + (float)(i - y1) * k), i);

			if (param.shadingType == GOURAUD_SHADING) {
				float lamdaAbs = sqrt(pow((param.vert1.x - param.vert2.x), 2) + pow((param.vert1.y - param.vert2.y), 2));
				float lamdaPointAbs = sqrt(pow((param.vert1.x - p.x), 2) + pow((param.vert1.y - p.y), 2));
				float lamda1 = 1 - lamdaPointAbs / lamdaAbs;
				float lamda2 = 1 - lamda1;

				psxColor24_t calculatedColor;

				calculatedColor.component.red = (((float)color1.component.red * lamda1 + (float)color2.component.red * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.red * lamda1 + (float)color2.component.red * lamda2;
				calculatedColor.component.green = (((float)color1.component.green * lamda1 + (float)color2.component.green * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.green * lamda1 + (float)color2.component.green * lamda2;
				calculatedColor.component.blue = (((float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2) > (float)0xff) ? 0xff :
					(float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2;

				frontPixelColor = format24to16Color(calculatedColor.data);
			}

			if (param.shadingType == FLAT_SHADING)
				frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				calculateTransparency(p, frontPixelColor);

			writeVram(p, frontPixelColor.data);
		}
	}
}

void gpu_t::drawTexRect15Bit(param_t param) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };
	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	uint16_t data;
	vertex_t pTex;
	vertex_t p;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	int16_t xTexMax = param.texCoord4.elem.x;
	int16_t yTexMax = param.texCoord4.elem.y;

	if (param.primitiveType == RECT) {
		xTexMax = param.texCoord1Palette.elem.x + (param.size_y_x & 0xffff);
		yTexMax = param.texCoord1Palette.elem.y + ((param.size_y_x >> 16) & 0xffff);
	}

	float ratioX = ((float)(xTexMax - xTexMin) / (float)(xMax - xMin));
	float ratioY = ((float)(yTexMax - yTexMin) / (float)(yMax - yMin));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	psxColor24_t blendingColor = param.commandColor1;
	psxColor16_t frontPixelColor;

	/*if (param.primitiveType == POLYGON) {
		xMax -= 1;
		yMax -= 1;
	}*/

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += (yTexMin + (int16_t)((float)y * ratioY)) & 255;
		pTex.y = (pTex.y & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		if (param.bFlipVertically)
			p.y = yMax - 1 - y;
		else
			p.y = yMin + y;

		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += (xTexMin + ((int16_t)((float)x * ratioX))) & 255;;
			pTex.x = (pTex.x & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

			if (param.bFlipHorizontally)
				p.x = xMax - 1 - x;
			else
				p.x = xMin + x;

			readVram(pTex, texData, true);
			frontPixelColor.data = texData;

			if (frontPixelColor.data) {
				if (param.modulationType == TEXTURE_BLENDING) {
					float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
					float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
					float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
					frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
					frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
					frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
				}

				if ((param.transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
					calculateTransparency(p, frontPixelColor);
				writeVram(p, frontPixelColor.data);
			}
		}
	}
}

void gpu_t::drawTexRect8Bit(param_t param) {
	clut_t clut = param.texCoord1Palette.elem.attribute;
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	psxColor16_t frontPixelColor;
	vertex_t pTex;
	vertex_t p;
	vertex_t pClut;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	int16_t xTexMax = param.texCoord4.elem.x;
	int16_t yTexMax = param.texCoord4.elem.y;

	if (param.primitiveType == RECT) {
		xTexMax = param.texCoord1Palette.elem.x + (param.size_y_x & 0xffff);
		yTexMax = param.texCoord1Palette.elem.y + ((param.size_y_x >> 16) & 0xffff);
	}

	float ratioX = ((float)(xTexMax - xTexMin) / (float)(xMax - xMin));
	float ratioY = ((float)(yTexMax - yTexMin) / (float)(yMax - yMin));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	psxColor24_t blendingColor = param.commandColor1;

	/*if (param.primitiveType == POLYGON) {
		xMax -= 1;
		yMax -= 1;
	}*/

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += (yTexMin + (int16_t)((float)y * ratioY)) & 255;
		pTex.y = (pTex.y & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		if (param.bFlipVertically)
			p.y = yMax - 1 - y;
		else
			p.y = yMin + y;

		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += ((xTexMin >> 1) + ((int16_t)((float)x * ratioX) >> 1)) & 127;;
			pTex.x = (pTex.x & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

			if (param.bFlipHorizontally)
				p.x = xMax - 1 - x;
			else
				p.x = xMin + x;

			readVram(pTex, texData, true);
			pClut.x = clut.elem.xCoord * 16 +
				(texData >> (((int16_t)((float)x * ratioX) & 0x1) * 8) & 0xff);
			pClut.y = clut.elem.yCoord;
			readVram(pClut, frontPixelColor.data, true);

			if (frontPixelColor.data) {
				if (param.modulationType == TEXTURE_BLENDING) {
					float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
					float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
					float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
					frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
					frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
					frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
				}

				if ((param.transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
					calculateTransparency(p, frontPixelColor);
				writeVram(p, frontPixelColor.data);
			}
		}
	}
}

void gpu_t::drawTexRect4Bit(param_t param) {
	clut_t clut = param.texCoord1Palette.elem.attribute;
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	psxColor16_t frontPixelColor;
	vertex_t pTex;
	vertex_t p;
	vertex_t pClut;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	int16_t xTexMax = param.texCoord4.elem.x;
	int16_t yTexMax = param.texCoord4.elem.y;

	if (param.primitiveType == RECT) {
		xTexMax = param.texCoord1Palette.elem.x + (param.size_y_x & 0xffff);
		yTexMax = param.texCoord1Palette.elem.y + ((param.size_y_x >> 16) & 0xffff);
	}

	float ratioX = ((float)(xTexMax - xTexMin) / (float)(xMax - xMin));
	float ratioY = ((float)(yTexMax - yTexMin) / (float)(yMax - yMin));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	psxColor24_t blendingColor = param.commandColor1;

	//if (param.primitiveType == POLYGON) {
	//	//xMax -= 1;
	//	//yMax -= 1;
	//	xMax = xMax;
	//}

	//if (param.bFlipHorizontally) {//??
	//	xTexMin += konstant;
	//}

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += (yTexMin + (int16_t)((float)y * ratioY)) & 255;
		pTex.y = (pTex.y & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		if (param.bFlipVertically)
			p.y = yMax - 1 - y;
		else
			p.y = yMin + y;

		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += ((xTexMin >> 2) + ((int16_t)((float)x * ratioX) >> 2)) & 63;;
			pTex.x = (pTex.x & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

			if (param.bFlipHorizontally)
				p.x = xMax - x + 1;
			else
				p.x = xMin + x;

			readVram(pTex, texData, true);
			pClut.x = (clut.elem.xCoord << 4) +
				(texData >> (((int16_t)((float)x * ratioX) & 0x3) << 2) & 0xf);
			pClut.y = clut.elem.yCoord;
			readVram(pClut, frontPixelColor.data, true);

			if (frontPixelColor.data) {
				if (param.modulationType == TEXTURE_BLENDING) {
					float red = ((float)blendingColor.component.red / 128.) * ((float)frontPixelColor.component.red * 8.);
					float green = ((float)blendingColor.component.green / 128.) * ((float)frontPixelColor.component.green * 8.);
					float blue = ((float)blendingColor.component.blue / 128.) * ((float)frontPixelColor.component.blue * 8.);
					frontPixelColor.component.red = ((uint16_t)red / 8) > 0x1f ? 0x1f : (uint16_t)red / 8;
					frontPixelColor.component.green = ((uint16_t)green / 8) > 0x1f ? 0x1f : (uint16_t)green / 8;
					frontPixelColor.component.blue = ((uint16_t)blue / 8) > 0x1f ? 0x1f : (uint16_t)blue / 8;
				}

				if ((param.transpanencyType == SEMI_TRANSPARENT) && (frontPixelColor.component.semiTransparencyFlag))
					calculateTransparency(p, frontPixelColor);
				writeVram(p, frontPixelColor.data);
			}
		}
	}
}

void gpu_t::copyRectCpuVram(const uint32_t dest_coord, const uint32_t width_height) {
	const uint32_t x1 = (dest_coord << 16 >> 16) & 0x3ff;
	uint32_t x2 = x1 + (width_height << 16 >> 16);
	const uint32_t y1 = (dest_coord >> 16) & 0x1ff;
	uint32_t y2 = y1 + (width_height >> 16);

	std::vector<uint16_t> buffer(fifo.size() << 1);
	for (uint32_t i = 0; i < (buffer.size() >> 1); i++) {
		buffer[i * 2] = fifo[0];
		buffer[i * 2 + 1] = fifo[0] >> 16;
		fifo.erase(fifo.begin());
	}

	uint32_t x;
	uint32_t y = y1;
	do {
		x = x1;
		do {
			vertex_t position(x, y);
			writeVram(position, buffer[0], true);
			buffer.erase(buffer.begin());
			if (buffer.size() == 0)
				return;
			x++;
		} while (x < x2);
		y++;
	} while (y < y2);

	if (!buffer.empty() && !(((width_height << 16 >> 16) * (width_height >> 16)) & 0x1)) {
		std::cout << "[GPU] EMULATION PAUSED! gp0 command 0xa0... buffer not empty 0x" << std::hex << buffer.size() << std::endl;
		g_emulationPaused = true;
	}
}

void gpu_t::copyRectVramCpu(const uint32_t source_coord, const uint32_t width_height) {
	const uint16_t x1 = source_coord << 16 >> 16;
	const uint16_t x2 = x1 + (width_height << 16 >> 16);
	const uint16_t y1 = source_coord >> 16;
	const uint16_t y2 = y1 + (width_height >> 16);

	uint16_t data;
	std::vector<uint16_t> buffer(fifo.size() * 2);

	uint16_t x;
	uint16_t y = y1;
	do {
		x = x1;
		do {
			vertex_t position(x, y);
			readVram(position, data, true);
			buffer.push_back(data);
			x++;
		} while (x < x2);
		y++;
	} while (y < y2);

	uint32_t temp;

	if (buffer.size() & 1)
		buffer.push_back(0);

	for (int i = 0; i < buffer.size(); i += 2) {
		temp = buffer[i];
		temp |= (uint32_t)buffer[i + 1] << 16;
		fifo.push_back(temp);
	}
}

void gpu_t::copyRectVramVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight) {
	const uint16_t sourceX1 = sourceCoord << 16 >> 16;
	const uint16_t sourceY1 = sourceCoord >> 16;

	const uint16_t destX1 = destCoord << 16 >> 16;
	const uint16_t destY1 = destCoord >> 16;

	const uint16_t width = widthHeight << 16 >> 16;
	const uint16_t height = widthHeight >> 16;


	uint16_t data;

	uint16_t x;
	uint16_t y = 0;
	do {
		x = 0;
		do {
			vertex_t sourcePosition(sourceX1 + x, sourceY1 + y);
			readVram(sourcePosition, data, true);
			vertex_t destPosition(destX1 + x, destY1 + y);
			writeVram(destPosition, data, true);
			x++;
		} while (x < width);
		y++;
	} while (y < height);
}

void gpu_t::fillRectVram(param_t param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert4.x;
	int16_t y2 = param.vert4.y;

	int16_t x = x1;
	int16_t y = y1;

	uint16_t data = format24to16Color(param.commandColor1);

	uint32_t oldSetMaskBitWhileDrawing = gpuStat.elem.setMaskBitWhileDrawing;
	gpuStat.elem.setMaskBitWhileDrawing = 0;
	uint32_t oldDrawPixels = gpuStat.elem.drawPixels;
	gpuStat.elem.drawPixels = 0;

	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			vertex_t p(x, y);
			writeVram(p, data, true);
		}
	}

	gpuStat.elem.setMaskBitWhileDrawing = oldSetMaskBitWhileDrawing;
	gpuStat.elem.drawPixels = oldDrawPixels;
}

void gpu_t::decodeExecuteGp0Command(const uint32_t& commandColor) {
	param_t param;

	switch (commandColor >> 29) {
	case 0b000: //Misc commands
		if ((gp0Command >> 24) == 0x02) {
			uint32_t size_x_y = fifo.back();
			fifo.pop_back();
			param.vert1 = (vertex_t)fifo.back();
			fifo.pop_back();
			param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
			param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
			param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
			param.commandColor1 = gp0Command;

			fillRectVram(param);
		}
		else if ((gp0Command >> 24) == 0x01) {
		}
		else {
			if (gp0Command >> 24 == 0x1f) {
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x" << std::hex << commandColor << " " <<
					gp0Lookup[(commandColor >> 24)].name << "\n";
				g_emulationPaused = true;
			}
		}
		break;
	case 0b001: //Polygon primitive
		param.primitiveType = POLYGON;

		param.shadingType = (commandColor & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.verticesNumber = (commandColor & 0x08000000) ? 4 : 3;
		param.bTextured = (commandColor & 0x04000000) ? true : false;
		param.transpanencyType = (commandColor & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (commandColor & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		param.vert4 = (0, 0);
		if (param.verticesNumber == 4) {
			if (param.bTextured) {
				param.texCoord4 = fifo.back();
				fifo.pop_back();
			}

			param.vert4 = fifo.back();
			fifo.pop_back();

			if (param.shadingType == GOURAUD_SHADING) {
				param.color4 = fifo.back();
				fifo.pop_back();
			}
		}

		if (param.bTextured) {
			param.texCoord3 = fifo.back();
			fifo.pop_back();
		}

		param.vert3 = fifo.back();
		fifo.pop_back();

		if (param.shadingType == GOURAUD_SHADING) {
			param.color3 = fifo.back();
			fifo.pop_back();
		}

		if (param.bTextured) {
			param.texCoord2TexPage = fifo.back();
			fifo.pop_back();
		}

		param.vert2 = fifo.back();
		fifo.pop_back();

		if (param.shadingType == GOURAUD_SHADING) {
			param.color2 = fifo.back();
			fifo.pop_back();
		}

		if (param.bTextured) {
			param.texCoord1Palette = fifo.back();
			fifo.pop_back();
		}

		param.vert1 = fifo.back();
		fifo.pop_back();

		param.commandColor1 = commandColor;

		if (param.bTextured) {
			gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
			gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
		}

		if (param.shadingType == FLAT_SHADING) {
			param.color2 = param.commandColor1 & 0x00ffffff;
			param.color3 = param.commandColor1 & 0x00ffffff;
			if (param.verticesNumber == 4)
				param.color4 = param.commandColor1 & 0x00ffffff;
		}

		//if ((param.bTextured)/* && (param.verticesNumber == 4)*/) {
		//	/*if (param.vert1.x > param.vert2.x)
		//		param.bFlipHorizontally = true;
		//	if (param.vert1.y > param.vert4.y)
		//		param.bFlipVertically = true;*/

		//	switch (gpuStat.elem.texPageColors) {
		//	case 0:
		//		drawTexPolygon4Bit(&param);
		//		break;
		//	case 1:
		//		drawTexPolygon8Bit(&param);
		//		break;
		//	case 2:
		//		drawTexPolygon15Bit(&param);
		//		break;
		//	case 3:
		//		throw std::runtime_error("texture page color mode RESERVED!");
		//		break;
		//	}
		//}
		//else
		//	drawPolygon(&param);

		if (param.bTextured)
			drawTexPolygon(&param);
		else
			drawPolygon(&param);

		break;

	case 0b010: //Line primitive
		param.primitiveType = LINE;

		param.shadingType = (commandColor & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.bPolyline = (commandColor & 0x08000000) ? true : false;
		param.transpanencyType = (commandColor & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;

		if (param.bPolyline)
			fifo.pop_back(); // pop 0x50005000

		param.commandColor1 = commandColor;

		if (param.shadingType == FLAT_SHADING) {
			for (int i = 0; i < fifo.size() - 1; i++) {
				param.vert1 = fifo[i];
				param.vert2 = fifo[i + 1];

				drawLine(param);
			}
		}

		if (param.shadingType == GOURAUD_SHADING) {
			for (int i = 0; i < fifo.size() - 2; i += 2) {
				param.vert1 = fifo[i];
				param.color2 = fifo[i + 1];
				param.vert2 = fifo[i + 2];

				if (fifo[0] == fifo[i + 2]) //!!?????
					param.color2 = commandColor & 0x00ffffff;

				drawLine(param);

				param.commandColor1 = (param.commandColor1 & 0xff000000) | param.color2.data;

			}
		}

		fifo.clear();

		break;

	case 0b011: { //Rectangle primitive
		param.primitiveType = RECT;

		param.rectSize = commandColor << 3 >> 30;
		param.bTextured = (commandColor & 0x04000000) ? true : false;
		param.transpanencyType = (commandColor & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (commandColor & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		switch (param.rectSize) {
		case 0b00:
			param.size_y_x = fifo.back();
			fifo.pop_back();
			break;
		case 0b01:
			param.size_y_x = 0x00010001;
			break;
		case 0b10:
			param.size_y_x = 0x00080008;
			break;
		case 0b11:
			param.size_y_x = 0x00100010;
			break;
		}

		if (param.bTextured) {
			param.texCoord1Palette = (texcoordType_t)fifo.back();
			fifo.pop_back();


			param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
			param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
				param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
		}

		param.vert1 = (vertex_t)fifo.back();
		fifo.pop_back();

		param.vert2 = param.vert1 + vertex_t(param.size_y_x & 0xffff, 0);
		param.vert3 = param.vert1 + vertex_t(0, (param.size_y_x >> 16) & 0xffff);
		param.vert4 = param.vert1 + vertex_t(param.size_y_x & 0xffff, (param.size_y_x >> 16) & 0xffff);

		param.commandColor1 = gp0Command;

		if (param.bTextured)
			switch (gpuStat.elem.texPageColors) {
			case 0:
				drawTexRect4Bit(param);
				break;
			case 1:
				drawTexRect8Bit(param);
				break;
			case 2:
				drawTexRect15Bit(param);
				break;
			case 3:
				throw std::runtime_error("texture page color mode RESERVED!");
				break;
			}
		else
			drawMonochromeRect(param);

		break;
	}
	case 0b100: { //VRAM-to-VRAM blit
		uint32_t widthHeight = fifo.back();
		fifo.pop_back();
		uint32_t destCoord = fifo.back();
		fifo.pop_back();
		uint32_t sourceCoord = fifo.back();
		fifo.pop_back();
		copyRectVramVram(sourceCoord, destCoord, widthHeight);
		break;
	}
	case 0b101: //CPU-to-VRAM blit
		if (bCollectCopyCpuVramParam) {
			bCollectCopyCpuVramParam = false;
			widthHeight = fifo.back();
			fifo.pop_back();
			destCoord = fifo.back();
			fifo.pop_back();
			//collect = ((((width_height >> 16) - 1) & 0x3ff) + 1) * ((((width_height << 16 >> 16) - 1) & 0x3ff) + 1) / 2 + (width_height & 0x1);
			collectCount = (widthHeight >> 16) * (widthHeight << 16 >> 16) / 2;
			if ((collectCount * 2) < ((widthHeight >> 16) * (widthHeight << 16 >> 16)))
				collectCount++;
		}
		else {
			copyRectCpuVram(destCoord, widthHeight);
		}
		break;
	case 0b110: //VRAM-to-CPU blit
		widthHeight = fifo.back();
		fifo.pop_back();
		sourceCoord = fifo.back();
		fifo.pop_back();
		copyRectVramCpu(sourceCoord, widthHeight);
		bTransferCommand = true;
		break;
	case 0b111: //Environment commands
		switch (commandColor >> 24) {
		case 0xe1:
			gpuStat.data &= 0xffff7800;
			gpuStat.data |= commandColor & 0x000087ff;
			gpuStat.elem.texDis = commandColor & 0x00008000 ? 1 : 0;
			break;
		case 0xe2:
			texWinMask.x = commandColor << 27 >> 27;
			texWinMask.y = commandColor << 22 >> 27;
			texWinOffset.x = commandColor << 17 >> 27;
			texWinOffset.y = commandColor << 12 >> 27;
			gp1TextureWindowSetting = commandColor & 0x000fffff;
			break;
		case 0xe3:
			drawAreaTopLeft.x = commandColor << 22 >> 22;
			drawAreaTopLeft.y = commandColor << 13 >> 23;
			gp1TopLeft = commandColor & 0x0007ffff;
			break;
		case 0xe4:
			drawAreaBottomRight.x = commandColor << 22 >> 22;
			drawAreaBottomRight.y = commandColor << 13 >> 23;
			gp1BottomRight = commandColor & 0x0007ffff;
			break;
		case 0xe5:
			drawOffset.x = (commandColor & 0x7ff) << 5;
			drawOffset.y = ((commandColor >> 11) & 0x7ff) << 5;
			gp1DrawOffset = commandColor & 0x003fffff;;
			break;
		case 0xe6:
			gpuStat.elem.setMaskBitWhileDrawing = commandColor & 0x1;
			gpuStat.elem.drawPixels = (commandColor & 0x2 ? 1 : 0);
			break;
		}
	}
}