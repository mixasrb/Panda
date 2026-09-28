#include "cp2.h"
#include "cw33300.h"

extern bool g_emulationPaused;
extern bool g_GteInstructionsLogged;

cp2_t::cp2_t() {
	opcodeLookup.resize(0x40);
	opcodeLookup = {
		{"N/A  ",&cp2_t::XXXXX,0},{"RTPS ",&cp2_t::RTPS,15},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"NCLIP",&cp2_t::NCLIP,8},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"OP      ",&cp2_t::OP,6},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},

		{"DPCS  ",&cp2_t::DPCS,8},{"INTPL",&cp2_t::INTPL,8},{"MVMVA",&cp2_t::MVMVA,8},{"NCDS ",&cp2_t::NCDS,19},
		{"CDP   ",&cp2_t::CDP,13},{"N/A  ",&cp2_t::XXXXX,0},{"NCDT ",&cp2_t::NCDT,44},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"NCCS ",&cp2_t::NCCS,17},
		{"CC     ",&cp2_t::CC,11},{"N/A  ",&cp2_t::XXXXX,0},{"NCS   ",&cp2_t::NCS,14},{"N/A  ",&cp2_t::XXXXX,0},

		{"NCT   ",&cp2_t::NCT,30},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"SQR    ",&cp2_t::SQR,5},{"DCPL  ",&cp2_t::DCPL,8},{"DPCT ",&cp2_t::DPCT,17},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"AVSZ3",&cp2_t::AVSZ3,5},{"AVSZ4",&cp2_t::AVSZ4,6},{"N/A  ",&cp2_t::XXXXX,0},

		{"RTPT ",&cp2_t::RTPT,23},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},{"N/A  ",&cp2_t::XXXXX,0},
		{"N/A  ",&cp2_t::XXXXX,0},{"GPF    ",&cp2_t::GPF,5},{"GPL    ",&cp2_t::GPL,5},{"NCCT ",&cp2_t::NCCT,39}
	};
}

#undef R11
#undef R12
#undef R13
#undef R21
#undef R22
#undef R23
#undef R31
#undef R32
#undef R33
#undef TRX
#undef TRY
#undef TRZ
#undef L11
#undef L12
#undef L13
#undef L21
#undef L22
#undef L23
#undef L31
#undef L32
#undef L33
#undef RBK
#undef GBK
#undef BBK
#undef LR1
#undef LR2
#undef LR3
#undef LG1
#undef LG2
#undef LG3
#undef LB1
#undef LB2
#undef LB3
#undef RFC
#undef GFC
#undef BFC
#undef OFX
#undef OFY
#undef H
#undef DQA
#undef DQB
#undef ZSF3
#undef ZSF4
#undef FLAG

#undef VX0
#undef VY0
#undef VZ0
#undef VX1
#undef VY1
#undef VZ1
#undef VX2
#undef VY2
#undef VZ2
#undef R
#undef G
#undef B
#undef CODE
#undef OTZ
#undef IR0
#undef IR1
#undef IR2
#undef IR3
#undef SXY0
#undef SX0
#undef SY0
#undef SXY1
#undef SX1
#undef SY1
#undef SXY2
#undef SX2
#undef SY2
#undef SXYP
#undef SXP
#undef SYP
#undef SZ0
#undef SZ1
#undef SZ2
#undef SZ3
#undef RGB0
#undef R0
#undef G0
#undef B0
#undef CD0
#undef RGB1
#undef R1
#undef G1
#undef B1
#undef CD1
#undef RGB2
#undef R2
#undef G2
#undef B2
#undef CD2
#undef MAC0
#undef MAC1
#undef MAC2
#undef MAC3
#undef IRGB
#undef ORGB
#undef LZCS
#undef LZCR

