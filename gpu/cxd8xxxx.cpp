#include "cxd85xxx.h"
#include "../bus/bus_interface.h"

#include "math.h"
#include "limits"

#include "rasterize.hh"

#include <array>
#include <vector>
#include <map>

class Slope
{
	float begin, step;
public:
	Slope() {}
	Slope(float from, float to, float num_steps)
	{
		float inv_step = 1.f / num_steps;
		begin = from;                   // Begin here
		step = (to - from) * inv_step; // Stepsize = (end-begin) / num_steps
	}
	float get() const { return begin; }
	void advance(float n = 1) { begin += step * n; }
};

template<typename PlotFunc, typename T, std::size_t Size = std::tuple_size_v<T>>
void DrawPolygonInterpolated(
	T p0,
	T p1,
	T p2,
	PlotFunc&& Plot)
{
	using SlopeData = std::array<Slope, Size - 1>; // All input units except y

	RasterizeTriangle(
		&p0, &p1, &p2,
		// GetXY: Retrieve std::tuple<int,int> or std::array<int,2> from a PointType
		[&](const auto& p) { return std::tuple{ int(std::get<0>(p)), int(std::get<1>(p)) }; },
		// Slope generator
		[&](const T* from, const T* to, int num_steps)
		{
			SlopeData result;
			// Number of steps = number of scanlines
			// Retrieve X coordinates for begin and end
			result[0] = Slope(std::get<0>(*from), std::get<0>(*to), num_steps);
			// Take the rest of the props
			[&] <std::size_t...p>(std::index_sequence<p...>)
			{
				// Retrieve begin to end for each prop. The +2 skips x and y.
				((result[p + 1] = Slope(std::get<p + 2>(*from), std::get<p + 2>(*to), num_steps)), ...);
			}
			(std::make_index_sequence<Size - 2>{});
			return result;
		},
		// Scanline function
		[&](int y, SlopeData& left, SlopeData& right)
		{
			float leftx = left[0].get(), endx = right[0].get();
			auto intleftx = std::floor(leftx + .5f), rounding_error = intleftx - leftx + .5f;

			// Number of steps = number of pixels on this scanline = endx-x
			std::array<Slope, Size - 2> props;
			for (unsigned p = 0; p < Size - 2; ++p)
			{
				props[p] = Slope(left[p + 1].get(), right[p + 1].get(), endx - leftx);
				props[p].advance(rounding_error);
			}

			for (int x = intleftx, e = endx + .5f; x < e; ++x) // Render each pixel
			{
				std::apply([&](auto... args) { Plot(x, y, args.get()...); }, props);

				// After each pixel, update the props by their step-sizes
				for (auto& prop : props) prop.advance();
			}

			// After the scanline is drawn, update the X coordinate and props on both sides
			for (auto& slope : left) slope.advance();
			for (auto& slope : right) slope.advance();
		});
}


extern bool g_emulationPaused;
extern bool g_GpuCommandsLogged;
extern int g_mousePossitionX;
extern int g_mousePossitionY;
extern bool g_logPixel;

#undef DISABLE_SCALING
#define DISABLE_SCALING 0

static uint16_t signed11bit(uint16_t val) {
	return ((int16_t)(val << 5)) >> 5;
}

gpu_t::gpu_t() {
	gpuStat.data = 0x14000000;

	fifo.resize(size_x * size_y);

	outputFifo.resize(size_x * size_y);

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
}

void gpu_t::readGpu32(const uint32_t& addr, uint32_t& data) {
	data = 0;

	if (addr == 0x1f801810) {
		if (gpuStat.elem.readyVramCpu) {
			data = outputFifo.getNext();
			if (outputFifo.empty()) {
				gpuStat.elem.readyVramCpu = 0;
				outputFifo.reset();
			}
			return;
		}

		data = gp1Result;
		return;
	}

	if (addr == 0x1f801814) {
		switch (gpuStat.elem.dmaDir) {
		case 0:
			gpuStat.elem.dataRequest = 0;
			break;
		case 1:
			if (fifoIndex >= 2)
				gpuStat.elem.dataRequest = 0;
			else
				gpuStat.elem.dataRequest = 1;
			break;
		case 2:
			gpuStat.elem.dataRequest = gpuStat.elem.readyDma;
			break;
		case 3:
			gpuStat.elem.dataRequest = gpuStat.elem.readyVramCpu;
			break;
		}


		data = gpuStat.data;
	}
}

