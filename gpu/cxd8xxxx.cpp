#include "cxd85xxx.h"
#include "../bus/bus_interface.h"

extern bool g_emulationPaused;

cxd85xxx::cxd85xxx() {
	gpuStat.data = 0x1c000000;
	gp0Lookup.resize(0xef);

	gp0Lookup = {
		//0x00
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::clearCache,"Clear Cache",1},
		{&cxd85xxx::fillRectVram,"Fill Rectangle in VRAM",3},
		{&cxd85xxx::unknown,"Unknown?",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		//0x10
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::nop,"Mirror of NOP (?)",1},
		{&cxd85xxx::interruptRequest,"Interrupt Request (IRQ1)",1},
		//0x20
		{&cxd85xxx::mono3PolyOpaq,"Monochrome three-point polygon, opaque",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::mono3PolySemiTransp,"Monochrome three - point polygon, semi - transparent",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::tex3PolyOpaqTexBlend,"Textured three-point polygon, opaque, texture-blending",7},
		{&cxd85xxx::tex3PolyOpaqRawTex,"Textured three-point polygon, opaque, raw-texture",7},
		{&cxd85xxx::tex3PolySemiTranspTexBlend,"Textured three-point polygon, semi-transparent, texture-blending",7},
		{&cxd85xxx::tex3PolySemiTranspRawTex,"Textured three-point polygon, semi-transparent, raw-texture",7},
		{&cxd85xxx::mono4PolyOpaq,"Monochrome four-point polygon, opaque",5},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::mono4PolySemiTransp,"Monochrome four-point polygon, semi-transparent",5},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::tex4PolyOpaqTexBlend,"Textured four-point polygon, opaque, texture-blending",9},
		{&cxd85xxx::tex4PolyOpaqRawTex,"Textured four-point polygon, opaque, raw-texture",9},
		{&cxd85xxx::tex4PolySemiTranspTexBlend,"Textured four-point polygon, semi-transparent, texture-blending",9},
		{&cxd85xxx::tex4PolySemiTranspRawTex,"Textured four-point polygon, semi-transparent, raw-texture",9},
		//0x30
		{&cxd85xxx::shaded3PolyOpaq,"Shaded three-point polygon, opaque",6},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shaded3PolySemiTransp,"Shaded three-point polygon, semi-transparent",6},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedTex3PolyOpaqTexBlend,"Shaded Textured three-point polygon, opaque, texture-blending",9},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedTex3PolySemiTranspTexBlend,"Shaded Textured three-point polygon, semi-transparent, tex-blend",9},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shaded4PolyOpaq,"Shaded four-point polygon, opaque",8},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shaded4PolySemiTransp,"Shaded four-point polygon, semi-transparent",8},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedTex4PolyOpaqTexBlend,"Shaded Textured four-point polygon, opaque, texture-blending",12},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedTex4PolySemiTranspTexBlend,"Shaded Textured four-point polygon, semi-transparent, tex-blend",12},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		//0x40
		{&cxd85xxx::monoLineOpaq,"Monochrome line, opaque",3},
		{&cxd85xxx::monoLineOpaq,"Monochrome line, opaque",3}, //???
		{&cxd85xxx::monoLineSemiTransp,"Monochrome line, semi-transparent",3},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoPolyLineOpaq,"Monochrome Poly-line, opaque",3},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoPolyLineSemiTransp,"Monochrome Poly-line, semi-transparent",3},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		//0x50
		{&cxd85xxx::shadedLineOpaq,"Shaded line, opaque",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedLineSemiTransp,"Shaded line, semi-transparent",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedPolyLineOpaq,"Shaded Poly-line, opaque",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::shadedPolyLineSemiTransp,"Shaded Poly-line, semi-transparent",4},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		//0x60
		{&cxd85xxx::monoRectVarOpaq,"Monochrome Rectangle (variable size) (opaque)",3},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoRectVarSemiTransp,"Monochrome Rectangle (variable size) (semi-transparent)",3},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::texRectVarOpaqTexBlend,"Textured Rectangle, variable size, opaque, texture-blending",4},
		{&cxd85xxx::texRectVarOpaqRawTex,"Textured Rectangle, variable size, opaque, raw-texture",4},
		{&cxd85xxx::texRectVarSemiTranspTexBlend,"Textured Rectangle, variable size, semi-transp, texture-blending",4},
		{&cxd85xxx::texRectVarSemiTranspRawTex,"Textured Rectangle, variable size, semi-transp, raw-texture",4},
		{&cxd85xxx::monoRect1x1Opaq,"Monochrome Rectangle (1x1) (Dot) (opaque)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoRect1x1SemiTransp,"Monochrome Rectangle (1x1) (Dot) (semi-transparent)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::texRect1x1OpaqTexBlend,"Textured Rectangle, 1x1 (nonsense), opaque, texture-blending",3},
		{&cxd85xxx::texRect1x1OpaqRawTex,"Textured Rectangle, 1x1 (nonsense), opaque, raw-texture",3},
		{&cxd85xxx::texRect1x1SemiTranspTexBlend,"Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending",3},
		{&cxd85xxx::texRect1x1SemiTranspRawTex,"Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture",3},
		//0x70
		{&cxd85xxx::monoRect8x8Opaq,"Monochrome Rectangle (8x8) (opaque)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoRect8x8SemiTransp,"Monochrome Rectangle (8x8) (semi-transparent)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::texRect8x8OpaqTexBlend,"Textured Rectangle, 8x8, opaque, texture-blending",3},
		{&cxd85xxx::texRect8x8OpaqRawTex,"Textured Rectangle, 8x8, opaque, raw-texture",3},
		{&cxd85xxx::texRect8x8SemiTranspTexBlend,"Textured Rectangle, 8x8, semi-transparent, texture-blending",3},
		{&cxd85xxx::texRect8x8SemiTranspRawTex,"Textured Rectangle, 8x8, semi-transparent, raw-texture",3},
		{&cxd85xxx::monoRect16x16Opaq,"Monochrome Rectangle (16x16) (opaque)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::monoRect16x16SemiTransp,"Monochrome Rectangle (16x16) (semi-transparent)",2},
		{&cxd85xxx::undocumented,"Undocumented/Nonsense",1},
		{&cxd85xxx::texRect16x16OpaqTexBlend,"Textured Rectangle, 16x16, opaque, texture-blending",3},
		{&cxd85xxx::texRect16x16OpaqRawTex,"Textured Rectangle, 16x16, opaque, raw-texture",3},
		{&cxd85xxx::texRect16x16SemiTranspTexBlend,"Textured Rectangle, 16x16, semi-transparent, texture-blending",3},
		{&cxd85xxx::texRect16x16SemiTranspRawTex,"Textured Rectangle, 16x16, semi-transparent, raw-texture",3},
		//0x80
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		//0x90
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		{&cxd85xxx::copyRectVramVram,"Copy Rectangle (VRAM to VRAM)",4},
		//0xa0
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		//0xb0										  			   
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		{&cxd85xxx::copyRectCpuVram,"Copy Rectangle (CPU to VRAM)",3},
		//0xc0
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		//0xd0										   			   
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		{&cxd85xxx::copyRectVramCpu,"Copy Rectangle (VRAM to CPU)",3},
		//0xe0
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::drawModeSetting,"Draw Mode setting (aka 'Texpage')",1},
		{&cxd85xxx::texWindowSetting,"Texture Window setting",1},
		{&cxd85xxx::setDrawAreaTopLeft,"Set Drawing Area top left (X1,Y1)",1},
		{&cxd85xxx::setDrawAreaBottomRight,"Set Drawing Area bottom right (X2,Y2)",1},
		{&cxd85xxx::setDrawOffset,"Set Drawing Offset (X,Y)",1},
		{&cxd85xxx::maskBitSetting,"Mask Bit Setting",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
		{&cxd85xxx::nop,"NOP (?)",1},
	};
}