#define R11 (gteRegisters.cp2Ctrl.p[0].shw.l)
#define R12 (gteRegisters.cp2Ctrl.p[0].shw.h)
#define R13 (gteRegisters.cp2Ctrl.p[1].shw.l)
#define R21 (gteRegisters.cp2Ctrl.p[1].shw.h)
#define R22 (gteRegisters.cp2Ctrl.p[2].shw.l)
#define R23 (gteRegisters.cp2Ctrl.p[2].shw.h)
#define R31 (gteRegisters.cp2Ctrl.p[3].shw.l)
#define R32 (gteRegisters.cp2Ctrl.p[3].shw.h)
#define R33 (gteRegisters.cp2Ctrl.p[4].shw.l)
#define TRX (gteRegisters.cp2Ctrl.p[5].sw)
#define TRY (gteRegisters.cp2Ctrl.p[6].sw)
#define TRZ (gteRegisters.cp2Ctrl.p[7].sw)
#define L11 (gteRegisters.cp2Ctrl.p[8].shw.l)
#define L12 (gteRegisters.cp2Ctrl.p[8].shw.h)
#define L13 (gteRegisters.cp2Ctrl.p[9].shw.l)
#define L21 (gteRegisters.cp2Ctrl.p[9].shw.h)
#define L22 (gteRegisters.cp2Ctrl.p[10].shw.l)
#define L23 (gteRegisters.cp2Ctrl.p[10].shw.h)
#define L31 (gteRegisters.cp2Ctrl.p[11].shw.l)
#define L32 (gteRegisters.cp2Ctrl.p[11].shw.h)
#define L33 (gteRegisters.cp2Ctrl.p[12].shw.l)
#define RBK (gteRegisters.cp2Ctrl.p[13].sw)
#define GBK (gteRegisters.cp2Ctrl.p[14].sw)
#define BBK (gteRegisters.cp2Ctrl.p[15].sw)
#define LR1 (gteRegisters.cp2Ctrl.p[16].shw.l)
#define LR2 (gteRegisters.cp2Ctrl.p[16].shw.h)
#define LR3 (gteRegisters.cp2Ctrl.p[17].shw.l)
#define LG1 (gteRegisters.cp2Ctrl.p[17].shw.h)
#define LG2 (gteRegisters.cp2Ctrl.p[18].shw.l)
#define LG3 (gteRegisters.cp2Ctrl.p[18].shw.h)
#define LB1 (gteRegisters.cp2Ctrl.p[19].shw.l)
#define LB2 (gteRegisters.cp2Ctrl.p[19].shw.h)
#define LB3 (gteRegisters.cp2Ctrl.p[20].shw.l)
#define RFC (gteRegisters.cp2Ctrl.p[21].sw)
#define GFC (gteRegisters.cp2Ctrl.p[22].sw)
#define BFC (gteRegisters.cp2Ctrl.p[23].sw)
#define OFX (gteRegisters.cp2Ctrl.p[24].sw)
#define OFY (gteRegisters.cp2Ctrl.p[25].sw)
#define H (gteRegisters.cp2Ctrl.p[26].hw.l)
#define DQA (gteRegisters.cp2Ctrl.p[27].shw.l)
#define DQB (gteRegisters.cp2Ctrl.p[28].sw)
#define ZSF3 (gteRegisters.cp2Ctrl.p[29].shw.l)
#define ZSF4 (gteRegisters.cp2Ctrl.p[30].shw.l)
#define FLAG (gteRegisters.cp2Ctrl.p[31].w)

#define VX0 (gteRegisters.cp2Data.p[0].shw.l)
#define VY0 (gteRegisters.cp2Data.p[0].shw.h)
#define VZ0 (gteRegisters.cp2Data.p[1].shw.l)
#define VX1 (gteRegisters.cp2Data.p[2].shw.l)
#define VY1 (gteRegisters.cp2Data.p[2].shw.h)
#define VZ1 (gteRegisters.cp2Data.p[3].shw.l)
#define VX2 (gteRegisters.cp2Data.p[4].shw.l)
#define VY2 (gteRegisters.cp2Data.p[4].shw.h)
#define VZ2 (gteRegisters.cp2Data.p[5].shw.l)
#define R (gteRegisters.cp2Data.p[6].b.l)
#define G (gteRegisters.cp2Data.p[6].b.h1)
#define B (gteRegisters.cp2Data.p[6].b.h2)
#define C (gteRegisters.cp2Data.p[6].b.h3)
#define OTZ (gteRegisters.cp2Data.p[7].hw.l)
#define IR0 (gteRegisters.cp2Data.p[8].shw.l)
#define IR1 (gteRegisters.cp2Data.p[9].shw.l)
#define IR2 (gteRegisters.cp2Data.p[10].shw.l)
#define IR3 (gteRegisters.cp2Data.p[11].shw.l)
#define SXY0 (gteRegisters.cp2Data.p[12].w)
#define SX0 (gteRegisters.cp2Data.p[12].shw.l)
#define SY0 (gteRegisters.cp2Data.p[12].shw.h)
#define SXY1 (gteRegisters.cp2Data.p[13].w)
#define SX1 (gteRegisters.cp2Data.p[13].shw.l)
#define SY1 (gteRegisters.cp2Data.p[13].shw.h)
#define SXY2 (gteRegisters.cp2Data.p[14].w)
#define SX2 (gteRegisters.cp2Data.p[14].shw.l)
#define SY2 (gteRegisters.cp2Data.p[14].shw.h)
#define SXYP (gteRegisters.cp2Data.p[15].w)
#define SXP (gteRegisters.cp2Data.p[15].shw.l)
#define SYP (gteRegisters.cp2Data.p[15].shw.h)
#define SZ0 (gteRegisters.cp2Data.p[16].hw.l)
#define SZ1 (gteRegisters.cp2Data.p[17].hw.l)
#define SZ2 (gteRegisters.cp2Data.p[18].hw.l)
#define SZ3 (gteRegisters.cp2Data.p[19].hw.l)
#define RGB0 (gteRegisters.cp2Data.p[20].w)
#define R0 (gteRegisters.cp2Data.p[20].b.l)
#define G0 (gteRegisters.cp2Data.p[20].b.h1)
#define B0 (gteRegisters.cp2Data.p[20].b.h2)
#define C0 (gteRegisters.cp2Data.p[20].b.h3)
#define RGB1 (gteRegisters.cp2Data.p[21].w)
#define R1 (gteRegisters.cp2Data.p[21].b.l)
#define G1 (gteRegisters.cp2Data.p[21].b.h1)
#define B1 (gteRegisters.cp2Data.p[21].b.h2)
#define C1 (gteRegisters.cp2Data.p[21].b.h3)
#define RGB2 (gteRegisters.cp2Data.p[22].w)
#define R2 (gteRegisters.cp2Data.p[22].b.l)
#define G2 (gteRegisters.cp2Data.p[22].b.h1)
#define B2 (gteRegisters.cp2Data.p[22].b.h2)
#define C2 (gteRegisters.cp2Data.p[22].b.h3)
#define MAC0 (gteRegisters.cp2Data.p[24].sw)
#define MAC1 (gteRegisters.cp2Data.p[25].sw)
#define MAC2 (gteRegisters.cp2Data.p[26].sw)
#define MAC3 (gteRegisters.cp2Data.p[27].sw)
#define LZCS (gteRegisters.cp2Data.p[30].sw)
#define LZCR (gteRegisters.cp2Data.p[31].w)