void gpu_t::writeGpu32(const uint32_t& addr, const uint32_t& data) {
	if (addr == 0x1f801810) {
		/*	if (g_GpuCommandsLogged)
				std::cout << std::hex << "0x" << data << std::endl;*/

		if (collectCount) {
			fifo.pushBack(data);
			collectCount--;
			if ((collectCount == 0) && (bCollectPolylineParam))
				if ((data & 0xf000f000) == 0x50005000)
					bCollectPolylineParam = false;
				else
					collectCount = 1;
		}
		else {
			gp0Command = data;
			if (!fifo.empty()) {
				std::cout << "[GPU] EMULATION PAUSED! fifo not empty 0x" << std::hex << fifo.size() << std::endl;
				g_emulationPaused = true;
			}
			fifo.reset();

			collectCount = gp0Lookup[gp0Command >> 24].collectCount - 1;


			if (((gp0Command >> 24) >= 0xa0) && ((gp0Command >> 24) < 0xc0))
				bCollectCopyCpuVramParam = true;
			else if ((gp0Command >> 29 == 0b010) && (gp0Command & 0x08000000))
				bCollectPolylineParam = true;
		}

		if (collectCount == 0) {

			if (g_GpuCommandsLogged) {
				std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << gp0Command << " " << gp0Lookup[gp0Command >> 24].name << std::endl;
				std::cout << "~[GPU] memAdrTemp: 0x" << std::hex << pBus->dma.dma2.memAddrTemp << std::endl;
			}

			decodeExecuteGp0Command(gp0Command);

			if (fifoIndex >= 2) {
				if (gpuStat.elem.dmaDir == 2)
					if (pBus->dma.dma2.channelControl.reg.startBusy) {
						pBus->dma.dma2.channelControl.reg.startBusy = 0;
						bWasActiveDMA = true;
						pBus->dma.isCpuStopped = false;
						gpuStat.elem.readyDma = 0;

						if (g_GpuCommandsLogged)
							std::cout << "~[GPU] FIFO FULL" << std::hex << std::endl;
					}

				gpuStat.elem.readyCommand = 0;
			}

			//??
			/*if (pBus->dma.dma2.channelControl.reg.startBusy)
				fifoCount++;*/
		}
		return;
	}

	if (addr == 0x1f801814) {
		gp1Command = data;

		if (g_GpuCommandsLogged)
			std::cout << "~[GPU] GPU1 command executed: 0x" << std::hex << gp1Command << std::endl;

		decodeExecuteGp1Command(gp1Command);
	}
}

void gpu_t::videoClock() {
	if (cpuScanlineClocks == cpuScanlineClocksNumber) {
		cpuScanlineClocks = 0;
		scanlineCount++;

		//Hblank clocking timer 1
		pBus->timer1.clock(_HBLANK_CLOCK);
		pBus->timer0.clock(_HBLANK_CLOCK);

		if (gpuStat.elem.verticalInterlace)
			if (!gpuStat.elem.verticalRes)
				gpuStat.elem.drawingEvenOddLines = (scanlineCount <= screenY2) ? scanlineCount & 0x1 : 0;

		if (scanlineCount == screenY2) {
			//fifoCount = 0;
			//gpuStat.elem.readyDma = 1;
			//gpuStat.elem.readyCommand = 1;

			//V-Sync interrupt is naturally edge trigered
			pBus->pCpu->cp0.interruptHandler(_IRQ_VBLANK_); //causes troubles to boot some games


			/*if (pBus->spu.spuControlRegister.elem.irq9Enable) {
				if ((pBus->spu.irqAddress < 0x1000) ||
					(pBus->spu.irqAddress >= pBus->spu.reverbCurrentAddress)) {
					pBus->pCpu->cp0.interruptHandler(_IRQ_SPU);
					pBus->spu.spuStatusRegister.elem.irq9Flag = 1;
				}
			}*/

			//Vblank synchronization timer 1
			pBus->timer1.clock(_VBLANK_CLOCK);
		}
		else if (scanlineCount == scanlineNumber) {
			scanlineCount = -1;

			//pBus->pCpu->cp0.interruptHandler(_IRQ_VBLANK_);

			if (gpuStat.elem.verticalInterlace)
				if (gpuStat.elem.verticalRes)
					gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;
		}
	}

	if ((cpuClocks % dotClockDevider) == 0)
		pBus->timer0.clock(_DOT_CLOCK);

	//if ((cpuClocks % (44100 * 0x300 / 4)) == 0) { //Rayman 2 - The Great Escape
	//	if (pBus->spu.spuControlRegister.elem.irq9Enable) {
	//		pBus->pCpu->cp0.interruptHandler(_IRQ_SPU);
	//		pBus->spu.spuStatusRegister.elem.irq9Flag = 1;
	//	}
	//}

	if (cpuClocks == CPU_CLOCK)
		cpuClocks = 0;
	else
		cpuClocks++;


	cpuScanlineClocks++;

	if (fifoIndex < 2) {
		if (gpuStat.elem.dmaDir == 2)
			if (bWasActiveDMA) {
				bWasActiveDMA = false;
				pBus->dma.dma2.channelControl.reg.startBusy = 1;
				pBus->dma.isCpuStopped = false;
				gpuStat.elem.readyDma = 1;
			}
		gpuStat.elem.readyCommand = 1;
	}

	if (fifoIndex > 0) {
		if (fifoClocks[0] == 0) {
			fifoIndex--;
			fifoClocks[0] = fifoClocks[1];
			fifoClocks[1] = fifoClocks[2];
			fifoClocks[2] = fifoClocks[3];
			fifoClocks[3] = fifoClocks[4];
			fifoClocks[4] = 0;
		}
		else {
			fifoClocks[0] -= 0x1;
			if ((int32_t)fifoClocks[0] < 0)
				fifoClocks[0] = 0;
		}
	}

	/*if (fifoIndex) {
		fifoClocks[fifoIndex]--;
		if (fifoClocks[fifoIndex] == 0) {
			pBus->dma.dma2.channelControl.reg.startBusy = 1;
			gpuStat.elem.readyDma = 1;
			gpuStat.elem.readyCommand = 1;
			fifoIndex--;
		}
	}*/


	//fifoClocks++;
	//if (fifoCount >= 3071) {
	//	pBus->dma.dma2.channelControl.reg.startBusy = 0;
	//	pBus->dma.isCpuStopped = false;
	//	gpuStat.elem.readyDma = 0;
	//	gpuStat.elem.readyCommand = 0;
	//	//pBus->dma.dma2.channelControl.reg.startBusy = 1;
	//	//pBus->dma.isCpuStopped = true;
	//}

	//DMA ACCESING FIX!? //not working ok in all games
	//if (fifoCount >= 16) {
	//	pBus->dma.dma2.channelControl.reg.startBusy = 0;
	//	pBus->dma.isCpuStopped = false;
	//	gpuStat.elem.readyDma = 0;
	//	gpuStat.elem.readyCommand = 0;
	//}
}