void cxd85xxx::videoClock() {
	const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
	const uint32_t CPU_CLOCKS_PER_FRAME = CPU_CLOCK / 60;
	const uint32_t CPU_CLOCKS_PER_SCANLINE = 2172;

	if (cpuFrameClocks == CPU_CLOCKS_PER_FRAME) {
		cpuFrameClocks = 0;

		//V-Sync interrupt is naturally edge trigered
		pBus->pCpu->cp0.interruptHandler(_IRQ_V_SYNC_);

		if (gpuStat.elem.verticalRes && gpuStat.elem.verticalInterlace)
			gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;

		scanline = 0;

		//Vblank synchronization timer 1
		pBus->timer1.clock(_VBLANK_CLOCK);

		//HAHHAHA BYPASS SPU_DMA
		if (pBus->dma.interruptRegister.reg.IRQ_enable_dma4 && pBus->dma.interruptRegister.reg.IRQ_master_enable) {
			pBus->dma.interruptRegister.reg.IRQ_master_flag = 1;
			pBus->dma.interruptRegister.reg.IRQ_flag_dma4 = 1;
			pBus->pCpu->cp0.interruptHandler(_IRQ_DMA);
		}
	}

	if (cpuScanlineClocks == CPU_CLOCKS_PER_SCANLINE) {
		cpuScanlineClocks = 0;
		if (!gpuStat.elem.verticalRes) {
			scanline = 0;
			gpuStat.elem.drawingEvenOddLines = ~gpuStat.elem.drawingEvenOddLines;
		}
		scanline++;

		//Hblank clocking timer 1
		pBus->timer1.clock(_HBLANK_CLOCK);

	}

	cpuFrameClocks++;
	cpuScanlineClocks++;
}

void cxd85xxx::readGpu32(const uint32_t& addr, uint32_t& data) {
	data = 0;
	switch (addr) {
	case 0x1f801810:
		if (b_trans_command) {
			data = collectList[0];
			collectList.erase(collectList.begin());
			if (collectList.empty())
				b_trans_command = false;
		}
		if (b_gp1Result) {
			b_gp1Result = false;
			data = gp1Result;
		}
		break;
	case 0x1f801814:
		data = gpuStat.data;
		break;
	}
}