#define MX11(i) (i < 3 ? gteRegisters.cp2Ctrl.p[i << 3].shw.l : -R << 4)
#define MX12(i) (i < 3 ? gteRegisters.cp2Ctrl.p[i << 3].shw.h : R << 4)
#define MX13(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 1].shw.l : IR0)
#define MX21(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 1].shw.h : R13)
#define MX22(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 2].shw.l : R13)
#define MX23(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 2].shw.h : R13)
#define MX31(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 3].shw.l : R22)
#define MX32(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 3].shw.h : R22)
#define MX33(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 4].shw.l : R22)

#define VX(i) (i < 3 ? gteRegisters.cp2Data.p[i << 1].shw.l : IR1)
#define VY(i) (i < 3 ? gteRegisters.cp2Data.p[i << 1].shw.h : IR2)
#define VZ(i) (i < 3 ? gteRegisters.cp2Data.p[(i << 1) + 1].shw.l : IR3)
#define CV1(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 5].sw : 0)
#define CV2(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 6].sw : 0)
#define CV3(i) (i < 3 ? gteRegisters.cp2Ctrl.p[(i << 3) + 7].sw : 0)

#define GTE_SF(op) ((op >> 19) & 0x1)
#define GTE_MX(op) ((op >> 17) & 0x3) 
#define GTE_V(op) ((op >> 15) & 0x3) 
#define GTE_CV(op) ((op >> 13) & 0x3) 
#define GTE_LM(op) ((op >> 10) & 0x1) 

void cp2_t::clock() {
	if (clocks == 0)
		commandBusy = false;
	else
		clocks--;
}

uint32_t cp2_t::saturate(int32_t value, int32_t min, int32_t max, uint32_t flag) {
	if (value < min) {
		FLAG |= flag;
		return min;
	}
	if (value > max) {
		FLAG |= flag;
		return max;
	}
	return value;
}


void cp2_t::MFC2(uint8_t rt, uint8_t rd) {
	switch (rd) {
	case 1:
	case 3:
	case 5:
	case 8:
	case 9:
	case 10:
	case 11:
		gteRegisters.cp2Data.p[rd].w = (int32_t)gteRegisters.cp2Data.p[rd].shw.l;
		break;

	case 7:
	case 16:
	case 17:
	case 18:
	case 19:
		gteRegisters.cp2Data.p[rd].w = (uint32_t)gteRegisters.cp2Data.p[rd].hw.l;
		break;

	case 15:
		gteRegisters.cp2Data.p[rd].w = SXY2;
		break;
	case 28:
	case 29:
		if (commandBusy)
			clocks++;

		gteRegisters.cp2Data.p[rd].w = saturate(IR1 >> 7, 0, 0x1f, 0) |
			(saturate(IR2 >> 7, 0, 0x1f, 0) << 5) | (saturate(IR3 >> 7, 0, 0x1f, 0) << 10);
		break;
	}

	if (commandBusy)
		pCpu->gteClocks = clocks;

	uint32_t value = gteRegisters.cp2Data.p[rd].w;
	if (pCpu->cycle == 0) {
		if (pCpu->cycle0.bWrite == true)
			g_emulationPaused = true;
		pCpu->cycle0.old = pCpu->r[rt];
		pCpu->cycle0.bWrite = true;
		pCpu->cycle0.addr = rt;
		pCpu->cycle0.data = value;
	}
	else if (pCpu->cycle == 1) {
		if (pCpu->cycle1.bWrite == true)
			g_emulationPaused = true;
		pCpu->cycle1.old = pCpu->r[rt];
		pCpu->cycle1.bWrite = true;
		pCpu->cycle1.addr = rt;
		pCpu->cycle1.data = value;
	}

	//pCpu->set(rt, gteRegisters.cp2Data.p[rd].w);
}

void cp2_t::CFC2(uint8_t rt, uint8_t rd) {
	if (commandBusy)
		pCpu->gteClocks = clocks;

	uint32_t value = gteRegisters.cp2Ctrl.p[rd].w;
	if (pCpu->cycle == 0) {
		if (pCpu->cycle0.bWrite == true)
			g_emulationPaused = true;
		pCpu->cycle0.old = pCpu->r[rt];
		pCpu->cycle0.bWrite = true;
		pCpu->cycle0.addr = rt;
		pCpu->cycle0.data = value;
	}
	else if (pCpu->cycle == 1) {
		if (pCpu->cycle1.bWrite == true)
			g_emulationPaused = true;
		pCpu->cycle1.old = pCpu->r[rt];
		pCpu->cycle1.bWrite = true;
		pCpu->cycle1.addr = rt;
		pCpu->cycle1.data = value;
	}

	//pCpu->set(rt, gteRegisters.cp2Ctrl.p[rd].w);
}