void gpu_t::decodeExecuteGp0Command(const uint32_t& command) {

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
			param.primitiveType = FILL;

			param.commandColor1 = gp0Command;

			uint32_t pos = fifo.getNext();
			uint16_t xpos = pos & 0xffff;
			uint16_t ypos = pos >> 16;
			maskPosFill(xpos, ypos);
			param.vert1 = vertex_t(xpos, ypos);

			uint32_t size_x_y = fifo.getNext();
			uint16_t xsiz = size_x_y & 0xffff;
			uint16_t ysiz = size_x_y >> 16;
			maskSizFill(xsiz, ysiz);
			param.vert4 = param.vert1 + vertex_t(xsiz, ysiz);

			/*fifoClocks[fifoIndex] = xsiz * ysiz / 0x40;
			fifoIndex++;*/

			fillRectVram(param);
		}
		else if ((gp0Command >> 24) == 0x01) {
			//clar cache unhandled
		}
		else {
			if (gp0Command >> 24 == 0x1f) {
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x" << std::hex << command << " " <<
					gp0Lookup[(command >> 24)].name << "\n";
				g_emulationPaused = true;
			}
		}
		break;
	case 0b001: { //Polygon primitive
		param.primitiveType = POLYGON;

		param.shadingType = (command & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.verticesNumber = (command & 0x08000000) ? 4 : 3;
		param.bTextured = (command & 0x04000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (command & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		param.commandColor1 = command;

		param.vert1 = fifo.getNext();
		param.vert1.x = (param.vert1.x + drawOffset.x);
		param.vert1.y = (param.vert1.y + drawOffset.y);

		if (param.bTextured)
			param.texCoord1Palette = fifo.getNext();

		if (param.shadingType == GOURAUD_SHADING)
			param.color2 = fifo.getNext();

		param.vert2 = fifo.getNext();
		param.vert2.x = (param.vert2.x + drawOffset.x);
		param.vert2.y = (param.vert2.y + drawOffset.y);

		if (param.bTextured)
			param.texCoord2TexPage = fifo.getNext();

		if (param.shadingType == GOURAUD_SHADING)
			param.color3 = fifo.getNext();

		param.vert3 = fifo.getNext();
		param.vert3.x = (param.vert3.x + drawOffset.x);
		param.vert3.y = (param.vert3.y + drawOffset.y);

		if (param.bTextured)
			param.texCoord3 = fifo.getNext();

		param.vert4 = (0, 0);//??
		if (param.verticesNumber == 4) {
			if (param.shadingType == GOURAUD_SHADING)
				param.color4 = fifo.getNext();

			param.vert4 = fifo.getNext();
			param.vert4.x = (param.vert4.x + drawOffset.x);
			param.vert4.y = (param.vert4.y + drawOffset.y);

			if (param.bTextured)
				param.texCoord4 = fifo.getNext();
		}

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

		//int16_t vertx[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
		//int16_t verty[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

		//int16_t maxX = 0;
		//int16_t minX = 0;
		//int16_t maxY = 0;
		//int16_t minY = 0;

		//maxX = utils::max(vertx, param.verticesNumber);
		//minX = utils::min(vertx, param.verticesNumber);
		//maxY = utils::max(verty, param.verticesNumber);
		//minY = utils::min(verty, param.verticesNumber);

		//float k = 1.0f;
		//k += param.shadingType ? 0.25f : 0.0f;
		//k += param.bTextured ? 0.25f : 0.0f;
		//k += (param.transpanencyType == SEMI_TRANSPARENT) ? 0.25f : 0.0f;

		//if (param.bTextured) {
		//	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute & 0x7ff);
		//	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

		//	if (gpuStat.elem.texPageColors >= 2)
		//		fifoClocks[fifoIndex] = 0x100;
		//	else
		//		fifoClocks[fifoIndex] = abs(maxX - minX) * abs(maxY - minY) * k / (0x300);
		//}

		//std::cout << "FIFO clocks: " << std::dec << fifoClocks[fifoIndex] << std::endl;

		//fifoIndex++;

		if (param.bTextured)
			drawTexPolygon(&param);
		else
			drawPolygon(&param);
	}

			  break;

	case 0b010: //Line primitive
		param.primitiveType = LINE;

		param.shadingType = (command & 0x10000000) ? GOURAUD_SHADING : FLAT_SHADING;
		param.bPolyline = (command & 0x08000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;

		if (param.bPolyline)
			fifo.popBack(); // pop 0x50005000

		param.commandColor1 = command;
		if (param.shadingType == FLAT_SHADING) {
			for (int i = 0; i < fifo.size() - 1; i++) {
				param.vert1 = fifo[i];
				param.vert1.x = (param.vert1.x + drawOffset.x);
				param.vert1.y = (param.vert1.y + drawOffset.y);
				param.vert2 = fifo[i + 1];
				param.vert2.x = (param.vert2.x + drawOffset.x);
				param.vert2.y = (param.vert2.y + drawOffset.y);
				drawLine(param);
			}
		}
		else if (param.shadingType == GOURAUD_SHADING) {
			for (int i = 0; i < fifo.size() - 2; i += 2) {
				param.vert1 = fifo[i];
				param.vert1.x = (param.vert1.x + drawOffset.x);
				param.vert1.y = (param.vert1.y + drawOffset.y);
				param.color2 = fifo[i + 1];
				param.vert2 = fifo[i + 2];
				param.vert2.x = (param.vert2.x + drawOffset.x);
				param.vert2.y = (param.vert2.y + drawOffset.y);

				if (fifo[0] == fifo[i + 2]) //!!?????
					param.color2 = command & 0x00ffffff;

				drawLine(param);

				param.commandColor1 = (param.commandColor1 & 0xff000000) | param.color2.data;

			}
		}

		fifo.reset();

		break;

	case 0b011: { //Rectangle primitive
		param.primitiveType = RECT;

		param.rectSize = command << 3 >> 30;
		param.bTextured = (command & 0x04000000) ? true : false;
		param.transpanencyType = (command & 0x02000000) ? SEMI_TRANSPARENT : NOT_TRANSPARENT;
		param.modulationType = (command & 0x01000000) ? RAW_TEXTURE : TEXTURE_BLENDING;

		param.commandColor1 = gp0Command;

		param.vert1 = fifo.getNext();
		param.vert1.x = signed11bit(param.vert1.x + drawOffset.x); //??
		param.vert1.y = signed11bit(param.vert1.y + drawOffset.y); //??

		if (param.bTextured)
			param.texCoord1Palette = (texcoordType_t)fifo.getNext();

		switch (param.rectSize) {
		case 0b00:
			param.size_y_x = fifo.getNext();
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

		uint16_t xsiz = param.size_y_x & 0xffff;
		uint16_t ysiz = param.size_y_x >> 16;

		/*fifoClocks[fifoIndex] = xsiz * ysiz / 14;
		fifoIndex++;*/

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
			case 3: //Reserved //Disney's Aladdin in Nasira's Revenge uses
				//g_emulationPaused = true;
				//std::cout << "~[GPU] EMULATION PAUSED! Texture page color mode RESERVED!" << std::endl;
				drawTexRect15Bit(param);
				break;
			}
		else
			drawMonochromeRect(param);

		break;
	}
	case 0b100: { //VRAM-to-VRAM blit
		uint32_t sourceCoord = fifo.getNext();
		uint16_t xpos = sourceCoord & 0xffff;
		uint16_t ypos = sourceCoord >> 16;
		maskPosCopy(xpos, ypos);
		sourceCoord = ((uint32_t)ypos << 16) | xpos;

		uint32_t destCoord = fifo.getNext();
		xpos = destCoord & 0xffff;
		ypos = destCoord >> 16;
		maskPosCopy(xpos, ypos);
		destCoord = ((uint32_t)ypos << 16) | xpos;

		uint32_t widthHeight = fifo.getNext();
		uint16_t xsiz = widthHeight & 0xffff;
		uint16_t ysiz = widthHeight >> 16;
		maskSizCopy(xsiz, ysiz);
		widthHeight = ((uint32_t)ysiz << 16) | xsiz;

		/*fifoClocks[fifoIndex] = xsiz * ysiz / 15;
		fifoIndex++;*/

		copyRectVramVram(sourceCoord, destCoord, widthHeight);
		break;
	}
	case 0b101: //CPU-to-VRAM blit
		if (bCollectCopyCpuVramParam) {
			bCollectCopyCpuVramParam = false;

			destCoord = fifo.getNext();
			uint16_t xpos = destCoord & 0xffff;
			uint16_t ypos = destCoord >> 16;
			maskPosCopy(xpos, ypos);
			destCoord = ((uint32_t)ypos << 16) | xpos;

			widthHeight = fifo.getNext();
			uint16_t xsiz = widthHeight & 0xffff;
			uint16_t ysiz = widthHeight >> 16;
			maskSizCopy(xsiz, ysiz);
			widthHeight = ((uint32_t)ysiz << 16) | xsiz;

			/*fifoClocks[fifoIndex] = xsiz * ysiz / 15;
			fifoIndex++;*/

			collectCount = xsiz * ysiz / 2;
			if ((collectCount * 2) < xsiz * ysiz)
				collectCount++;
		}
		else
			copyRectCpuVram(destCoord, widthHeight);

		break;

	case 0b110: { //VRAM-to-CPU blit
		sourceCoord = fifo.getNext();
		uint16_t xpos = sourceCoord & 0xffff;
		uint16_t ypos = sourceCoord >> 16;
		maskPosCopy(xpos, ypos);
		sourceCoord = ((uint32_t)ypos << 16) | xpos;

		widthHeight = fifo.getNext();
		uint16_t xsiz = widthHeight & 0xffff;
		uint16_t ysiz = widthHeight >> 16;
		maskSizCopy(xsiz, ysiz);
		widthHeight = ((uint32_t)ysiz << 16) | xsiz;

		/*fifoClocks[fifoIndex] = xsiz * ysiz / 15;
		fifoIndex++;*/

		copyRectVramCpu(sourceCoord, widthHeight);
		gpuStat.elem.readyVramCpu = 1;
		break;
	}
	case 0b111: //Environment commands
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
			drawOffset.x = signed11bit(uint16_t(command & 0x7ff));
			drawOffset.y = signed11bit(uint16_t((command >> 11) & 0x7ff));
			gp1DrawOffset = command & 0x003fffff;;
			break;
		case 0xe6:
			gpuStat.elem.setMaskBitWhileDrawing = command & 0x1;
			gpuStat.elem.drawPixels = (command & 0x2 ? 1 : 0);
			break;
		}
	}
}

void gpu_t::decodeExecuteGp1Command(const uint32_t& command) {

	switch (command >> 24) {
	case 0x0:
		//clear fifo

		gpuStat.elem.irq1 = 0;

		gpuStat.elem.displayEnable = 0;

		gpuStat.elem.dmaDir = 0;

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

		if (!gpuStat.elem.videoMode) {
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
			scanlineNumber = SCANLINE_NUMBER_NTSC;
		}
		else {
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

		drawOffset.x = signed11bit(uint16_t(0 & 0x7ff));
		drawOffset.y = signed11bit(uint16_t((0 >> 11) & 0x7ff));
		gp1DrawOffset = 0 & 0x003fffff;;

		gpuStat.elem.setMaskBitWhileDrawing = 0 & 0x1;
		gpuStat.elem.drawPixels = (0 & 0x2 ? 1 : 0);

		gpuStat.data = 0x14802000;
		break;
	case 0x1:
		//std::cout << "~[GPU] unhandled properly GPU1 command: Reset Command Buffer" << std::endl;
		break;
	case 0x2:
		gpuStat.elem.irq1 = 0;
		break;
	case 0x3:
		gpuStat.elem.displayEnable = command & 0x1;
		break;
	case 0x4:
		gpuStat.elem.dmaDir = command & 0x3;
		/*switch (command & 0x3) {
		case 0:
			gpuStat.elem.dataRequest = 0;
			break;
		case 1:
			gpuStat.elem.dataRequest = (fifoCount >= 16) ? 0 : 1;
			break;
		case 2:
			gpuStat.elem.dataRequest = gpuStat.elem.readyDma;
			break;
		case 3:
			gpuStat.elem.dataRequest = gpuStat.elem.readyVramCpu;
			break;
		}*/
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
			cpuScanlineClocksNumber = CPU_CLOCKS_PER_SCANLINE_NTSC;
			scanlineNumber = SCANLINE_NUMBER_NTSC;
		}
		else {
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
			std::cout << "~[GPU] Old 160pin GPU" << std::endl;
			break;
		default:
			std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x" << std::hex << command << std::endl;
			g_emulationPaused = true;
			break;
		}
		break;
	default:
		std::cout << "~[GPU] unhandled GPU1 command: 0x" << std::hex << command << std::endl;
		g_emulationPaused = true;
		break;
	}
	return;
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

	if (g_logPixel && (possition.x == g_mousePossitionX) && (possition.y == g_mousePossitionY)) {
		vramView.SetPixel(possition.x, possition.y, olc::YELLOW);
		std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << gp0Command << " " << gp0Lookup[gp0Command >> 24].name << std::endl;
		g_emulationPaused = true;
	}
}

void gpu_t::writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		if ((position.x < 0) || (position.y < 0)) {
			std::cout << "~[GPU] EMULATION PAUSED! Write Vram out of bounds!" << std::dec << " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			g_emulationPaused = true;
			return;
		}

		uint16_t backPixel;
		readVram(position, backPixel, bIgnoreDrawingArea);

		if ((gpuStat.elem.drawPixels && ((backPixel & 0x8000) == 0x0000)) || (gpuStat.elem.drawPixels == 0)) {
			*((uint16_t*)(vram + ((position.x & 1023) << 1) + (position.y & 511) * size_x_b)) = data | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x8000 : 0);
			updateVramView(position, data);
		}
		return;
	}

	if (((position.x) >= drawAreaTopLeft.x) &&
		((position.y) >= drawAreaTopLeft.y) &&
		((position.x) <= drawAreaBottomRight.x) &&
		((position.y) <= drawAreaBottomRight.y)) {

		uint16_t backPixel;
		readVram(position, backPixel, bIgnoreDrawingArea);

		if ((gpuStat.elem.drawPixels && ((backPixel & 0x8000) == 0x0000)) || (gpuStat.elem.drawPixels == 0)) {
			*((uint16_t*)(vram + ((position.x & 1023) << 1) + (position.y & 511) * size_x_b)) = data | ((gpuStat.elem.setMaskBitWhileDrawing) ? 0x8000 : 0);
			updateVramView(position, data);
		}
	}
}

