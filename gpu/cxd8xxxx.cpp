#include "cxd85xxx.h"
#include "../bus/bus_interface.h"

extern bool isEmulationPaused;

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

	cpuFrameClocks++;
	cpuScanlineClocks++;

	if (cpuFrameClocks == CPU_CLOCKS_PER_FRAME) {

		cpuFrameClocks = 0;

		//V-Sync interrupt is naturally edge trigered
		pBus->pCp0->interruptHandler(_V_SYNC);

		if (gpuStat.elem.vertical_res && gpuStat.elem.vertical_interlace)
			gpuStat.elem.drawing_even_odd_lines = ~gpuStat.elem.drawing_even_odd_lines;

		scanline = 0;

		//Vblank synchronization timer 1
		pBus->timer1.clock(VBLANK_CLOCK);
	}

	if (cpuScanlineClocks == CPU_CLOCKS_PER_SCANLINE) {
		cpuScanlineClocks = 0;
		if (!gpuStat.elem.vertical_res) {
			scanline = 0;
			gpuStat.elem.drawing_even_odd_lines = ~gpuStat.elem.drawing_even_odd_lines;
		}
		scanline++;

		//Hblank clocking timer 1
		pBus->timer1.clock(HBLANK_CLOCK);
	}
}

void cxd85xxx::readGpu32(const uint32_t& addr, uint32_t& data) {
	data = 0;
	switch (addr) {
	case 0x1f801810:
		if (b_trans_command) {
			data = collect_list[0];
			collect_list.erase(collect_list.begin());
			if (collect_list.empty())
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
			collect_list.push_back(fifo.back());
			collect--;
		}
		else {
			command = fifo.back();
			if (collect_list.size() != 0) {
				std::cout << "[GPU] EMULATION PAUSED! collect list size != 0  0x" << std::hex << collect_list.size() << std::endl;
				isEmulationPaused = true;
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
			gpuStat.elem.irq1 = RESET;
		case 0x3:
			gpuStat.elem.display_enable = data & 0x1;
			break;
		case 0x4:
			gpuStat.elem.dma_dir = gp1 & 0x3;
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
			gpuStat.elem.horizontal_res_1 = gp1 & 0x3;
			gpuStat.elem.vertical_res = (gp1 & 0x4 ? 1 : 0);
			gpuStat.elem.video_mode = (gp1 & 0x8 ? 1 : 0);
			gpuStat.elem.display_area_color_depth = (gp1 & 0x10 ? 1 : 0);
			gpuStat.elem.vertical_interlace = (gp1 & 0x20 ? 1 : 0);
			gpuStat.elem.horizontal_res_2 = gp1 & 0x40;
			gpuStat.elem.reverseflag = (gp1 & 0x80 ? 1 : 0);
			if (gpuStat.elem.vertical_interlace)
				gpuStat.elem.interlace_field = 1;
			else
				gpuStat.elem.interlace_field = 0;
			break;
		case 0x10:
			switch (gp1 & 0x7) {
			case 3:
				gp1Result = gp1_top_left;
				b_gp1Result = true;
				break;
			case 4:
				gp1Result = gp1_bottom_right;
				b_gp1Result = true;
				break;
			case 5:
				gp1Result = gp1_draw_offset;
				b_gp1Result = true;
				break;
			case 7:
				std::cout << "~[GPU] unhandled GPU1 command 10 subfunction : 0x" << std::hex << data << std::endl;
				break;
			default:
				std::cout << "~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x" << std::hex << data << std::endl;
				isEmulationPaused = true;
				break;
			}
			break;
		default:
			std::cout << "~[GPU] unhandled GPU1 command: 0x" << std::hex << data << std::endl;
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

void cxd85xxx::writeVram(const vertex_t& possition, const uint16_t& data, const bool bIgnoreDrawingArea) {
	if (bIgnoreDrawingArea) {
		vram[possition.x * 2 * size_y + possition.y] = data << 8 >> 8;
		vram[(possition.x * 2 + 1) * size_y + possition.y] = data >> 8;
		updateVramView(possition, data);
		return;
	}

	if (((possition.x + drawOffset.x) >= drawAreaTopLeft.x) &&
		((possition.x + drawOffset.x) <= drawAreaBottomRight.x) &&
		((possition.y + drawOffset.y) >= drawAreaTopLeft.y) &&
		((possition.y + drawOffset.y) <= drawAreaBottomRight.y)) {

		vram[(possition.x + drawOffset.x) * 2 * size_y + (possition.y + drawOffset.y)] = data << 8 >> 8;
		vram[((possition.x + drawOffset.x) * 2 + 1) * size_y + (possition.y + drawOffset.y)] = data >> 8;
		vertex_t p = { 0, 0 };
		p.x = possition.x + drawOffset.x;
		p.y = possition.y + drawOffset.y;
		updateVramView(p, data);
	}

	/*vram[((int16_t)possition.x + (int16_t)drawing_offset.x) * 2 * size_y + ((int16_t)possition.y + (int16_t)drawing_offset.y)] = data << 8 >> 8;
	vram[(((int16_t)possition.x + (int16_t)drawing_offset.x) * 2 + 1) * size_y + ((int16_t)possition.y + (int16_t)drawing_offset.y)] = data >> 8;
	updateVramView(possition, data);*/

	/*updateVramView(possition, data);
	vram[possition.x * 2 * size_y + possition.y] = data << 8 >> 8;
	vram[(possition.x * 2 + 1) * size_y + possition.y] = data >> 8;*/
}

void cxd85xxx::readVram(const vertex_t& position, uint16_t& data) {
	data = vram[position.x * 2 * size_y + position.y];
	data |= vram[(position.x * 2 + 1) * size_y + position.y] << 8;
}

olc::Sprite* cxd85xxx::getVram() {
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

float cxd85xxx::edgeFunc(vertex_t v1, vertex_t v2, vertex_t p) {
	return ((float)v1.y - (float)v2.y) * ((float)p.x - (float)v1.x) +
		((float)v1.x - (float)v2.x) * ((float)v1.y - (float)p.y);
}

void cxd85xxx::CheckDrawingOffset(vertex_t& v1, vertex_t& v2, vertex_t& v3, vertex_t& v4) {
	//???
	/*if ((uint16_t)v1.x < (uint16_t)v1Draw.x) v1.x = v1Draw.x;
	if ((uint16_t)v1.y < (uint16_t)v1Draw.y) v1.y = v1Draw.y;
	if ((uint16_t)v2.x < (uint16_t)v1Draw.x) v2.x = v1Draw.x;
	if ((uint16_t)v2.y < (uint16_t)v1Draw.y) v2.y = v1Draw.y;
	if ((uint16_t)v3.x < (uint16_t)v1Draw.x) v3.x = v1Draw.x;
	if ((uint16_t)v3.y < (uint16_t)v1Draw.y) v3.y = v1Draw.y;
	if ((uint16_t)v4.x < (uint16_t)v1Draw.x) v4.x = v1Draw.x;
	if ((uint16_t)v4.y < (uint16_t)v1Draw.y) v4.y = v1Draw.y;

	if ((uint16_t)v1.x > v4Draw.x) v1.x = v4Draw.x;
	if ((uint16_t)v1.y > v4Draw.y) v1.y = v4Draw.y;
	if ((uint16_t)v2.x > v4Draw.x) v2.x = v4Draw.x;
	if ((uint16_t)v2.y > v4Draw.y) v2.y = v4Draw.y;
	if ((uint16_t)v3.x > v4Draw.x) v3.x = v4Draw.x;
	if ((uint16_t)v3.y > v4Draw.y) v3.y = v4Draw.y;
	if ((uint16_t)v4.x > v4Draw.x) v4.x = v4Draw.x;
	if ((uint16_t)v4.y > v4Draw.y) v4.y = v4Draw.y;*/
}

void cxd85xxx::rasterization(param_t* p_param, const RasterizationModes& color_mode, const RasterizationModes& transparency_mode,
	const uint8_t& vertex_count, vertex_t v1, vertex_t v2, vertex_t v3, vertex_t v4) {

	if (transparency_mode == PSX_OPAQUE) {
		if (vertex_count == 4) {
			vertex_t p;
			int16_t arrayX[4] = { v1.x, v2.x, v3.x, v4.x };
			int16_t arrayY[4] = { v1.y, v2.y, v3.y, v4.y };
			int16_t xMin = psx::gpu::min(arrayX, 4);
			int16_t xMax = psx::gpu::max(arrayX, 4);
			int16_t yMin = psx::gpu::min(arrayY, 4);
			int16_t yMax = psx::gpu::max(arrayY, 4);
			psxColor24_t c1 = p_param->command_color1;
			psxColor24_t c2 = p_param->color2;
			psxColor24_t c3 = p_param->color3;
			psxColor24_t c4 = p_param->color4;
			psxColor24_t color;

			float P123 = edgeFunc(v1, v2, v3) / 2;
			float P432 = edgeFunc(v4, v3, v2) / 2;
			for (int x = xMin; x <= xMax; x++) {
				p.x = x;
				for (int y = yMin; y <= yMax; y++) {
					p.y = y;
					float e12 = edgeFunc(v1, v2, p);
					float e23 = edgeFunc(v2, v3, p);
					float e32 = edgeFunc(v3, v2, p);
					float e31 = edgeFunc(v3, v1, p);
					float e43 = edgeFunc(v4, v3, p);
					float e24 = edgeFunc(v2, v4, p);
					if ((e12 >= 0) && (e31 >= 0) && (e23 >= 0)) {
						float lamda1 = e23 / 2. / P123;
						float lamda2 = e31 / 2. / P123;
						float lamda3 = e12 / 2. / P123;
						color.component.red = (float)c1.component.red * lamda1 + (float)c2.component.red * lamda2 + (float)c3.component.red * lamda3;
						color.component.green = (float)c1.component.green * lamda1 + (float)c2.component.green * lamda2 + (float)c3.component.green * lamda3;
						color.component.blue = (float)c1.component.blue * lamda1 + (float)c2.component.blue * lamda2 + (float)c3.component.blue * lamda3;
						writeVram(p, format24to16Color(color.data));
					}
					else if ((e32 >= 0) && (e24 >= 0) && (e43 >= 0)) {
						float lamda2 = e43 / 2. / P432;
						float lamda3 = e24 / 2. / P432;
						float lamda4 = e32 / 2. / P432;
						color.component.red = (float)c2.component.red * lamda2 + (float)c3.component.red * lamda3 + (float)c4.component.red * lamda4;
						color.component.green = (float)c2.component.green * lamda2 + (float)c3.component.green * lamda3 + (float)c4.component.green * lamda4;
						color.component.blue = (float)c2.component.blue * lamda2 + (float)c3.component.blue * lamda3 + (float)c4.component.blue * lamda4;
						writeVram(p, format24to16Color(color.data));
					}
				}
			}
		}
		else if (vertex_count == 3) {
			vertex_t p;
			int16_t arrayX[4] = { v1.x, v2.x, v3.x };
			int16_t arrayY[4] = { v1.y, v2.y, v3.y };
			int16_t xMin = psx::gpu::min(arrayX, 3);
			int16_t xMax = psx::gpu::max(arrayX, 3);
			int16_t yMin = psx::gpu::min(arrayY, 3);
			int16_t yMax = psx::gpu::max(arrayY, 3);
			psxColor24_t c1 = p_param->command_color1;
			psxColor24_t c2 = p_param->color2;
			psxColor24_t c3 = p_param->color3;

			psxColor24_t color;
			float P123 = edgeFunc(v1, v2, v3) / 2;
			if (P123 > 0) {
				for (int x = xMin; x <= xMax; x++) {
					p.x = x;
					for (int y = yMin; y <= yMax; y++) {
						p.y = y;
						float e12 = edgeFunc(v1, v2, p);
						float e23 = edgeFunc(v2, v3, p);
						float e31 = edgeFunc(v3, v1, p);
						vertex_t e1 = v3 - v2;
						vertex_t e2 = v1 - v3;
						vertex_t e3 = v2 - v1;
						if ((e12 > 0) || ((e12 == 0) ? ((e3.y == 0) && (e3.x < 0)) || (e3.y < 0) : false))
							if ((e23 > 0) || ((e23 == 0) ? ((e1.y == 0) && (e1.x < 0)) || (e1.y < 0) : false))
								if ((e31 > 0) || ((e31 == 0) ? ((e2.y == 0) && (e2.x < 0)) || (e2.y < 0) : false)) {
									float lamda1 = e23 / 2. / P123;
									float lamda2 = e31 / 2. / P123;
									float lamda3 = e12 / 2. / P123;
									color.component.red = (float)c1.component.red * lamda1 + (float)c2.component.red * lamda2 + (float)c3.component.red * lamda3;
									color.component.green = (float)c1.component.green * lamda1 + (float)c2.component.green * lamda2 + (float)c3.component.green * lamda3;
									color.component.blue = (float)c1.component.blue * lamda1 + (float)c2.component.blue * lamda2 + (float)c3.component.blue * lamda3;
									writeVram(p, format24to16Color(color.data));
								}
					}
				}
			}
			else {
				P123 = -P123;
				for (int x = xMin; x <= xMax; x++) {
					p.x = x;
					for (int y = yMin; y <= yMax; y++) {
						p.y = y;
						float e21 = edgeFunc(v2, v1, p);
						float e32 = edgeFunc(v3, v2, p);
						float e13 = edgeFunc(v1, v3, p);
						vertex_t e1 = v3 - v2;
						vertex_t e2 = v1 - v3;
						vertex_t e3 = v2 - v1;
						if ((e21 > 0) || ((e21 == 0) ? ((e3.y == 0) && (e3.x > 0)) || (e3.y > 0) : false))
							if ((e32 > 0) || ((e32 == 0) ? ((e1.y == 0) && (e1.x > 0)) || (e1.y > 0) : false))
								if ((e13 > 0) || ((e13 == 0) ? ((e2.y == 0) && (e2.x > 0)) || (e2.y > 0) : false)) {
									float lamda1 = e32 / 2. / P123;
									float lamda2 = e13 / 2. / P123;
									float lamda3 = e21 / 2. / P123;
									color.component.red = (float)c1.component.red * lamda1 + (float)c2.component.red * lamda2 + (float)c3.component.red * lamda3;
									color.component.green = (float)c1.component.green * lamda1 + (float)c2.component.green * lamda2 + (float)c3.component.green * lamda3;
									color.component.blue = (float)c1.component.blue * lamda1 + (float)c2.component.blue * lamda2 + (float)c3.component.blue * lamda3;
									writeVram(p, format24to16Color(color.data));
								}
					}
				}
			}
		}
	}
}

void cxd85xxx::drawMonochromeRect(param_t param) {
	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert2.x;
	int16_t y2 = param.vert4.y;

	int16_t x = x1;
	int16_t y = y1;

	x1 += drawOffset.x;
	y1 += drawOffset.y;
	x2 += drawOffset.x;
	y2 += drawOffset.y;

	uint16_t data = format24to16Color(param.command_color1);

	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			vertex_t position(x, y);
			writeVram(position, data, false);
		}
	}
	return;
}

void cxd85xxx::drawMonochromeLine(param_t param) {

	int16_t arrayX[2] = { param.vert1.x, param.vert2.x };
	int16_t arrayY[2] = { param.vert1.y, param.vert2.y };
	int16_t xMin = psx::gpu::min(arrayX, 2);
	int16_t yMin = psx::gpu::min(arrayY, 2);
	int16_t xMax = psx::gpu::max(arrayX, 2);
	int16_t yMax = psx::gpu::max(arrayY, 2);

	vertex_t p;
	for (int x = xMin; x <= xMax; x++) {
		p.x = x;
		for (int y = yMin; y <= yMax; y++) {
			p.y = y;
			float e12 = edgeFunc(param.vert1, param.vert2, p);
			if (e12 == 0)
				writeVram(p, format24to16Color(param.command_color1));
		}
	}
}

void cxd85xxx::drawTexRect15Bit(param_t param) {
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.tex_dis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };
	int16_t xMin = psx::gpu::min(arrayX, 4);
	int16_t yMin = psx::gpu::min(arrayY, 4);
	int16_t xMax = psx::gpu::max(arrayX, 4);
	int16_t yMax = psx::gpu::max(arrayY, 4);
	uint16_t texData;
	uint16_t data = 0;
	vertex_t pTex;
	vertex_t p;
	int16_t arrayTexX[4] = { param.texCoord1Palette.elem.x, param.texCoord2TexPage.elem.x,
		param.texCoord3.elem.x, param.texCoord4.elem.x };
	int16_t arrayTexY[4] = { param.texCoord1Palette.elem.y, param.texCoord2TexPage.elem.y,
		param.texCoord3.elem.y, param.texCoord4.elem.y };
	int16_t xTexMin = psx::gpu::min(arrayTexX, 4);
	int16_t yTexMin = psx::gpu::min(arrayTexY, 4);
	int16_t xTexMax = psx::gpu::max(arrayTexX, 4);
	int16_t yTexMax = psx::gpu::max(arrayTexY, 4);

	float ratioX = ((float)(xTexMax - xTexMin) / (float)(xMax - xMin));
	float ratioY = ((float)(yTexMax - yTexMin) / (float)(yMax - yMin));

	for (int y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.tex_page_y_base * 256 + yTexMin + (uint32_t)((float)y * ratioY);
		p.y = yMin + y;
		for (int x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.tex_page_x_base * 64 + xTexMin + (uint32_t)((float)x * ratioX);
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
	gpuStat.elem.tex_dis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };
	int16_t xMin = psx::gpu::min(arrayX, 4);
	int16_t yMin = psx::gpu::min(arrayY, 4);
	int16_t xMax = psx::gpu::max(arrayX, 4);
	int16_t yMax = psx::gpu::max(arrayY, 4);

	uint16_t texData;
	vertex_t pTex;
	int16_t arrayTexX[4] = { param.texCoord1Palette.elem.x, param.texCoord2TexPage.elem.x,
		param.texCoord3.elem.x, param.texCoord4.elem.x };
	int16_t arrayTexY[4] = { param.texCoord1Palette.elem.y, param.texCoord2TexPage.elem.y,
		param.texCoord3.elem.y, param.texCoord4.elem.y };
	int16_t xTexMin = psx::gpu::min(arrayTexX, 4);
	int16_t yTexMin = psx::gpu::min(arrayTexY, 4);
	int16_t xTexMax = psx::gpu::max(arrayTexX, 4);
	int16_t yTexMax = psx::gpu::max(arrayTexY, 4);

	psxColor24_t blending = param.command_color1;

	vertex_t p;
	vertex_t pClut;
	psxColor16_t pallete_color = 0;

	float ratioX = ((float)(xTexMax - xTexMin) / (float)(xMax - xMin));
	float ratioY = ((float)(yTexMax - yTexMin) / (float)(yMax - yMin));

	for (int y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.tex_page_y_base * 256 + yTexMin + (uint32_t)((float)y * ratioY);
		p.y = yMin + y;
		for (int x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.tex_page_x_base * 64 + (xTexMin >> 1) + ((uint32_t)((float)x * ratioX) >> 1);
			p.x = xMin + x;
			readVram(pTex, texData);
			pClut.x = clut.elem.xCoord * 16 +
				(texData >> (((uint32_t)((float)x * ratioX) & 0x1) * 8) & 0xff);
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
	gpuStat.elem.tex_dis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;

	int16_t arrayX[4] = { param.vert1.x, param.vert2.x, param.vert3.x, param.vert4.x };
	int16_t arrayY[4] = { param.vert1.y, param.vert2.y, param.vert3.y, param.vert4.y };

	int16_t xMin = psx::gpu::min(arrayX, 4);
	int16_t yMin = psx::gpu::min(arrayY, 4);
	int16_t xMax = psx::gpu::max(arrayX, 4);
	int16_t yMax = psx::gpu::max(arrayY, 4);

	uint16_t texData;
	psxColor16_t pallete_color = 0;
	vertex_t pTex;
	vertex_t p;
	vertex_t pClut;

	int16_t xTexMin = param.texCoord1Palette.elem.x;
	int16_t yTexMin = param.texCoord1Palette.elem.y;
	psxColor24_t blending = param.command_color1;

	float ratioX = 1.;
	float ratioY = 1.;

	for (int y = 0; y < yMax - yMin; y++) {
		pTex.y = gpuStat.elem.tex_page_y_base * 256 + yTexMin + (uint32_t)((float)y * ratioY);
		pTex.y = (pTex.y & (~(tex_win_mask.y * 8))) | ((tex_win_offset.y & tex_win_mask.y) * 8);
		p.y = yMin + y;
		for (int x = 0; x < xMax - xMin; x++) {
			pTex.x = gpuStat.elem.tex_page_x_base * 64 + (xTexMin >> 2) + ((uint32_t)((float)x * ratioX) >> 2);
			pTex.x = (pTex.x & (~(tex_win_mask.x * 8))) | ((tex_win_offset.x & tex_win_mask.x) * 8);
			p.x = xMin + x;
			readVram(pTex, texData);
			pClut.x = clut.elem.xCoord * 16 +
				(texData >> (((uint32_t)((float)x * ratioX) & 0x3) * 4) & 0xf);
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
	const int x1 = (dest_coord << 16 >> 16) & 0x3ff;
	int x2 = x1 + (width_height << 16 >> 16);
	const int y1 = (dest_coord >> 16) & 0x1ff;
	int y2 = y1 + (width_height >> 16);
	int x;
	int y = y1;


	std::vector<uint16_t> buffer(collect_list.size() * 2);
	for (int i = 0; i < buffer.size(); i += 2) {
		buffer[i] = collect_list[i / 2] << 16 >> 16;
		buffer[i + 1] = collect_list[i / 2] >> 16;
		collect_list.pop_back();
	}

	do {
		x = x1;
		do {
			vertex_t position(x, y);
			writeVram(position, buffer[0], true);
			buffer.erase(buffer.begin());
			x++;
		} while (x < x2);
		y++;
	} while (y < y2);
}

void cxd85xxx::copyRectVramCpu(const uint32_t source_coord, const uint32_t width_height) {
	const int x1 = source_coord << 16 >> 16;
	const int x2 = x1 + (width_height << 16 >> 16);
	const int y1 = source_coord >> 16;
	const int y2 = y1 + (width_height >> 16);
	int x;
	int y = y1;

	uint16_t data;
	std::vector<uint16_t> buffer(collect_list.size() * 2);

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
		collect_list.push_back(temp);
	}
}

//gp0 commands

void cxd85xxx::nop(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::clearCache(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::fillRectVram(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.command_color1 = command;
	command = 0;

	int16_t x1 = param.vert1.x;
	int16_t y1 = param.vert1.y;
	int16_t x2 = param.vert2.x;
	int16_t y2 = param.vert4.y;

	int16_t x = x1;
	int16_t y = y1;

	uint16_t data = format24to16Color(param.command_color1);

	for (int16_t y = y1; y < y2; y++) {
		for (int16_t x = x1; x < x2; x++) {
			vertex_t position(x, y);
			writeVram(position, data, true);
		}
	}
}

void cxd85xxx::unknown(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::copyRectVramVram(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::copyRectCpuVram(const uint32_t& commandColor) {
	if (b_collect_param) {
		b_collect_param = false;
		width_height = collect_list.back();
		collect_list.pop_back();
		dest_coord = collect_list.back();
		collect_list.pop_back();
		//collect = ((((width_height >> 16) - 1) & 0x3ff) + 1) * ((((width_height << 16 >> 16) - 1) & 0x3ff) + 1) / 2 + (width_height & 0x1);
		collect = (width_height >> 16) * (width_height << 16 >> 16) / 2;
		collect += collect & 0x1;
	}
	else {
		copyRectCpuVram(dest_coord, width_height);
	}
}

void cxd85xxx::copyRectVramCpu(const uint32_t& commandColor) {
	width_height = collect_list.back();
	collect_list.pop_back();
	source_coord = collect_list.back();
	collect_list.pop_back();
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
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
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
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::mono4PolySemiTransp(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::tex3PolyOpaqTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";

	param_t param;
	param.texCoord3 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	param.color3 = command;

	//just a bypass
	gpuStat.data = (gpuStat.data & 0xfffffe00) | (param.texCoord2TexPage.elem.attribute) << 7 >> 7;
	gpuStat.elem.tex_dis = (param.texCoord2TexPage.elem.attribute & 0x800) ? 0 : 1;
}

void cxd85xxx::tex3PolyOpaqRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex3PolySemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex3PolySemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex4PolyOpaqTexBlend(const uint32_t& commandColor) {
	param_t param;
	switch (command >> 24) {
	case 0x2c:
		param.type = TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM;
		break;
	case 0x2d:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	case 0x2f:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	}
	param.texCoord4 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord3 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		//	//throw std::runtime_error("unhandled texture page color 8 bit mode!");
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::tex4PolyOpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	switch (command >> 24) {
	case 0x2c:
		param.type = TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM;
		break;
	case 0x2d:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	case 0x2f:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	}
	param.texCoord4 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord3 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		//	//throw std::runtime_error("unhandled texture page color 8 bit mode!");
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::tex4PolySemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::tex4PolySemiTranspRawTex(const uint32_t& commandColor) {
	param_t param;
	switch (command >> 24) {
	case 0x2c:
		param.type = TEXTURED_4_POINT_OPAQUE_BLENDING_PARAM;
		break;
	case 0x2d:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	case 0x2f:
		param.type = TEXTURED_4_POINT_OPAQUE_RAW_PARAM;
		break;
	}
	param.texCoord4 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord3 = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord2TexPage = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	param.color3 = command;
	if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x100)
		drawTexRect15Bit(param);
	else if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x80)
		//	//throw std::runtime_error("unhandled texture page color 8 bit mode!");
		drawTexRect8Bit(param);
	else  if ((param.texCoord2TexPage.elem.attribute & 0x180) == 0x0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::shaded3PolyOpaq(const uint32_t& commandColor) {
	param_t param;
	param.type = SHADED_3_POINT_PARAM;
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.color3 = (psxColor24_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.color2 = (psxColor24_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
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
	param.vert4 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.color4 = (psxColor24_t)collect_list.back();
	collect_list.pop_back();
	param.vert3 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.color3 = (psxColor24_t)collect_list.back();
	collect_list.pop_back();
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.color2 = (psxColor24_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.command_color1 = command;
	rasterization((param_t*)&param, PSX_SHADED, PSX_OPAQUE,
		4, param.vert1, param.vert2, param.vert3, param.vert4);
}

void cxd85xxx::shaded4PolySemiTransp(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedTex3PolyOpaqTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedTex3PolySemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::shadedTex4PolyOpaqTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
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
	param.vert2 = (vertex_t)collect_list.back();
	collect_list.pop_back();;
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
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
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
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

	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t((size_x_y << 16 >> 16), (size_x_y >> 16));
	param.vert3 = param.vert1 + vertex_t(0, (size_x_y >> 16));
	param.vert2 = param.vert1 + vertex_t((size_x_y << 16 >> 16), 0);
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::monoRectVarSemiTransp(const uint32_t& commandColor) {
	param_t param;

	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t((size_x_y << 16 >> 16), (size_x_y >> 16));
	param.vert3 = param.vert1 + vertex_t(0, (size_x_y >> 16));
	param.vert2 = param.vert1 + vertex_t((size_x_y << 16 >> 16), 0);
	param.command_color1 = command;
	drawMonochromeRect(param);
}

void cxd85xxx::monoRect1x1Opaq(const uint32_t& commandColor) {
	param_t param;
	param.type = RECT_PARAM;
	uint32_t size_x_y = 0x00010001;
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.command_color1 = command;
	drawMonochromeRect(param);
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

	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();

	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();

	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.tex_dis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.tex_page_colors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.tex_page_colors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.tex_page_colors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarOpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	param.type = TEXTURED_RECT_OPAQUE_RAW_PARAM;

	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();

	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();

	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);

	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.tex_dis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;

	param.command_color1 = command;

	if (gpuStat.elem.tex_page_colors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.tex_page_colors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.tex_page_colors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarSemiTranspTexBlend(const uint32_t& commandColor) {
	param_t param;

	param.type = TEXTURED_RECT_OPAQUE_RAW_PARAM;

	uint32_t size_x_y = collect_list.back();
	collect_list.pop_back();

	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	//calculating texture coodriantes
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	//calculating verteces
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);

	param.command_color1 = command;

	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.tex_dis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;

	if (gpuStat.elem.tex_page_colors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.tex_page_colors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.tex_page_colors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRectVarSemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
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
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.tex_dis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.tex_page_colors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.tex_page_colors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.tex_page_colors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect8x8OpaqRawTex(const uint32_t& commandColor) {
	param_t param;
	uint32_t size_x_y = 0x00080008;
	param.texCoord1Palette = (texcoordData_t)collect_list.back();
	collect_list.pop_back();
	param.vert1 = (vertex_t)collect_list.back();
	collect_list.pop_back();
	param.vert4 = param.vert1 + vertex_t(size_x_y << 16 >> 16, size_x_y >> 16);
	param.vert3 = param.vert1 + vertex_t(0, size_x_y >> 16);
	param.vert2 = param.vert1 + vertex_t(size_x_y << 16 >> 16, 0);
	param.texCoord2TexPage.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord2TexPage.elem.y = param.texCoord1Palette.elem.y;
	param.texCoord2TexPage.elem.attribute = (uint16_t)gpuStat.data & 0x01ff;
	param.texCoord2TexPage.elem.attribute = (gpuStat.elem.tex_dis) ?
		param.texCoord2TexPage.elem.attribute & 0xfffff7ff : param.texCoord2TexPage.elem.attribute | 0x800;
	param.texCoord3.elem.x = param.texCoord1Palette.elem.x;
	param.texCoord3.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);

	param.texCoord4.elem.x = param.texCoord1Palette.elem.x + (size_x_y << 16 >> 16);
	param.texCoord4.elem.y = param.texCoord1Palette.elem.y + (size_x_y >> 16);
	param.command_color1 = command;

	if (gpuStat.elem.tex_page_colors == 2)
		drawTexRect15Bit(param);
	else if (gpuStat.elem.tex_page_colors == 1)
		drawTexRect8Bit(param);
	else  if (gpuStat.elem.tex_page_colors == 0)
		drawTexRect4Bit(param);
	else
		throw std::runtime_error("texture page color mode RESERVED!");
}

void cxd85xxx::texRect8x8SemiTranspTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect8x8SemiTranspRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect16x16OpaqTexBlend(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
}

void cxd85xxx::texRect16x16OpaqRawTex(const uint32_t& commandColor) {
	std::cout << "~[GPU] unhandled GPU0 command 0x" << commandColor << " " <<
		gp0Lookup[(commandColor >> 24)].name << "\n";
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
	gpuStat.data |= commandColor << 21 >> 21;
	gpuStat.elem.tex_dis = (commandColor & 0x00000800 ? 1 : 0);
}

void cxd85xxx::texWindowSetting(const uint32_t& commandColor) {
	tex_win_mask.x = commandColor << 27 >> 27;
	tex_win_mask.y = commandColor << 22 >> 27;
	tex_win_offset.x = commandColor << 17 >> 27;
	tex_win_offset.y = commandColor << 12 >> 27;
	if (commandColor << 8) {
		std::cout << "[GPU] EMULATION PAUSED! texture win != 0";
		isEmulationPaused = true;
	}
}

void cxd85xxx::setDrawAreaTopLeft(const uint32_t& commandColor) {
	drawAreaTopLeft.x = commandColor << 22 >> 22;
	drawAreaTopLeft.y = commandColor << 13 >> 23;
	gp1_top_left = commandColor << 8 >> 8;
}

void cxd85xxx::setDrawAreaBottomRight(const uint32_t& commandColor) {
	drawAreaBottomRight.x = commandColor << 22 >> 22;
	drawAreaBottomRight.y = commandColor << 13 >> 23;
	gp1_bottom_right = commandColor << 8 >> 8;
}

void cxd85xxx::setDrawOffset(const uint32_t& commandColor) {
	drawOffset.x = commandColor << 21 >> 21;
	drawOffset.y = commandColor << 11 >> 21;
	gp1_draw_offset = commandColor << 8 >> 8;
}

void cxd85xxx::maskBitSetting(const uint32_t& commandColor) {
	gpuStat.elem.set_mask_bit_while_drawing = commandColor & 0x1;
	gpuStat.elem.draw_pixels = (commandColor & 0x2 ? 1 : 0);
}