void cp2_t::MTC2(uint8_t rt, uint8_t rd) {
	uint32_t value = pCpu->get(rt);

	switch (rd) {
	case 15:
		SXY0 = SXY1;
		SXY1 = SXY2;
		SXY2 = value;
		break;
	case 28:
		IR1 = (value & 0x1f) << 7;
		IR2 = (value & (0x1f << 5)) << 2;
		IR3 = (value & (0x1f << 10)) >> 3;
		break;
	case 29:
		return;
	case 30:
		LZCR = countLeadingBits(value);
		break;
	case 31:
		return;
	}

	gteRegisters.cp2Data.p[rd].w = value;
}

void cp2_t::CTC2(uint8_t rt, uint8_t rd) {
	uint32_t value = pCpu->get(rt);

	switch (rd) {
	case 4:
	case 12:
	case 20:
	case 26:
	case 27:
	case 29:
	case 30:
		value = (int32_t)(int16_t)value;
		break;
	case 31:
		value = value & 0x7ffff000;
		if (value & 0x7f87e000)
			value |= 0x80000000;
		break;
	}

	gteRegisters.cp2Ctrl.p[rd].w = value;
}

void cp2_t::BC2F(uint16_t imm) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2F 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << '\n';
	g_emulationPaused = true;
}

void cp2_t::BC2T(uint16_t imm) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled CP2 instruction BC2T 0x" << pCpu->opcode
		<< " pc 0x" << pCpu->pc - 4 << '\n';
	g_emulationPaused = true;
}

void cp2_t::COP2(uint32_t imm) {
	FLAG = 0;
	(this->*opcodeLookup[imm & 0x3f].opcode)(imm);

	clocks += opcodeLookup[imm & 0x3f].clocks;

	if (commandBusy)
		pCpu->gteClocks = clocks;

	commandBusy = true;

	if (g_GteInstructionsLogged)
		std::cout << "[CP2] EMULATION PAUSED! unhandled opcode " << opcodeLookup[imm & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << '\n';
}

void cp2_t::LWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	uint32_t value;
	uint8_t cycles;
	pCpu->read32(pCpu->get(rs) + (int32_t)imm, value, cycles);

	switch (rt) {
	case 15:
		SXY0 = SXY1;
		SXY1 = SXY2;
		SXY2 = value;
		break;
	case 28:
		IR1 = (value & 0x1f) << 7;
		IR2 = (value & (0x1f << 5)) << 2;
		IR3 = (value & (0x1f << 10)) >> 3;
		break;
	case 29:
		return;
	case 30:
		LZCR = countLeadingBits(value);
		break;
	case 31:
		return;
	}

	gteRegisters.cp2Data.p[rt].w = value;
}

void cp2_t::SWC2(uint8_t rs, uint8_t rt, int16_t imm) {
	switch (rt) {
	case 1:
	case 3:
	case 5:
	case 8:
	case 9:
	case 10:
	case 11:
		gteRegisters.cp2Data.p[rt].w = (int32_t)gteRegisters.cp2Data.p[rt].shw.l;
		break;
	case 15:
		gteRegisters.cp2Data.p[rt].w = SXY2;
		break;
	case 28:
	case 29:
		if (commandBusy)
			clocks++;

		gteRegisters.cp2Data.p[rt].w = saturate(IR1 >> 7, 0, 0x1f, 0) |
			(saturate(IR2 >> 7, 0, 0x1f, 0) << 5) | (saturate(IR3 >> 7, 0, 0x1f, 0) << 10);
		break;
	}
	uint32_t value = gteRegisters.cp2Data.p[rt].w;
	uint8_t cycles;

	if (commandBusy)
		pCpu->gteClocks = clocks;

	pCpu->write32(pCpu->get(rs) + (int32_t)imm, value, cycles);
}

#define S64(value) value##LL

int32_t cp2_t::bounds(int44 value, uint32_t minFlag, uint32_t maxFlag) {
	if (value.negativeOverflow()) FLAG |= minFlag;
	if (value.positiveOverflow()) FLAG |= maxFlag;

	return gteSR(value.value(), sf);
}

int32_t cp2_t::A1(int44 a) { return bounds(a, (1 << 27) | (1 << 31), (1 << 30) | (1 << 31)); }
int32_t cp2_t::A2(int44 a) { return bounds(a, (1 << 26) | (1 << 31), (1 << 29) | (1 << 31)); }
int32_t cp2_t::A3(int44 a) {
	s_mac3 = a.value();
	return bounds(a, (1 << 25) | (1 << 31), (1 << 28) | (1 << 31));
}

int32_t cp2_t::Lm_B1(int32_t a, int lm) { return saturate(a, -0x8000 * !lm, 0x7fff, (1 << 24) | (1 << 31)); }
int32_t cp2_t::Lm_B2(int32_t a, int lm) { return saturate(a, -0x8000 * !lm, 0x7fff, (1 << 23) | (1 << 31)); }
int32_t cp2_t::Lm_B3(int32_t a, int lm) { return saturate(a, -0x8000 * !lm, 0x7fff, (1 << 22)); }

int32_t cp2_t::Lm_B3_RTP(int64_t a, int sf, int lm) {
	int32_t value_sf = gteSR(a, sf);
	int32_t value_sr = gteSR(a, 1);

	if (value_sr < -0x8000 || value_sr > 0x7fff) FLAG |= 1 << 22;

	return saturate(value_sf, -0x8000 * !lm, 0x7fff, 0);
}