void cxd85xxx::writeGpu32(const uint32_t& addr, const uint32_t& data) {
	switch (addr) {
	case 0x1f801810:
		b_trans_command = false;
		fifo.push_back(data);

		if (collect) {
			collectList.push_back(fifo.back());
			collect--;
		}
		else {
			command = fifo.back();
			collectList.clear();//!!!!
			if (!collectList.empty()) {
				//std::cout << "[GPU] EMULATION PAUSED! collect list not empty 0x" << std::hex << collectList.size() << std::endl;
			//	isEmulationPaused = true;
			}

			collect = gp0Lookup[command >> 24].collectCount - 1;
			if (command == 0xa0000000)
				b_collect_param = true;
		}

		if (collect == 0) {
			//std::cout << "~[GPU] GPU0 command executed: 0x" << std::hex << command << " " << gp0Lookup[command >> 24].name << std::endl;
			(this->*gp0Lookup[command >> 24].command)(command);
		}

		fifo.pop_back();
		break;

	case 0x1f801814:
		gp1 = data;
		switch (gp1 >> 24) {
		case 0x0:
			gpuStat.data = 0x1c802000;
			break;
		case 0x2:
			gpuStat.elem.irq1 = 0;
		case 0x3:
			gpuStat.elem.displayEnable = data & 0x1;
			break;
		case 0x4:
			gpuStat.elem.dmaDir = gp1 & 0x3;
			break;
		case 0x5:
			display_start.x = gp1 << 22 >> 22;
			display_start.y = gp1 << 13 >> 23;
			break;
		case 0x6:
			x1 = gp1 << 20 >> 20;
			x2 = gp1 << 12 >> 24;
			break;
		case 0x7:
			y1 = gp1 << 22 >> 22;
			y2 = gp1 << 12 >> 22;
			break;
		case 0x8:
			gpuStat.elem.horizontalRes1 = gp1 & 0x3;
			gpuStat.elem.verticalRes = (gp1 & 0x4 ? 1 : 0);
			gpuStat.elem.videoMode = (gp1 & 0x8 ? 1 : 0);
			gpuStat.elem.displayAreaColorDepth = (gp1 & 0x10 ? 1 : 0);
			gpuStat.elem.verticalInterlace = (gp1 & 0x20 ? 1 : 0);
			gpuStat.elem.horizontalRes2 = gp1 & 0x40;
			gpuStat.elem.reverseflag = (gp1 & 0x80 ? 1 : 0);
			if (gpuStat.elem.verticalInterlace)
				gpuStat.elem.interlaceField = 1;
			else
				gpuStat.elem.interlaceField = 0;
			break;
		case 0x10:
			switch (gp1 & 0x7) {
			case 3:
				gp1Result = gp1TopLeft;
				b_gp1Result = true;
				break;
			case 4:
				gp1Result = gp1BottomRight;
				b_gp1Result = true;
				break;
			case 5:
				gp1Result = gp1DrawOffset;
				b_gp1Result = true;
				break;
			case 7:
				gp1Result = 0xff;
				std::cout << "~[GPU] unhandled GPU1 command 10 subfunction : 0x" << std::hex << data << std::endl;
				//g_emulationPaused = true;
				b_gp1Result = true;
				break;
			default:
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x" << std::hex << data << std::endl;
				g_emulationPaused = true;
				break;
			}
			break;
		default:
			//std::cout << "~[GPU] unhandled GPU1 command: 0x" << std::hex << data << std::endl;
			//isEmulationPaused = true;
			break;
		}
		break;
	}
}

void cxd85xxx::updateVramView(const vertex_t& possition, const psxColor16_t& color16) {
	auto ConvertPSXtoPC_RGB_Values = [=](const psxColor16_t& color16) {
		olc::Pixel pixel;
		pixel.r = color16.component.red << 3;
		pixel.g = color16.component.green << 3;
		pixel.b = color16.component.blue << 3;
		/*if (color16.component.red <= 8)
			pixel.r = color16.component.red << 4;
		else {
			float k = 15. / 23.;
			pixel.r = (uint8_t)(((float)color16.component.red * k + 10.78) * 8.);
		}
		if (color16.component.green <= 8)
			pixel.g = color16.component.green << 4;
		else {
			float k = 15. / 23.;
			pixel.g = (uint8_t)(((float)color16.component.green * k + 10.78) * 8.);
		}
		if (color16.component.blue <= 8)
			pixel.b = color16.component.blue << 4;
		else {
			float k = 15. / 23.;
			pixel.b = (uint8_t)(((float)color16.component.blue * k + 10.78) * 8.);
		}*/
		return pixel;
	};
	vramView.SetPixel(possition.x, possition.y, ConvertPSXtoPC_RGB_Values(color16));
}