void gpu_t::readVram(const vertex_t& position, uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		if ((position.x < 0) || (position.y < 0)) {
			std::cout << "~[GPU] EMULATION PAUSED! Read Vram out of bounds!" << std::dec << " (x,y) (" << position.x << "," <<
				position.y << ")" << "\n";
			g_emulationPaused = true;
			return;
		}

		data = *((uint16_t*)(vram + ((position.x & 1023) << 1) + (position.y & 511) * size_x_b));
		return;
	}

	if ((position.x >= drawAreaTopLeft.x) &&
		(position.y >= drawAreaTopLeft.y) &&
		(position.x <= drawAreaBottomRight.x) &&
		(position.y <= drawAreaBottomRight.y)) {

		data = *((uint16_t*)(vram + ((position.x & 1023) << 1) + (position.y & 511) * size_x_b));
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

void gpu_t::calculateTransparency(const vertex_t& p, psxColor16_t& frontPixelColor) {
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

void gpu_t::drawPolygon(param_t* pParam) {
	const uint8_t vertexCount = pParam->verticesNumber;

	vertex_t vert1 = pParam->vert1;
	vertex_t vert2 = pParam->vert2;
	vertex_t vert3 = pParam->vert3;
	vertex_t vert4 = pParam->vert4;

	psxColor24_t color1 = pParam->commandColor1;
	psxColor24_t color2 = pParam->color2;
	psxColor24_t color3 = pParam->color3;
	psxColor24_t color4 = pParam->color4;

	psxColor16_t frontPixelColor;
	psxColor24_t calculatedPixelColor;

	std::tuple p0{ vert1.x, vert1.y, color1.component.blue, color1.component.green, color1.component.red };
	std::tuple p1{ vert2.x, vert2.y, color2.component.blue, color2.component.green, color2.component.red };
	std::tuple p2{ vert3.x, vert3.y, color3.component.blue, color3.component.green, color3.component.red };

	DrawPolygonInterpolated(
		p0, p1, p2,
		[&](int x, int y, int b, int g, int r)
		{

			vertex_t p((int16_t)x, (int16_t)y);

			if (pParam->shadingType == GOURAUD_SHADING) {
				calculatedPixelColor.component.red = (uint8_t)r;
				calculatedPixelColor.component.blue = (uint8_t)b;
				calculatedPixelColor.component.green = (uint8_t)g;
			}
			else
				calculatedPixelColor = pParam->commandColor1;


			frontPixelColor.data = format24to16Color(calculatedPixelColor.data);

			if (pParam->transpanencyType == SEMI_TRANSPARENT)
				calculateTransparency(p, frontPixelColor);

			writeVram(p, frontPixelColor.data);
		});


	if (vertexCount == 4) {

		std::tuple p0{ vert2.x, vert2.y, color2.component.blue, color2.component.green, color2.component.red };
		std::tuple p1{ vert3.x, vert3.y, color3.component.blue, color3.component.green, color3.component.red };
		std::tuple p2{ vert4.x, vert4.y, color4.component.blue, color4.component.green, color4.component.red };

		DrawPolygonInterpolated(
			p0, p1, p2,
			[&](int x, int y, int b, int g, int r)
			{
				vertex_t p((int16_t)x, (int16_t)y);

				if (pParam->shadingType == GOURAUD_SHADING) {
					calculatedPixelColor.component.red = (uint8_t)r;
					calculatedPixelColor.component.blue = (uint8_t)b;
					calculatedPixelColor.component.green = (uint8_t)g;
				}
				else
					calculatedPixelColor = pParam->commandColor1;

				frontPixelColor.data = format24to16Color(calculatedPixelColor.data);

				if (pParam->transpanencyType == SEMI_TRANSPARENT)
					calculateTransparency(p, frontPixelColor);

				writeVram(p, frontPixelColor.data);
			});
	}
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
	case 2: //15bit
		texBittnesVar.bit15 = true;
		texBittnesVar.devider = 1;
		texBittnesVar.firstAnder = 0x0;
		break;
	case 3: //Reserved //Disney's Aladdin in Nasira's Revenge uses
		//g_emulationPaused = true;
		//std::cout << "~[GPU] EMULATION PAUSED! Texture page color mode RESERVED!" << std::endl;
		texBittnesVar.bit15 = true;
		texBittnesVar.devider = 1;
		texBittnesVar.firstAnder = 0x0;
		break;
	}

	clut_t clut = pParam->texCoord1Palette.elem.attribute;

	const uint8_t vertexCount = pParam->verticesNumber;

	int x1 = pParam->vert1.x;
	int y1 = pParam->vert1.y;
	int x2 = pParam->vert2.x;
	int y2 = pParam->vert2.y;
	int x3 = pParam->vert3.x;
	int y3 = pParam->vert3.y;
	int x4 = pParam->vert4.x;
	int y4 = pParam->vert4.y;

	psxColor24_t color1 = pParam->commandColor1;
	psxColor24_t color2 = pParam->color2;
	psxColor24_t color3 = pParam->color3;
	psxColor24_t color4 = pParam->color4;

	int u1 = pParam->texCoord1Palette.elem.x;
	int v1 = pParam->texCoord1Palette.elem.y;
	int u2 = pParam->texCoord2TexPage.elem.x;
	int v2 = pParam->texCoord2TexPage.elem.y;
	int u3 = pParam->texCoord3.elem.x;
	int v3 = pParam->texCoord3.elem.y;
	int u4 = pParam->texCoord4.elem.x;
	int v4 = pParam->texCoord4.elem.y;


	psxColor24_t blendingColor;
	psxColor16_t frontPixelColor;

	std::tuple p0{ x1, y1, color1.component.blue, color1.component.green, color1.component.red, u1, v1 };
	std::tuple p1{ x2, y2, color2.component.blue, color2.component.green, color2.component.red, u2, v2 };
	std::tuple p2{ x3, y3, color3.component.blue, color3.component.green, color3.component.red, u3, v3 };

	DrawPolygonInterpolated(
		p0, p1, p2,
		[&](int x, int y, int b, int g, int r, int u, int v) {
			vertex_t p((int16_t)x, (int16_t)y);

			vertex_t pTex;
			pTex.y = gpuStat.elem.texPageYBase << 8;
			pTex.y += ((int)round(v) & 255 & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += (((((int)round(u) /*+ ((texBittnesVar.devider == 4) ? ((4 - (u1 & texBittnesVar.firstAnder)) & 0x3) : (u1 & texBittnesVar.firstAnder))*/)
				/ texBittnesVar.devider & 255 / texBittnesVar.devider) & (~((texWinMask.x << 3) / texBittnesVar.devider)))
				| ((texWinOffset.x & texWinMask.x) / texBittnesVar.devider << 3));

			uint16_t data;
			readVram(pTex, data, true);

			if (texBittnesVar.bit15)
				frontPixelColor.data = data;
			else {
				vertex_t pClut;
				pClut.x = (clut.elem.xCoord << 4) +
					((data >> ((((int)round(u) /*+ ((texBittnesVar.devider == 4) ? ((4 - (u1 & texBittnesVar.firstAnder)) & 0x3) : (u1 & texBittnesVar.firstAnder))*/)
						& texBittnesVar.firstAnder) << texBittnesVar.shifter)) & texBittnesVar.secondAnder);
				pClut.y = clut.elem.yCoord;
				readVram(pClut, frontPixelColor.data, true);
			}

			if (frontPixelColor.data) {
				if (pParam->shadingType == GOURAUD_SHADING) {
					blendingColor.component.red = (uint8_t)r;
					blendingColor.component.blue = (uint8_t)b;
					blendingColor.component.green = (uint8_t)g;
				}
				else
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
		});

	if (vertexCount == 4) {

		std::tuple p0{ x2, y2, color2.component.blue, color2.component.green, color2.component.red, u2, v2 };
		std::tuple p1{ x3, y3, color3.component.blue, color3.component.green, color3.component.red, u3, v3 };
		std::tuple p2{ x4, y4, color4.component.blue, color4.component.green, color4.component.red, u4, v4 };

		DrawPolygonInterpolated(
			p0, p1, p2,
			[&](int x, int y, int b, int g, int r, int u, int v) {
				vertex_t p((int16_t)x, (int16_t)y);

				vertex_t pTex;
				pTex.y = gpuStat.elem.texPageYBase << 8;
				pTex.y += ((int)round(v) & 255 & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

				pTex.x = gpuStat.elem.texPageXBase << 6;
				pTex.x += (((((int)round(u) /*+ ((texBittnesVar.devider == 4) ? ((4 - (u1 & texBittnesVar.firstAnder)) & 0x3) : (u1 & texBittnesVar.firstAnder))*/) / texBittnesVar.devider & 255 / texBittnesVar.devider) & (~((texWinMask.x << 3) / texBittnesVar.devider)))
					| ((texWinOffset.x & texWinMask.x) / texBittnesVar.devider << 3));

				uint16_t data;
				readVram(pTex, data, true);

				if (texBittnesVar.bit15)
					frontPixelColor.data = data;
				else {
					vertex_t pClut;
					pClut.x = (clut.elem.xCoord << 4) +
						((data >> ((((int)round(u) /*+ ((texBittnesVar.devider == 4) ? ((4 - (u1 & texBittnesVar.firstAnder)) & 0x3) : (u1 & texBittnesVar.firstAnder))*/) & texBittnesVar.firstAnder) << texBittnesVar.shifter)) & texBittnesVar.secondAnder);
					pClut.y = clut.elem.yCoord;
					readVram(pClut, frontPixelColor.data, true);
				}

				if (frontPixelColor.data) {
					if (pParam->shadingType == GOURAUD_SHADING) {
						blendingColor.component.red = (uint8_t)r;
						blendingColor.component.blue = (uint8_t)b;
						blendingColor.component.green = (uint8_t)g;
					}
					else
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
			});
	}
}

void gpu_t::drawMonochromeRect(const param_t& param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.size_y_x & 0xffff);
	int16_t y2 = y1 + ((param.size_y_x >> 16) & 0xffff);

	for (int16_t y = y1; y < y2; y++)
		for (int16_t x = x1; x < x2; x++) {
			vertex_t p(x, y);

			psxColor16_t frontPixelColor = format24to16Color(param.commandColor1);

			if (param.transpanencyType == SEMI_TRANSPARENT)
				calculateTransparency(p, frontPixelColor);

			writeVram(p, frontPixelColor.data);
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

void gpu_t::drawLine(const param_t& param) { //could be optimized

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

void gpu_t::drawTexRect15Bit(const param_t& param) {
	psxColor24_t blendingColor = param.commandColor1;
	int16_t u1 = param.texCoord1Palette.elem.x;
	int16_t v1 = param.texCoord1Palette.elem.y;
	int16_t u2 = u1 + (param.size_y_x & 0xffff);
	int16_t v2 = v1 + ((param.size_y_x >> 16) & 0xffff);
	uint16_t texData;
	vertex_t pTex;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.size_y_x & 0xffff);
	int16_t y2 = y1 + ((param.size_y_x >> 16) & 0xffff);

	psxColor16_t frontPixelColor;
	vertex_t p;

	float ratioX = ((float)(u2 - u1) / (float)(x2 - x1));
	float ratioY = ((float)(v2 - v1) / (float)(y2 - y1));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	for (int16_t y = 0; y < y2 - y1; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += ((v1 + (int16_t)((float)y * ratioY)) & 255 & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		p.y = y1 + y;

		for (int16_t x = 0; x < x2 - x1; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += ((u1 + ((int16_t)((float)x * ratioX))) & 255 & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);

			p.x = x1 + x;

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

void gpu_t::drawTexRect8Bit(const param_t& param) {
	psxColor24_t blendingColor = param.commandColor1;
	int16_t u1 = param.texCoord1Palette.elem.x;
	int16_t v1 = param.texCoord1Palette.elem.y;
	int16_t u2 = u1 + (param.size_y_x & 0xffff);
	int16_t v2 = v1 + ((param.size_y_x >> 16) & 0xffff);
	uint16_t texData;
	vertex_t pTex;
	clut_t clut = param.texCoord1Palette.elem.attribute;
	vertex_t pClut;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.size_y_x & 0xffff);
	int16_t y2 = y1 + ((param.size_y_x >> 16) & 0xffff);

	psxColor16_t frontPixelColor;
	vertex_t p;

	float ratioX = ((float)(u2 - u1) / (float)(x2 - x1));
	float ratioY = ((float)(v2 - v1) / (float)(y2 - y1));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	for (int16_t y = 0; y < y2 - y1; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += ((v1 + (int16_t)((float)y * ratioY)) & 255 & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		p.y = y1 + y;

		for (int16_t x = 0; x < x2 - x1; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += (((u1 >> 1) + (((int16_t)((float)x * ratioX) + (u1 & 0x1)) >> 1)) & (255 >> 1) & (~(texWinMask.x << 2))) | ((texWinOffset.x & texWinMask.x) << 2);

			p.x = x1 + x;

			readVram(pTex, texData, true);
			pClut.x = clut.elem.xCoord * 16 +
				(texData >> ((((int16_t)((float)x * ratioX) + (u1 & 0x1)) & 0x1) * 8) & 0xff);
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

void gpu_t::drawTexRect4Bit(const param_t& param) {
	psxColor24_t blendingColor = param.commandColor1;
	int16_t u1 = param.texCoord1Palette.elem.x;
	int16_t v1 = param.texCoord1Palette.elem.y;
	int16_t u2 = u1 + (param.size_y_x & 0xffff);
	int16_t v2 = v1 + ((param.size_y_x >> 16) & 0xffff);
	uint16_t texData;
	vertex_t pTex;
	clut_t clut = param.texCoord1Palette.elem.attribute;
	vertex_t pClut;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = x1 + (param.size_y_x & 0xffff);
	int16_t y2 = y1 + ((param.size_y_x >> 16) & 0xffff);

	psxColor16_t frontPixelColor;
	psxColor16_t randa = rand() & 0x7fff;
	vertex_t p;

	float ratioX = ((float)(u2 - u1) / (float)(x2 - x1));
	float ratioY = ((float)(v2 - v1) / (float)(y2 - y1));

#if DISABLE_SCALING
	ratioX = 1.f;
	ratioY = 1.f;
#endif

	for (int16_t y = 0; y < y2 - y1; y++) {
		pTex.y = gpuStat.elem.texPageYBase << 8;
		pTex.y += ((v1 + (int16_t)((float)y * ratioY)) & 255 & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);

		p.y = y1 + y;

		for (int16_t x = 0; x < x2 - x1; x++) {
			pTex.x = gpuStat.elem.texPageXBase << 6;
			pTex.x += (((u1 >> 2) + (((int16_t)((float)x * ratioX) + (u1 & 0x3)) >> 2)) & (255 >> 2) & (~(texWinMask.x << 1))) | ((texWinOffset.x & texWinMask.x) << 1);

			p.x = x1 + x;

			readVram(pTex, texData, true);
			pClut.x = (clut.elem.xCoord << 4) +
				((texData >> ((((int16_t)((float)x * ratioX) + (u1 & 0x3)) & 0x3) << 2)) & 0xf);
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

void gpu_t::copyRectCpuVram(const uint32_t& dest_coord, const uint32_t& width_height) {
	const uint32_t x1 = (dest_coord & 0xffff) & 0x3ff;
	uint32_t x2 = x1 + (width_height & 0xffff);
	const uint32_t y1 = (dest_coord >> 16) & 0x1ff;
	uint32_t y2 = y1 + (width_height >> 16);

	uint32_t i = 0;
	uint32_t data;
	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			if ((i & 0x1) == 0)
				data = fifo.getNext();

			vertex_t position(x, y);
			writeVram(position, ((i & 0x1) == 0) ? (uint16_t)data : uint16_t(data >> 16), true);

			i++;
		}
	}
}

void gpu_t::copyRectVramCpu(const uint32_t& source_coord, const uint32_t& width_height) {
	const uint16_t x1 = source_coord & 0xffff;
	const uint16_t x2 = x1 + (width_height & 0xffff);
	const uint16_t y1 = source_coord >> 16;
	const uint16_t y2 = y1 + (width_height >> 16);

	uint16_t data16;
	uint32_t data32;
	uint32_t i = 0;
	for (uint16_t y = y1; y < y2; y++) {
		for (uint16_t x = x1; x < x2; x++) {
			vertex_t position(x, y);
			readVram(position, data16, true);

			if ((i & 0x1) == 0)
				data32 = 0x00000000 | (uint32_t)data16;
			else {
				data32 |= (uint32_t)data16 << 16;
				outputFifo.pushBack(data32);
			}
			i++;
		}
	}
}

void gpu_t::copyRectVramVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight) {
	const uint16_t sourceX1 = sourceCoord & 0xffff;
	const uint16_t sourceY1 = sourceCoord >> 16;

	const uint16_t destX1 = destCoord & 0xffff;
	const uint16_t destY1 = destCoord >> 16;

	const uint16_t width = widthHeight & 0xffff;
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

void gpu_t::fillRectVram(const param_t& param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert4.x;
	int16_t y2 = param.vert4.y;

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