int32_t cp2_t::Lm_C1(int32_t a) { return saturate(a, 0, 0xff, (1 << 21)); }
int32_t cp2_t::Lm_C2(int32_t a) { return saturate(a, 0, 0xff, (1 << 20)); }
int32_t cp2_t::Lm_C3(int32_t a) { return saturate(a, 0, 0xff, (1 << 19)); }

int64_t cp2_t::Lm_D(int64_t a, int sf) { return saturate(gteSR(a, sf), 0, 0xffff, (1 << 18) | (1 << 31)); }

int64_t cp2_t::F(int64_t a) {
	s_mac0 = a;
	if (a > S64(0x7fffffff)) FLAG |= (1 << 16) | (1 << 31);
	if (a < -S64(0x80000000)) FLAG |= (1 << 15) | (1 << 31);
	return a;
}

int32_t cp2_t::Lm_G1(int64_t a) { return saturate(a, -0x400, 0x3ff, (1 << 14) | (1 << 31)); }
int32_t cp2_t::Lm_G2(int64_t a) { return saturate(a, -0x400, 0x3ff, (1 << 13) | (1 << 31)); }

int32_t cp2_t::Lm_H(int64_t value, int sf) {
	int64_t value_sf = gteSR(value, sf);
	int32_t value_12 = gteSR(value, 1);

	constexpr int32_t max = 0x1000;
	constexpr int32_t min = 0x0000;

	if (value_sf < min || value_sf > max) FLAG |= (1 << 12);
	return std::clamp<int32_t>(value_12, min, max);
}

uint32_t cp2_t::gteDivide(uint16_t numerator, uint16_t denominator) {
	//if (numerator >= denominator * 2) { //check if owerflow
	//	FLAG |= (1 << 17) | (1 << 31);
	//	return 0x1ffff;
	//}

	//static uint8_t unrTable[] = {
	//0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,0xee,0xec,0xea,0xe8,0xe6,0xe4,0xe3,
	//0xe1,0xdf,0xdd,0xdc,0xda,0xd8,0xd6,0xd5,0xd3,0xd1,0xd0,0xce,0xcd,0xcb,0xc9,0xc8,
	//0xc6,0xc5,0xc3,0xc1,0xc0,0xbe,0xbd,0xbb,0xba,0xb8,0xb7,0xb5,0xb4,0xb2,0xb1,0xb0,
	//0xae,0xad,0xab,0xaa,0xa9,0xa7,0xa6,0xa4,0xa3,0xa2,0xa0,0x9f,0x9e,0x9c,0x9b,0x9a,
	//0x99,0x97,0x96,0x95,0x94,0x92,0x91,0x90,0x8f,0x8d,0x8c,0x8b,0x8a,0x89,0x87,0x86,
	//0x85,0x84,0x83,0x82,0x81,0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x75,0x74,
	//0x73,0x72,0x71,0x70,0x6f,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,0x68,0x67,0x66,0x65,0x64,
	//0x63,0x62,0x61,0x60,0x5f,0x5e,0x5d,0x5d,0x5c,0x5b,0x5a,0x59,0x58,0x57,0x56,0x55,
	//0x54,0x53,0x53,0x52,0x51,0x50,0x4f,0x4e,0x4d,0x4d,0x4c,0x4b,0x4a,0x49,0x48,0x48,
	//0x47,0x46,0x45,0x44,0x43,0x43,0x42,0x41,0x40,0x3f,0x3f,0x3e,0x3d,0x3c,0x3c,0x3b,
	//0x3a,0x39,0x39,0x38,0x37,0x36,0x36,0x35,0x34,0x33,0x33,0x32,0x31,0x31,0x30,0x2f,
	//0x2e,0x2e,0x2d,0x2c,0x2c,0x2b,0x2a,0x2a,0x29,0x28,0x28,0x27,0x26,0x26,0x25,0x24,
	//0x24,0x23,0x22,0x22,0x21,0x20,0x20,0x1f,0x1e,0x1e,0x1d,0x1d,0x1c,0x1b,0x1b,0x1a,
	//0x19,0x19,0x18,0x18,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x12,0x12,0x11,0x11,
	//0x10,0x0f,0x0f,0x0e,0x0e,0x0d,0x0d,0x0c,0x0c,0x0b,0x0a,0x0a,0x09,0x09,0x08,0x08,
	//0x07,0x07,0x06,0x06,0x05,0x05,0x04,0x04,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,
	//0x00
	//};

	//const uint32_t z = countLeadingZeros16(denominator);
	//const uint32_t n = (uint32_t)numerator << z;
	//uint32_t d = (uint32_t)denominator << z;
	//uint32_t u = unrTable[(d - 0x7fc0) >> 7] + 0x101;
	//d = (0x2000080 - (d * u)) >> 8;
	//d = (0x0000080 + (d * u)) >> 8;
	//return std::min<uint32_t>(0x1ffff, (((uint64_t)d * n) + 0x8000) >> 16);

	if (numerator >= denominator * 2) {  // Division overflow
		FLAG |= (1 << 31) | (1 << 17);
		return 0x1ffff;
	}

	static uint8_t table[] = {
		0xff, 0xfd, 0xfb, 0xf9, 0xf7, 0xf5, 0xf3, 0xf1, 0xef, 0xee, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe3, 0xe1, 0xdf,
		0xdd, 0xdc, 0xda, 0xd8, 0xd6, 0xd5, 0xd3, 0xd1, 0xd0, 0xce, 0xcd, 0xcb, 0xc9, 0xc8, 0xc6, 0xc5, 0xc3, 0xc1,
		0xc0, 0xbe, 0xbd, 0xbb, 0xba, 0xb8, 0xb7, 0xb5, 0xb4, 0xb2, 0xb1, 0xb0, 0xae, 0xad, 0xab, 0xaa, 0xa9, 0xa7,
		0xa6, 0xa4, 0xa3, 0xa2, 0xa0, 0x9f, 0x9e, 0x9c, 0x9b, 0x9a, 0x99, 0x97, 0x96, 0x95, 0x94, 0x92, 0x91, 0x90,
		0x8f, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b,
		0x7a, 0x79, 0x78, 0x77, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70, 0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68,
		0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60, 0x5f, 0x5e, 0x5d, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57,
		0x56, 0x55, 0x54, 0x53, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x48,
		0x47, 0x46, 0x45, 0x44, 0x43, 0x43, 0x42, 0x41, 0x40, 0x3f, 0x3f, 0x3e, 0x3d, 0x3c, 0x3c, 0x3b, 0x3a, 0x39,
		0x39, 0x38, 0x37, 0x36, 0x36, 0x35, 0x34, 0x33, 0x33, 0x32, 0x31, 0x31, 0x30, 0x2f, 0x2e, 0x2e, 0x2d, 0x2c,
		0x2c, 0x2b, 0x2a, 0x2a, 0x29, 0x28, 0x28, 0x27, 0x26, 0x26, 0x25, 0x24, 0x24, 0x23, 0x22, 0x22, 0x21, 0x20,
		0x20, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d, 0x1c, 0x1b, 0x1b, 0x1a, 0x19, 0x19, 0x18, 0x18, 0x17, 0x16, 0x16, 0x15,
		0x15, 0x14, 0x14, 0x13, 0x12, 0x12, 0x11, 0x11, 0x10, 0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0b,
		0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02,
		0x01, 0x01, 0x00, 0x00, 0x00 };

	int shift = countLeadingZeros16(denominator);

	int r1 = (denominator << shift) & 0x7fff;
	int r2 = table[((r1 + 0x40) >> 7)] + 0x101;
	int r3 = ((0x80 - (r2 * (r1 + 0x8000))) >> 8) & 0x1ffff;
	uint32_t reciprocal = ((r2 * r3) + 0x80) >> 8;

	const uint32_t res = ((((uint64_t)reciprocal * (numerator << shift)) + 0x8000) >> 16);

	// Some divisions like 0xF015/0x780B result in 0x20000, but are saturated to 0x1ffff without setting FLAG
	return std::min<uint32_t>(0x1ffff, res);
}