void cxd85xxx::writeVram(const vertex_t& position, const uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		if ((position.x >= 1024) || (position.x < 0) || (position.y >= 512) || (position.y) < 0)
			return;

		vram[position.x * 2 * size_y + position.y] = data << 8 >> 8;
		vram[(position.x * 2 + 1) * size_y + position.y] = data >> 8;
		updateVramView(position, data);
		return;
	}

	if (((position.x + (drawOffset.x >> 5)) >= drawAreaTopLeft.x) &&
		((position.x + (drawOffset.x >> 5)) <= drawAreaBottomRight.x) &&
		((position.y + (drawOffset.y >> 5)) >= drawAreaTopLeft.y) &&
		((position.y + (drawOffset.y >> 5)) <= drawAreaBottomRight.y)) {

		if ((position.x + (drawOffset.x >> 5) >= 1024) || (position.x + (drawOffset.x >> 5) < 0) ||
			(position.y + (drawOffset.y >> 5) >= 512) || (position.y + (drawOffset.y >> 5)) < 0)
			return;

		vram[(position.x + (drawOffset.x >> 5)) * 2 * size_y + (position.y + (drawOffset.y >> 5))] = data << 8 >> 8;
		vram[((position.x + (drawOffset.x >> 5)) * 2 + 1) * size_y + (position.y + (drawOffset.y >> 5))] = data >> 8;
		vertex_t p = { 0, 0 };
		p.x = position.x + (drawOffset.x >> 5);
		p.y = position.y + (drawOffset.y >> 5);
		updateVramView(p, data);
	}
}

void cxd85xxx::readVram(const vertex_t& position, uint16_t& data) {
	if ((position.x >= 1024) || (position.x < 0) || (position.y >= 512) || (position.y) < 0) {
		data = 0;
		return;
	}

	data = vram[position.x * 2 * size_y + position.y];
	data |= vram[(position.x * 2 + 1) * size_y + position.y] << 8;
}

olc::Sprite* cxd85xxx::getVramSprite() {
	return &vramView;
}

uint16_t cxd85xxx::format24to16Color(const uint32_t& input) {
	psxColor24_t color24 = (psxColor24_t)input;
	psxColor16_t color16 = 0;
	color16.component.red = color24.component.red >> 3;
	color16.component.green = color24.component.green >> 3;
	color16.component.blue = color24.component.blue >> 3;
	return color16.data;
}

float cxd85xxx::edgeFunc(vertex_t vert1, vertex_t vert2, vertex_t point) {
	return ((float)vert1.y - (float)vert2.y) * ((float)point.x - (float)vert1.x) +
		((float)vert1.x - (float)vert2.x) * ((float)vert1.y - (float)point.y);
}