//Instructions

void cp2_t::XXXXX(uint32_t opcode) {
	std::cout << "[CP2] EMULATION PAUSED! unhandled opcode " << opcodeLookup[opcode & 0x3f].name
		<< " 0x" << pCpu->opcode << " pc 0x" << pCpu->pc - 4 << '\n';
	g_emulationPaused = true;
}

void cp2_t::RTPS(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(int44((int64_t)TRX << 12) + (R11 * VX0) + (R12 * VY0) + (R13 * VZ0));
	MAC2 = A2(int44((int64_t)TRY << 12) + (R21 * VX0) + (R22 * VY0) + (R23 * VZ0));
	MAC3 = A3(int44((int64_t)TRZ << 12) + (R31 * VX0) + (R32 * VY0) + (R33 * VZ0));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3_RTP(s_mac3, sf, lm);
	SZ0 = SZ1;
	SZ1 = SZ2;
	SZ2 = SZ3;
	SZ3 = Lm_D(s_mac3, 1);
	const int32_t h_over_sz3 = gteDivide(H, SZ3);
	SXY0 = SXY1;
	SXY1 = SXY2;
	SX2 = Lm_G1(F((int64_t)OFX + ((int64_t)IR1 * h_over_sz3)) >> 16);
	SY2 = Lm_G2(F((int64_t)OFY + ((int64_t)IR2 * h_over_sz3)) >> 16);
	MAC0 = F((int64_t)DQB + ((int64_t)DQA * h_over_sz3));
	IR0 = Lm_H(s_mac0, 1);
}

void cp2_t::RTPT(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	int32_t h_over_sz3 = 0;

	for (int i = 0; i < 3; i++) {
		MAC1 = A1(int44((int64_t)TRX << 12) + (R11 * VX(i)) + (R12 * VY(i)) + (R13 * VZ(i)));
		MAC2 = A2(int44((int64_t)TRY << 12) + (R21 * VX(i)) + (R22 * VY(i)) + (R23 * VZ(i)));
		MAC3 = A3(int44((int64_t)TRZ << 12) + (R31 * VX(i)) + (R32 * VY(i)) + (R33 * VZ(i)));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3_RTP(s_mac3, sf, lm);
		SZ0 = SZ1;
		SZ1 = SZ2;
		SZ2 = SZ3;
		SZ3 = Lm_D(s_mac3, 1);

		h_over_sz3 = gteDivide(H, SZ3);
		SXY0 = SXY1;
		SXY1 = SXY2;
		SX2 = Lm_G1(F((int64_t)OFX + ((int64_t)IR1 * h_over_sz3)) >> 16);
		SY2 = Lm_G2(F((int64_t)OFY + ((int64_t)IR2 * h_over_sz3)) >> 16);
	}

	MAC0 = F((int64_t)DQB + ((int64_t)DQA * h_over_sz3));
	IR0 = Lm_H(s_mac0, 1);
}

void cp2_t::MVMVA(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int mx = GTE_MX(opcode);
	const int v = GTE_V(opcode);
	const int cv = GTE_CV(opcode);
	const int lm = GTE_LM(opcode);

	switch (cv) {
	case 2:
		MAC1 = A1((int64_t)(MX12(mx) * VY(v)) + (MX13(mx) * VZ(v)));
		MAC2 = A2((int64_t)(MX22(mx) * VY(v)) + (MX23(mx) * VZ(v)));
		MAC3 = A3((int64_t)(MX32(mx) * VY(v)) + (MX33(mx) * VZ(v)));
		Lm_B1(A1(((int64_t)CV1(cv) << 12) + (MX11(mx) * VX(v))), 0);
		Lm_B2(A2(((int64_t)CV2(cv) << 12) + (MX21(mx) * VX(v))), 0);
		Lm_B3(A3(((int64_t)CV3(cv) << 12) + (MX31(mx) * VX(v))), 0);
		break;
	default:
		MAC1 = A1(int44((int64_t)CV1(cv) << 12) + (MX11(mx) * VX(v)) + (MX12(mx) * VY(v)) + (MX13(mx) * VZ(v)));
		MAC2 = A2(int44((int64_t)CV2(cv) << 12) + (MX21(mx) * VX(v)) + (MX22(mx) * VY(v)) + (MX23(mx) * VZ(v)));
		MAC3 = A3(int44((int64_t)CV3(cv) << 12) + (MX31(mx) * VX(v)) + (MX32(mx) * VY(v)) + (MX33(mx) * VZ(v)));
		break;
	}

	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
}

void cp2_t::DCPL(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(((R << 4) * IR1) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - ((R << 4) * IR1)), 0))));
	MAC2 = A2(((G << 4) * IR2) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - ((G << 4) * IR2)), 0))));
	MAC3 = A3(((B << 4) * IR3) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - ((B << 4) * IR3)), 0))));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::DPCS(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((R << 16) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - (R << 16)), 0))));
	MAC2 = A2((G << 16) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - (G << 16)), 0))));
	MAC3 = A3((B << 16) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - (B << 16)), 0))));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::DPCT(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	for (int i = 0; i < 3; i++) {
		MAC1 = A1((R0 << 16) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - (R0 << 16)), 0))));
		MAC2 = A2((G0 << 16) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - (G0 << 16)), 0))));
		MAC3 = A3((B0 << 16) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - (B0 << 16)), 0))));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		RGB0 = RGB1;
		RGB1 = RGB2;
		C2 = C;
		R2 = Lm_C1(MAC1 >> 4);
		G2 = Lm_C2(MAC2 >> 4);
		B2 = Lm_C3(MAC3 >> 4);
	}
}

void cp2_t::INTPL(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((IR1 << 12) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - (IR1 << 12)), 0))));
	MAC2 = A2((IR2 << 12) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - (IR2 << 12)), 0))));
	MAC3 = A3((IR3 << 12) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - (IR3 << 12)), 0))));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::SQR(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(IR1 * IR1);
	MAC2 = A2(IR2 * IR2);
	MAC3 = A3(IR3 * IR3);
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
}

void cp2_t::NCS(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((int64_t)(L11 * VX0) + (L12 * VY0) + (L13 * VZ0));
	MAC2 = A2((int64_t)(L21 * VX0) + (L22 * VY0) + (L23 * VZ0));
	MAC3 = A3((int64_t)(L31 * VX0) + (L32 * VY0) + (L33 * VZ0));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
	MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
	MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::NCT(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	for (int i = 0; i < 3; i++) {
		MAC1 = A1((int64_t)(L11 * VX(i)) + (L12 * VY(i)) + (L13 * VZ(i)));
		MAC2 = A2((int64_t)(L21 * VX(i)) + (L22 * VY(i)) + (L23 * VZ(i)));
		MAC3 = A3((int64_t)(L31 * VX(i)) + (L32 * VY(i)) + (L33 * VZ(i)));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
		MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
		MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		RGB0 = RGB1;
		RGB1 = RGB2;
		C2 = C;
		R2 = Lm_C1(MAC1 >> 4);
		G2 = Lm_C2(MAC2 >> 4);
		B2 = Lm_C3(MAC3 >> 4);
	}
}

void cp2_t::NCDS(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((int64_t)(L11 * VX0) + (L12 * VY0) + (L13 * VZ0));
	MAC2 = A2((int64_t)(L21 * VX0) + (L22 * VY0) + (L23 * VZ0));
	MAC3 = A3((int64_t)(L31 * VX0) + (L32 * VY0) + (L33 * VZ0));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
	MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
	MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1(((R << 4) * IR1) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - ((R << 4) * IR1)), 0))));
	MAC2 = A2(((G << 4) * IR2) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - ((G << 4) * IR2)), 0))));
	MAC3 = A3(((B << 4) * IR3) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - ((B << 4) * IR3)), 0))));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::NCDT(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	for (int i = 0; i < 3; i++) {
		MAC1 = A1((int64_t)(L11 * VX(i)) + (L12 * VY(i)) + (L13 * VZ(i)));
		MAC2 = A2((int64_t)(L21 * VX(i)) + (L22 * VY(i)) + (L23 * VZ(i)));
		MAC3 = A3((int64_t)(L31 * VX(i)) + (L32 * VY(i)) + (L33 * VZ(i)));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
		MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
		MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		MAC1 = A1(((R << 4) * IR1) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - ((R << 4) * IR1)), 0))));
		MAC2 = A2(((G << 4) * IR2) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - ((G << 4) * IR2)), 0))));
		MAC3 = A3(((B << 4) * IR3) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - ((B << 4) * IR3)), 0))));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		RGB0 = RGB1;
		RGB1 = RGB2;
		C2 = C;
		R2 = Lm_C1(MAC1 >> 4);
		G2 = Lm_C2(MAC2 >> 4);
		B2 = Lm_C3(MAC3 >> 4);
	}
}