void cxd85xxx::rasterization(param_t* pParam, const rasterizationMode_t& colorMode, const rasterizationMode_t& transparencyMode,
	const uint8_t& vertexCount, vertex_t vert1, vertex_t vert2, vertex_t vert3, vertex_t vert4) {

	vertex_t calculatedPoint;
	psxColor24_t calculatedColor;
	psxColor24_t color1 = pParam->command_color1;
	psxColor24_t color2 = pParam->color2;
	psxColor24_t color3 = pParam->color3;
	psxColor24_t color4 = pParam->color4;

	int16_t arrayX[4] = { vert1.x, vert2.x, vert3.x, vert4.x };
	int16_t arrayY[4] = { vert1.y, vert2.y, vert3.y, vert4.y };

	int16_t xMin = utils::min(arrayX, vertexCount);
	int16_t xMax = utils::max(arrayX, vertexCount);
	int16_t yMin = utils::min(arrayY, vertexCount);
	int16_t yMax = utils::max(arrayY, vertexCount);

	if (vertexCount == 3) {
		float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;

		triangleOrientation_t orientation = P123 > 0 ? PSX_CLOCKWISE : PSX_ANTI_CLOCKWISE;

		vertex_t e1 = vert3 - vert2;
		vertex_t e2 = vert1 - vert3;
		vertex_t e3 = vert2 - vert1;

		if (orientation == PSX_CLOCKWISE)
			for (int x = xMin; x <= xMax; x++) {
				calculatedPoint.x = x;
				for (int y = yMin; y <= yMax; y++) {
					calculatedPoint.y = y;

					float e12 = edgeFunc(vert1, vert2, calculatedPoint);
					float e23 = edgeFunc(vert2, vert3, calculatedPoint);
					float e31 = edgeFunc(vert3, vert1, calculatedPoint);

					auto testEdgeAndWrite = [&]() {
						if ((e12 > 0) || ((e12 == 0) ? ((e3.y == 0) && (e3.x < 0)) || (e3.y < 0) : false))
							if ((e23 > 0) || ((e23 == 0) ? ((e1.y == 0) && (e1.x < 0)) || (e1.y < 0) : false))
								if ((e31 > 0) || ((e31 == 0) ? ((e2.y == 0) && (e2.x < 0)) || (e2.y < 0) : false)) {
									float lamda1 = e23 / 2.f / P123;
									float lamda2 = e31 / 2.f / P123;
									float lamda3 = e12 / 2.f / P123;
									calculatedColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
									calculatedColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
									calculatedColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
									//writeVram(calculatedPoint, format24to16Color(calculatedColor.data));
									writeVram(calculatedPoint, 0x2334334);
								}
					};

					testEdgeAndWrite();
				}
			}


		if (orientation == PSX_ANTI_CLOCKWISE) {
			P123 = -P123;
			for (int x = xMin; x <= xMax; x++) {
				calculatedPoint.x = x;
				for (int y = yMin; y <= yMax; y++) {
					calculatedPoint.y = y;

					float e21 = edgeFunc(vert2, vert1, calculatedPoint);
					float e32 = edgeFunc(vert3, vert2, calculatedPoint);
					float e13 = edgeFunc(vert1, vert3, calculatedPoint);

					auto testEdgeAndWrite = [&]() {

						if ((e21 > 0) || ((e21 == 0) ? ((e3.y == 0) && (e3.x > 0)) || (e3.y > 0) : false))
							if ((e32 > 0) || ((e32 == 0) ? ((e1.y == 0) && (e1.x > 0)) || (e1.y > 0) : false))
								if ((e13 > 0) || ((e13 == 0) ? ((e2.y == 0) && (e2.x > 0)) || (e2.y > 0) : false)) {
									float lamda1 = e32 / 2.f / P123;
									float lamda2 = e13 / 2.f / P123;
									float lamda3 = e21 / 2.f / P123;
									calculatedColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
									calculatedColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
									calculatedColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
									//writeVram(calculatedPoint, format24to16Color(calculatedColor.data));
									writeVram(calculatedPoint, 0x2334334);
								}
					};

					testEdgeAndWrite();
				}
			}
		}
	}


	if (vertexCount == 4) {
		float P123 = edgeFunc(vert1, vert2, vert3) / 2.f;
		float P432 = edgeFunc(vert4, vert3, vert2) / 2.f;

		for (int x = xMin; x <= xMax; x++) {
			calculatedPoint.x = x;
			for (int y = yMin; y <= yMax; y++) {
				calculatedPoint.y = y;

				float e12 = edgeFunc(vert1, vert2, calculatedPoint);
				float e23 = edgeFunc(vert2, vert3, calculatedPoint);
				float e32 = edgeFunc(vert3, vert2, calculatedPoint);
				float e31 = edgeFunc(vert3, vert1, calculatedPoint);
				float e43 = edgeFunc(vert4, vert3, calculatedPoint);
				float e24 = edgeFunc(vert2, vert4, calculatedPoint);

				if ((e12 >= 0) && (e31 >= 0) && (e23 >= 0)) {
					float lamda1 = e23 / 2.f / P123;
					float lamda2 = e31 / 2.f / P123;
					float lamda3 = e12 / 2.f / P123;
					calculatedColor.component.red = (float)color1.component.red * lamda1 + (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3;
					calculatedColor.component.green = (float)color1.component.green * lamda1 + (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3;
					calculatedColor.component.blue = (float)color1.component.blue * lamda1 + (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3;
					writeVram(calculatedPoint, format24to16Color(calculatedColor.data));
				}

				if ((e32 >= 0) && (e24 >= 0) && (e43 >= 0)) {
					float lamda2 = e43 / 2.f / P432;
					float lamda3 = e24 / 2.f / P432;
					float lamda4 = e32 / 2.f / P432;
					calculatedColor.component.red = (float)color2.component.red * lamda2 + (float)color3.component.red * lamda3 + (float)color4.component.red * lamda4;
					calculatedColor.component.green = (float)color2.component.green * lamda2 + (float)color3.component.green * lamda3 + (float)color4.component.green * lamda4;
					calculatedColor.component.blue = (float)color2.component.blue * lamda2 + (float)color3.component.blue * lamda3 + (float)color4.component.blue * lamda4;
					writeVram(calculatedPoint, format24to16Color(calculatedColor.data));
				}
			}
		}
	}
}

void cxd85xxx::drawMonochromeRect(param_t param) {
	uint16_t color16 = format24to16Color(param.command_color1);

	for (int16_t y = param.vert1.y; y < param.vert4.y; y++)
		for (int16_t x = param.vert1.x; x < param.vert4.x; x++) {
			vertex_t p(x, y);
			writeVram(p, color16, false);
		}
}

void cxd85xxx::drawMonochromeLine(param_t param) {
	int16_t arrayX[2] = { param.vert1.x, param.vert2.x };
	int16_t arrayY[2] = { param.vert1.y, param.vert2.y };
	int16_t xMin = utils::min(arrayX, 2);
	int16_t yMin = utils::min(arrayY, 2);
	int16_t xMax = utils::max(arrayX, 2);
	int16_t yMax = utils::max(arrayY, 2);

	vertex_t p;
	for (int16_t x = xMin; x <= xMax; x++) {
		p.x = x;
		for (int16_t y = yMin; y <= yMax; y++) {
			p.y = y;
			if (edgeFunc(param.vert1, param.vert2, p) == 0)
				writeVram(p, format24to16Color(param.command_color1));
		}
	}
}

void cxd85xxx::drawTexRect15Bit(param_t param) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };
	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	uint16_t data = 0;
	vertex_t pTex;
	vertex_t p;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;

	float ratioX = 1.;
	float ratioY = 1.;

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = (gpuStat.elem.texPageYBase << 8) + yTexMin;
		pTex.y = ((pTex.y + y) & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);
		p.y = yMin + y;
		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = (gpuStat.elem.texPageXBase << 6) + xTexMin;
			pTex.x = ((pTex.x + x) & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);
			p.x = xMin + x;
			readVram(pTex, texData);
			if (texData)
				writeVram(p, texData);
		}
	}
}