void cp2_t::NCCS(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((int64_t)(L11 * VX0) + (L12 * VY0) + (L13 * VZ0));
	MAC2 = A2((int64_t)(L21 * VX0) + (L22 * VY0) + (L23 * VZ0));
	MAC3 = A3((int64_t)(L31 * VX0) + (L32 * VY0) + (L33 * VZ0));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
	MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
	MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1((R << 4) * IR1);
	MAC2 = A2((G << 4) * IR2);
	MAC3 = A3((B << 4) * IR3);
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::NCCT(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	for (int i = 0; i < 3; i++) {
		MAC1 = A1((int64_t)(L11 * VX(i)) + (L12 * VY(i)) + (L13 * VZ(i)));
		MAC2 = A2((int64_t)(L21 * VX(i)) + (L22 * VY(i)) + (L23 * VZ(i)));
		MAC3 = A3((int64_t)(L31 * VX(i)) + (L32 * VY(i)) + (L33 * VZ(i)));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
		MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
		MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		MAC1 = A1((R << 4) * IR1);
		MAC2 = A2((G << 4) * IR2);
		MAC3 = A3((B << 4) * IR3);
		IR1 = Lm_B1(MAC1, lm);
		IR2 = Lm_B2(MAC2, lm);
		IR3 = Lm_B3(MAC3, lm);
		RGB0 = RGB1;
		RGB1 = RGB2;
		C2 = C;
		R2 = Lm_C1(MAC1 >> 4);
		G2 = Lm_C2(MAC2 >> 4);
		B2 = Lm_C3(MAC3 >> 4);
	}
}

void cp2_t::CDP(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
	MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
	MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1(((R << 4) * IR1) + (IR0 * (Lm_B1(A1(((int64_t)RFC << 12) - ((R << 4) * IR1)), 0))));
	MAC2 = A2(((G << 4) * IR2) + (IR0 * (Lm_B2(A2(((int64_t)GFC << 12) - ((G << 4) * IR2)), 0))));
	MAC3 = A3(((B << 4) * IR3) + (IR0 * (Lm_B3(A3(((int64_t)BFC << 12) - ((B << 4) * IR3)), 0))));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::CC(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(int44((int64_t)RBK << 12) + (LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3));
	MAC2 = A2(int44((int64_t)GBK << 12) + (LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3));
	MAC3 = A3(int44((int64_t)BBK << 12) + (LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	MAC1 = A1((R << 4) * IR1);
	MAC2 = A2((G << 4) * IR2);
	MAC3 = A3((B << 4) * IR3);
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::NCLIP(uint32_t opcode) {
	MAC0 = F((int64_t)(SX0 * SY1) + (SX1 * SY2) + (SX2 * SY0) - (SX0 * SY2) - (SX1 * SY0) - (SX2 * SY1));
}

void cp2_t::AVSZ3(uint32_t opcode) {
	MAC0 = F((int64_t)(ZSF3 * SZ1) + (ZSF3 * SZ2) + (ZSF3 * SZ3));
	OTZ = Lm_D(s_mac0, 1);
}

void cp2_t::AVSZ4(uint32_t opcode) {
	MAC0 = F((int64_t)(ZSF4 * SZ0) + (ZSF4 * SZ1) + (ZSF4 * SZ2) + (ZSF4 * SZ3));
	OTZ = Lm_D(s_mac0, 1);
}

void cp2_t::OP(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1((int64_t)(R22 * IR3) - (R33 * IR2));
	MAC2 = A2((int64_t)(R33 * IR1) - (R11 * IR3));
	MAC3 = A3((int64_t)(R11 * IR2) - (R22 * IR1));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
}

void cp2_t::GPF(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(IR0 * IR1);
	MAC2 = A2(IR0 * IR2);
	MAC3 = A3(IR0 * IR3);
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}

void cp2_t::GPL(uint32_t opcode) {
	sf = GTE_SF(opcode);
	const int lm = GTE_LM(opcode);

	MAC1 = A1(gteSL(MAC1, sf) + (IR0 * IR1));
	MAC2 = A2(gteSL(MAC2, sf) + (IR0 * IR2));
	MAC3 = A3(gteSL(MAC3, sf) + (IR0 * IR3));
	IR1 = Lm_B1(MAC1, lm);
	IR2 = Lm_B2(MAC2, lm);
	IR3 = Lm_B3(MAC3, lm);
	RGB0 = RGB1;
	RGB1 = RGB2;
	C2 = C;
	R2 = Lm_C1(MAC1 >> 4);
	G2 = Lm_C2(MAC2 >> 4);
	B2 = Lm_C3(MAC3 >> 4);
}