void cxd85xxx::drawTexRect8Bit(param_t param) {
	clut_t clut = param.texCoord1Palette.elem.attribute;
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	psxColor16_t pallete_color = 0;
	vertex_t pTex;
	vertex_t p;
	vertex_t pClut;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	psxColor24_t blending = param.command_color1;

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = (gpuStat.elem.texPageYBase << 8) + yTexMin;
		pTex.y = ((pTex.y + y) & (~(texWinMask.y << 3))) | ((texWinOffset.y & texWinMask.y) << 3);
		p.y = yMin + y;
		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = (gpuStat.elem.texPageXBase << 6) + (xTexMin >> 1);
			pTex.x = ((pTex.x + (x >> 1)) & (~(texWinMask.x << 3))) | ((texWinOffset.x & texWinMask.x) << 3);
			p.x = xMin + x;
			readVram(pTex, texData);
			pClut.x = clut.elem.xCoord * 16 +
				(texData >> ((x & 0x1) * 8) & 0xff);
			pClut.y = clut.elem.yCoord;
			readVram(pClut, pallete_color.data);
			if (pallete_color.data) {
				if (param.type == TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM) {
					pallete_color.component.red = (float)pallete_color.component.red * (float)blending.component.red / (float)0x80;
					pallete_color.component.green = (float)pallete_color.component.green * (float)blending.component.green / (float)0x80;
					pallete_color.component.blue = (float)pallete_color.component.blue * (float)blending.component.blue / (float)0x80;
				}
				writeVram(p, pallete_color.data);
			}
		}
	}
}

void cxd85xxx::drawTexRect4Bit(param_t param) {
	clut_t clut = param.texCoord1Palette.elem.attribute;
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

	int16_t xMin = utils::min(arrayX, 4);
	int16_t yMin = utils::min(arrayY, 4);
	int16_t xMax = utils::max(arrayX, 4);
	int16_t yMax = utils::max(arrayY, 4);

	uint16_t texData;
	psxColor16_t pallete_color = 0;
	vertex_t pTex;
	vertex_t p;
	vertex_t pClut;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	psxColor24_t blending = param.command_color1;

	for (int16_t y = 0; y < yMax - yMin; y++) {
		pTex.y = (gpuStat.elem.texPageYBase << 8) + yTexMin;
		pTex.y = ((pTex.y + y) & (~(texWinMask.y * 8))) | ((texWinOffset.y & texWinMask.y) * 8);
		p.y = yMin + y;
		for (int16_t x = 0; x < xMax - xMin; x++) {
			pTex.x = (gpuStat.elem.texPageXBase << 6) + (xTexMin >> 2);
			pTex.x = ((pTex.x + (x >> 2)) & (~(texWinMask.x * 8))) | ((texWinOffset.x & texWinMask.x) * 8);
			p.x = xMin + x;
			readVram(pTex, texData);
			pClut.x = (clut.elem.xCoord << 4) +
				(texData >> ((x & 0x3) << 2) & 0xf);
			pClut.y = clut.elem.yCoord;
			readVram(pClut, pallete_color.data);
			if (pallete_color.data) {
				if (param.type == TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM) {
					pallete_color.component.red = (float)pallete_color.component.red * (float)blending.component.red / (float)0x80;
					pallete_color.component.green = (float)pallete_color.component.green * (float)blending.component.green / (float)0x80;
					pallete_color.component.blue = (float)pallete_color.component.blue * (float)blending.component.blue / (float)0x80;
				}
				writeVram(p, pallete_color.data);
			}
		}
	}
}

void cxd85xxx::copyRectCpuVram(const uint32_t dest_coord, const uint32_t width_height) {
	const uint32_t x1 = (dest_coord << 16 >> 16) & 0x3ff;
	uint32_t x2 = x1 + (width_height << 16 >> 16);
	const uint32_t y1 = (dest_coord >> 16) & 0x1ff;
	uint32_t y2 = y1 + (width_height >> 16);
	uint32_t x;
	uint32_t y = y1;


	std::vector<uint16_t> buffer(collectList.size() << 1);
	for (uint32_t i = 0; i < (buffer.size() >> 1); i++) {
		buffer[i * 2] = collectList[0];
		buffer[i * 2 + 1] = collectList[0] >> 16;
		collectList.erase(collectList.begin());
	}

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

	if (!buffer.empty()) {
		std::cout << "[GPU] EMULATION PAUSED! 0xa0... command buffer not empty 0x" << std::hex << buffer.size() << std::endl;
		g_emulationPaused = true;
	}
}

void cxd85xxx::copyRectVramCpu(const uint32_t source_coord, const uint32_t width_height) {
	const uint16_t x1 = source_coord << 16 >> 16;
	const uint16_t x2 = x1 + (width_height << 16 >> 16);
	const uint16_t y1 = source_coord >> 16;
	const uint16_t y2 = y1 + (width_height >> 16);

	uint16_t data;
	std::vector<uint16_t> buffer(collectList.size() * 2);

	uint16_t x;
	uint16_t y = y1;
	do {
		x = x1;
		do {
			vertex_t position(x, y);
			readVram(position, data);
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
		collectList.push_back(temp);
	}
}

void cxd85xxx::copyRectVramVram(const uint32_t& sourceCoord, const uint32_t& destCoord, const uint32_t& widthHeight) {
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
			readVram(sourcePosition, data);
			vertex_t destPosition(destX1 + x, destY1 + y);
			writeVram(destPosition, data, true);
			x++;
		} while (x < width);
		y++;
	} while (y < height);
}

//gp0 commands

void cxd85xxx::nop(const uint32_t& commandColor) {
	/*std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";*/
}

void cxd85xxx::clearCache(const uint32_t& commandColor) {
	/*std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";*/
}

void cxd85xxx::fillRectVram(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	uint32_t size_x_y = collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.command_color1 = command;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert2.x;
	int16_t y2 = param.vert4.y;

	int16_t x = x1;
	int16_t y = y1;

	uint16_t data = format24to16Color(param.command_color1);

	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			vertex_t p(x, y);
			writeVram(p, data, true);
		}
	}
}

void cxd85xxx::unknown(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::copyRectVramVram(const uint32_t& commandColor) {
	uint32_t widthHeight = collectList.back();
	collectList.pop_back();
	uint32_t destCoord = collectList.back();
	collectList.pop_back();
	uint32_t sourceCoord = collectList.back();
	collectList.pop_back();
	copyRectVramVram(sourceCoord, destCoord, widthHeight);
}

void cxd85xxx::copyRectCpuVram(const uint32_t& commandColor) {
	if (b_collect_param) {
		b_collect_param = false;
		width_height = collectList.back();
		collectList.pop_back();
		dest_coord = collectList.back();
		collectList.pop_back();
		//collect = ((((width_height >> 16) - 1) & 0x3ff) + 1) * ((((width_height << 16 >> 16) - 1) & 0x3ff) + 1) / 2 + (width_height & 0x1);
		collect = (width_height >> 16) * (width_height << 16 >> 16) / 2;
		if ((collect * 2) < ((width_height >> 16) * (width_height << 16 >> 16)))
			collect++;
	}
	else {
		copyRectCpuVram(dest_coord, width_height);
	}
}

void cxd85xxx::copyRectVramCpu(const uint32_t& commandColor) {
	width_height = collectList.back();
	collectList.pop_back();
	source_coord = collectList.back();
	collectList.pop_back();
	copyRectVramCpu(source_coord, width_height);
	b_trans_command = true;
}

void cxd85xxx::interruptRequest(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::mono3PolyOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = TRIANGLE_PARAM;
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	rasterization((param_t*)&param, PSX_MONOCHROME, PSX_OPAQUE,
		3, param.vert1, param.vert2, param.vert3);
}

void cxd85xxx::mono3PolySemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::mono4PolyOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::mono4PolySemiTransp(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::tex3PolyOpaqTexBlend(const uint32_t& commandColor) {
	/*std::cout << "~[GPU] unhandled GPU0 command {only bypass} 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";*/

	param_t param;
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;

	//just a bypass
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
}

void cxd85xxx::tex3PolyOpaqRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex3PolySemiTranspTexBlend(const uint32_t& commandColor) {
	/*std::cout << "~[GPU] unhandled GPU0 command {only bypass} 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";*/

	param_t param;
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;

	//just a bypass
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
}

void cxd85xxx::tex3PolySemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex4PolyOpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM;
	param.texCoord4 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::tex4PolyOpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
	param.texCoord4 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::tex4PolySemiTranspTexBlend(const uint32_t& commandColor) {
	//std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
	//	gp0Lookup[(commandColor >> 24)].name << "\n";

	param_t param;
	param.texCoord4 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::tex4PolySemiTranspRawTex(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
	param.texCoord4 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::shaded3PolyOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = SHADED_3_POINT_PARAM;
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.color3 = (psxColor24_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.color2 = (psxColor24_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	rasterization((param_t*)&param, PSX_MONOCHROME, PSX_OPAQUE,
		3, param.vert1, param.vert2, param.vert3, (0, 0));
}

void cxd85xxx::shaded3PolySemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shaded4PolyOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = SHADED_4_POINT_PARAM;
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.color4 = (psxColor24_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.color3 = (psxColor24_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.color2 = (psxColor24_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	rasterization((param_t*)&param, PSX_SHADED, PSX_OPAQUE,
		4, param.vert1, param.vert2, param.vert3, param.vert4);
}

void cxd85xxx::shaded4PolySemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedTex3PolyOpaqTexBlend(const uint32_t& commandColor) {

	param_t param;
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;

	//just a bypass
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
}

void cxd85xxx::shadedTex3PolySemiTranspTexBlend(const uint32_t& commandColor) {
	param_t param;
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;

	//just a bypass
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.texDis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
}

void cxd85xxx::shadedTex4PolyOpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	param.texCoord4 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert4 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord3 = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert3 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::shadedTex4PolySemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::undocumented(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoLineOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = MONOCHROME_LINE_PARAM;
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	drawMonochromeLine(param);
}

void cxd85xxx::monoLineSemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoPolyLineOpaq(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoPolyLineSemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedLineOpaq(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedLineSemiTransp(const uint32_t& commandColor) {
	param_t param;
	param.vert2 = (vertex_t)collectList.back();
	collectList.pop_back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;
	drawMonochromeLine(param);
}

void cxd85xxx::shadedPolyLineOpaq(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedPolyLineSemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoRectVarOpaq(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t((size_x_y << 16 >> 16), (size_x_y >> 16));
	param.vert3 = param.vert1 + vertex_t(0, (size_x_y >> 16));
	param.vert2 = param.vert1 + vertex_t((size_x_y << 16 >> 16), 0);
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::monoRectVarSemiTransp(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t((size_x_y << 16 >> 16), (size_x_y >> 16));
	param.vert3 = param.vert1 + vertex_t(0, (size_x_y >> 16));
	param.vert2 = param.vert1 + vertex_t((size_x_y << 16 >> 16), 0);
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::monoRect1x1Opaq(const uint32_t& commandColor) {
	param_t param;
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.command_color1 = command;

	uint16_t color16 = format24to16Color(param.command_color1);
	writeVram(param.vert1, color16, false);
}

void cxd85xxx::monoRect1x1SemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoRect8x8Opaq(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoRect8x8SemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoRect16x16Opaq(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::monoRect16x16SemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRectVarOpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM;

	uint32_t size_x_y = collectList.back();
	collectList.pop_back();

	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();

	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarOpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_RECT_OPAQUE_RAW_PARAM;

	uint32_t size_x_y = collectList.back();
	collectList.pop_back();

	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();

	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);

	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;

	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarSemiTranspTexBlend(const uint32_t& commandColor) {
	param_t param;

	param.type = TEXTURED_RECT_OPAQUE_RAW_PARAM;

	uint32_t size_x_y = collectList.back();
	collectList.pop_back();

	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	//calculating texture coodriantes
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	//calculating verteces
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);

	param.command_color1 = command;

	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarSemiTranspRawTex(const uint32_t& commandColor) {
	param_t param;

	param.type = TEXTURED_RECT_OPAQUE_RAW_PARAM;

	uint32_t size_x_y = collectList.back();
	collectList.pop_back();

	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	//calculating texture coodriantes
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	//calculating verteces
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);

	param.command_color1 = command;

	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect1x1OpaqTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect1x1OpaqRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect1x1SemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect1x1SemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect8x8OpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00080008;
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect8x8OpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00080008;
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect8x8SemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect8x8SemiTranspRawTex(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00080008;
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect16x16OpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00100010;
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect16x16OpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00100010;
	param.texCoord1Palette = (texcoordData_t)collectList.back();
	collectList.pop_back();
	param.vert1 = (vertex_t)collectList.back();
	collectList.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.texDis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.texPageColors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.texPageColors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.texPageColors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect16x16SemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect16x16SemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::drawModeSetting(const uint32_t& commandColor) {
	gpuStat.data &= 0xffff7800;
	gpuStat.data |= commandColor & 0x000087ff;
	gpuStat.elem.texDis = commandColor & 0x00008000 ? 1 : 0;
}

void cxd85xxx::texWindowSetting(const uint32_t& commandColor) {
	texWinMask.x = commandColor << 27 >> 27;
	texWinMask.y = commandColor << 22 >> 27;
	texWinOffset.x = commandColor << 17 >> 27;
	texWinOffset.y = commandColor << 12 >> 27;
}

void cxd85xxx::setDrawAreaTopLeft(const uint32_t& commandColor) {
	drawAreaTopLeft.x = commandColor << 22 >> 22;
	drawAreaTopLeft.y = commandColor << 13 >> 23;
	gp1TopLeft = commandColor << 8 >> 8;
}

void cxd85xxx::setDrawAreaBottomRight(const uint32_t& commandColor) {
	drawAreaBottomRight.x = commandColor << 22 >> 22;
	drawAreaBottomRight.y = commandColor << 13 >> 23;
	gp1BottomRight = commandColor << 8 >> 8;
}

void cxd85xxx::setDrawOffset(const uint32_t& commandColor) {
	drawOffset.x = (commandColor & 0x7ff) << 5;
	drawOffset.y = ((commandColor >> 11) & 0x7ff) << 5;
	gp1DrawOffset = commandColor << 8 >> 8;
}

void cxd85xxx::maskBitSetting(const uint32_t& commandColor) {
	gpuStat.elem.setMaskBitWhileDrawing = commandColor & 0x1;
	gpuStat.elem.drawPixels = (commandColor & 0x2 ? 1 : 0);
}