#include <iostream>
#include <sstream>
#include <string>

#define OLC_PGE_APPLICATION
#include "dependencies/olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "dependencies/olcPGEX_Sound.h"

#include "debugger/debugger.h"

#include "console.h"
#include "settings.h"
#include "gpu/cxdxxxxx.h"

uint32_t g_systemClocks = 0;
uint32_t g_systemFrames = 0;
uint32_t g_systemSeconds = 0;
uint32_t g_systemMinutes = 0;

int g_mousePossitionX = 0;
int g_mousePossitionY = 0;

extern bool g_isEmulationPaused;

class ApplicationEmulator : public olc::PixelGameEngine {
private:
	static ApplicationEmulator* instancePtr;


public:

	ApplicationEmulator() {
		sAppName = "Panda by MM";
	}

	PsxConsole psx;

	Output sampleLR = { 0.0f, 0.0f };

	bool isDebuggerViewed = false;
	bool bDrawScreenWindow = false;

	static float soundOut(int nChanel, float fGlobalTime, float fTimeStep) {
		if (nChanel == 0) {
#if ACCURATE_SPU_CLOCKING
			if (!g_emulationPaused) {

				for (int i = 0; i < 0x300; i++) {
					if (g_systemClocks == CPU_CLOCKS_PER_FRAME) {
						g_systemClocks = 0;
						pInstance->psx.gpu.cpuScanlineClocks = 0;
					}

					pInstance->psx.clock();
					g_systemClocks++;
				}
			}
#endif

			instancePtr->sampleLR = instancePtr->psx.bus.spu.Clock();
		}

		return (nChanel == 0) ? instancePtr->sampleLR.channel0 : instancePtr->sampleLR.channel1;
	}

	std::unique_ptr<olc::Decal> decSprite;

	bool OnUserCreate() override {

#if !CONSOLE_LOGGING
		std::cout.setstate(std::ios_base::failbit);
#endif

		instancePtr = this;

		olc::SOUND::InitialiseAudio(44100, 2, 8, 512);
		olc::SOUND::SetUserSynthFunction(soundOut);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {

		//Clear(olc::VERY_DARK_GREY);
		Clear(olc::BLACK);

		//gamepad input 
		if (!g_isEmulationPaused) {

			enum keyMap_swlo {
				PSX_SELECT = 0x01,
				PSX_L3 = 0x02,
				PSX_R3 = 0x04,
				PSX_START = 0x08,
				PSX_JOYPAD_UP = 0x10,
				PSX_JOYPAD_RIGHT = 0x20,
				PSX_JOYPAD_DOWN = 0x40,
				PSX_JOYPAD_LEFT = 0x80
			};
			enum keyMap_swhi {
				PSX_L2 = 0x01,
				PSX_R2 = 0x02,
				PSX_L1 = 0x04,
				PSX_R1 = 0x08,
				PSX_TRIANGLE = 0x10,
				PSX_CIRCLE = 0x20,
				PSX_CROSS = 0x40,
				PSX_SQUARE = 0x80
			};

			psx.joyMemCard.swloPad1 = 0xff;
			psx.joyMemCard.swhiPad1 = 0xff;

			if (GetKey(olc::Key::F).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_SELECT;
			if (GetKey(olc::Key::K3).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_L3;
			if (GetKey(olc::Key::K8).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_R3;
			if (GetKey(olc::Key::H).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_START;

			if (GetKey(olc::Key::W).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_JOYPAD_UP;
			if (GetKey(olc::Key::D).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_JOYPAD_RIGHT;
			if (GetKey(olc::Key::S).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_JOYPAD_DOWN;
			if (GetKey(olc::Key::A).bHeld)
				psx.joyMemCard.swloPad1 ^= PSX_JOYPAD_LEFT;

			if (GetKey(olc::Key::K2).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_L2;
			if (GetKey(olc::Key::K9).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_R2;
			if (GetKey(olc::Key::K1).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_L1;
			if (GetKey(olc::Key::K0).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_R1;

			if (GetKey(olc::Key::I).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_TRIANGLE;
			if (GetKey(olc::Key::L).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_CIRCLE;
			if (GetKey(olc::Key::K).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_CROSS;
			if (GetKey(olc::Key::J).bHeld)
				psx.joyMemCard.swhiPad1 ^= PSX_SQUARE;

			psx.joyMemCard.swloPad2 = 0xff;
			psx.joyMemCard.swhiPad2 = 0xff;

			if (GetKey(olc::Key::NP1).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_SELECT;
			if (GetKey(olc::Key::NP2).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_START;

			if (GetKey(olc::Key::UP).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_JOYPAD_UP;
			if (GetKey(olc::Key::RIGHT).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_JOYPAD_RIGHT;
			if (GetKey(olc::Key::DOWN).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_JOYPAD_DOWN;
			if (GetKey(olc::Key::LEFT).bHeld)
				psx.joyMemCard.swloPad2 ^= PSX_JOYPAD_LEFT;

			if (GetKey(olc::Key::NP8).bHeld)
				psx.joyMemCard.swhiPad2 ^= PSX_TRIANGLE;
			if (GetKey(olc::Key::NP6).bHeld)
				psx.joyMemCard.swhiPad2 ^= PSX_CIRCLE;
			if (GetKey(olc::Key::NP5).bHeld)
				psx.joyMemCard.swhiPad2 ^= PSX_CROSS;
			if (GetKey(olc::Key::NP4).bHeld)
				psx.joyMemCard.swhiPad2 ^= PSX_SQUARE;
		}

		//clocker
#if !ACCURATE_SPU_CLOCKING
		if (!g_isEmulationPaused) {
			for (g_systemClocks = 0; g_systemClocks < CPU_CLOCKS_PER_FRAME; g_systemClocks++) {

				psx.clock();

				if (g_isEmulationPaused) break;
			}
			psx.gpu.cpuScanlineClocks = 0;

			g_systemFrames++;
			if (g_systemFrames == 60) {
				g_systemFrames = 0;
				g_systemSeconds++;
				if (g_systemSeconds == 60) {
					g_systemSeconds = 0;
					g_systemMinutes++;
				}
			}
		}
#endif

		//debugger
		bool executeOneInstruction = false;

		if (GetKey(olc::Key::CTRL).bHeld) {

			if (IsFocused() && GetMouse(0).bHeld) {
				g_logPixel = true;
				g_mousePossitionX = GetMouseX();
				g_mousePossitionY = GetMouseY();

				if (g_mousePossitionY > 511)
					g_logPixel = false;
			}

			if (GetKey(olc::Key::Q).bHeld) {

				//Metal Slug DMA 2 linked list loop bypass
				//psx.bus.dma.dma2.channelControl.reg.startBusy = ~psx.bus.dma.dma2.channelControl.reg.startBusy;

				psx.bus.dma.isCpuStopped = !psx.bus.dma.isCpuStopped;
				for (uint32_t i = 0; i < CPU_CLOCKS_PER_FRAME; i++) {
					psx.cpuSOC.Clock();
					//psx.clock();
				}
			}

			if (GetKey(olc::Key::SPACE).bPressed)
				g_isEmulationPaused = !g_isEmulationPaused;

			if (GetKey(olc::Key::X).bPressed)
				g_executedCpuInstructionsLogged = !g_executedCpuInstructionsLogged;

			if (GetKey(olc::Key::N).bPressed)
				g_biosCallbacksLogged = !g_biosCallbacksLogged;

			if (GetKey(olc::Key::C).bPressed)
				g_GteInstructionsLogged = !g_GteInstructionsLogged;

			if (GetKey(olc::Key::Z).bPressed)
				g_GpuCommandsLogged = !g_GpuCommandsLogged;

			if (GetKey(olc::Key::B).bPressed)
				g_buttonPressed = !g_buttonPressed;

			/*
			if (GetKey(olc::Key::R).bPressed)
				psx.debugger.dump("logs/ram.bin", psx.bus.ram.get(), 2048 * 1024);

			if (GetKey(olc::Key::Y).bPressed)
				psx.debugger.dump("logs/scratchpad.bin", psx.bus.scratchpad.get(), 1024);

			if (GetKey(olc::Key::V).bPressed)
				psx.debugger.dump("logs/vram.bin", psx.gpu.vram.get(), 2048 * 512);

			if (GetKey(olc::Key::S).bPressed)
				psx.debugger.dump("logs/spu_dram.bin", psx.bus.spu.ls, 512 * 1024);

			if (GetKey(olc::Key::M).bPressed)
				psx.debugger.dump("logs/memcard_dumped.bin", psx.joyMemCard.memCards[0], 128 * 1024);
				*/

			if (GetKey(olc::Key::O).bPressed)
				executeOneInstruction = true;

			if (GetKey(olc::Key::E).bPressed)
				isDebuggerViewed = !isDebuggerViewed;

			if (GetKey(olc::Key::T).bPressed)
				g_manualInterruptDMA = !g_manualInterruptDMA;

			if (GetKey(olc::Key::W).bPressed)
				bDrawScreenWindow = !bDrawScreenWindow;
		}

		if (executeOneInstruction) {
			do {
				psx.clock();
			} while (!psx.cpuSOC.isInstructionExecuted);
			psx.cpuSOC.isInstructionExecuted = false;
			executeOneInstruction = !executeOneInstruction;
		}

		if (!isDebuggerViewed) {
			// Main Drawing	
#if DEBUG_VIEW
			DrawSprite(0, 0, psx.gpu.GetVramSprite());
#endif
			uint16_t x1 = psx.gpu.displayStart.x;
			uint16_t y1 = psx.gpu.displayStart.y;
			uint16_t x2;
			uint16_t y2;

			uint16_t horizontalResolution;
			uint16_t verticalResolution;
			if (psx.gpu.gpuStat.elem.horizontalRes2) {
				//x1 = x1 + psx.gpu.screenX1 / 7;
				horizontalResolution = (psx.gpu.screenX2 - psx.gpu.screenX1) / 7;
				//horizontalResolution = 368;
				x2 = x1 + horizontalResolution;
			}
			else switch (psx.gpu.gpuStat.elem.horizontalRes1) {
			case 0:
				//x1 = x1 + psx.gpu.screenX1 / 10;
				horizontalResolution = (psx.gpu.screenX2 - psx.gpu.screenX1) / 10;
				//horizontalResolution = 256;
				x2 = x1 + horizontalResolution;
				break;
			case 1:
				//x1 = x1 + psx.gpu.screenX1 / 8;
				horizontalResolution = (psx.gpu.screenX2 - psx.gpu.screenX1) / 8;
				//horizontalResolution = 320;
				x2 = x1 + horizontalResolution;
				break;
			case 2:
				//x1 = x1 + psx.gpu.screenX1 / 5;
				horizontalResolution = (psx.gpu.screenX2 - psx.gpu.screenX1) / 5;
				//horizontalResolution = 512;
				x2 = x1 + horizontalResolution;
				break;
			case 3:
				//x1 = x1 + psx.gpu.screenX1 / 4;
				horizontalResolution = (psx.gpu.screenX2 - psx.gpu.screenX1) / 4;
				//horizontalResolution = 640;
				x2 = x1 + horizontalResolution;
				break;
			}
			if (psx.gpu.gpuStat.elem.verticalRes) {
				verticalResolution = (psx.gpu.screenY2 - psx.gpu.screenY1) * 2;
				//verticalResolution = 480;
				y2 = y1 + verticalResolution;
			}
			else {
				verticalResolution = psx.gpu.screenY2 - psx.gpu.screenY1;
				//verticalResolution = 240;
				y2 = y1 + verticalResolution;
			}
#if DEBUG_VIEW
			if (bDrawScreenWindow)
				DrawRect({ x1, y1 }, { x2 - x1, y2 - y1 });
#else
			if (!psx.gpu.gpuStat.elem.displayEnable) {
				if (psx.bus.pGpu->gpuStat.elem.displayAreaColorDepth) {
					olc::Sprite vramView = olc::Sprite(horizontalResolution, verticalResolution);
					for (int x = 0; x < horizontalResolution * 3 / 2; x++) {
						int i = 0; //??
						for (int y = 0; y < verticalResolution; y++) {
							olc::Pixel pixel;
							switch ((x + i) % 3) {
							case 0: {
								uint16_t data;
								Vector2D p;
								p.x = x1 + x + i;
								p.y = y1 + y;
								psx.bus.pGpu->ReadVram(p, data, true);
								pixel.r = data & 0xff;
								pixel.g = (data >> 8) & 0xff;
								p.x = x1 + x + 1 + i;
								psx.bus.pGpu->ReadVram(p, data, true);
								pixel.b = data & 0xff;
								break;
							}
							case 1: {
								uint16_t data;
								Vector2D p;
								p.x = x1 + x + i;
								p.y = y1 + y;
								psx.bus.pGpu->ReadVram(p, data, true);
								pixel.r = (data >> 8) & 0xff;
								p.x = x1 + x + 1 + i;
								psx.bus.pGpu->ReadVram(p, data, true);
								pixel.g = data & 0xff;
								pixel.b = (data >> 8) & 0xff;
								break;
							}
							case 2:
								i++;
								break;
							}
							vramView.SetPixel((x + 1) * 2 / 3, y, pixel);
						}
					}
					decSprite.reset(new olc::Decal(&vramView));
					DrawDecal({ 0, 0 }, decSprite.get(), { 640.0f / ((float)x2 - (float)x1), 480.0f / ((float)y2 - (float)y1) });
				}
				else {
					decSprite.reset(new olc::Decal(psx.gpu.GetVramSprite()));
					DrawPartialDecal({ 0, 0 }, decSprite.get(), { (float)x1, (float)y1 },
						{ (float)x2 - (float)x1, (float)y2 - (float)y1 },
						{ 640.0f / ((float)x2 - (float)x1), 480.0f / ((float)y2 - (float)y1) });
				}
			}
			else
				Clear(olc::BLACK);
#endif
		}
		else {
			//Debugger

			for (int i = 0; i < 32; i++) {
				uint32_t data;
				uint8_t cycels;

				psx.bus.Read32(psx.cpuSOC.pc_old - 16 * 4 + i * 4, data, cycels, true);
				uint32_t pc = psx.cpuSOC.pc_old - 16 * 4 + i * 4;
				DrawString(489, 0 + 9 * i, psx.cpuSOC.getInstructionStr(data, pc));
				if (i == 16) {
					DrawString(489, 0 + 9 * i, psx.cpuSOC.getInstructionStr(data, psx.cpuSOC.pc_old - 16 * 4 + i * 4), olc::RED);
					DrawString(464, 108 + 36, "pc:", olc::WHITE);
					DrawLine(480, 88 + 72, 800, 88 + 72, olc::RED);
				}
			}
			for (int i = 0; i < 32; i++) {
				std::string s = psx.cpuSOC.executedOpcodes[i];
				DrawString(489, 0 + 9 * (33 + i), s);
			}
			for (int i = 0; i < 32; i++) {
				std::stringstream ss;
				if (i < 10)
					ss << "r[0" << i << "] 0x" << std::hex << psx.cpuSOC.GetRegister(i);
				else
					ss << "r[" << i << "] 0x" << std::hex << psx.cpuSOC.GetRegister(i);
				DrawString(0, i * 9, ss.str());
			}
			std::stringstream ss;
			ss << "hi 0x" << std::hex << psx.cpuSOC.hi;
			DrawString(0, 32 * 9, ss.str());
			ss.str("\0");
			ss << "lo 0x" << std::hex << psx.cpuSOC.lo;
			DrawString(0, 33 * 9, ss.str());
			ss.str("\0");
			ss << "Cop0 SR 0x" << std::hex << psx.cpuSOC.cp0.cop0r[12];
			DrawString(0, 34 * 9, ss.str());
			ss.str("\0");
			ss << "I_MASK 0x" << std::hex << psx.bus.interruptMask;
			DrawString(0, 35 * 9, ss.str());
			ss.str("\0");
			ss << "I_STAT 0x" << std::hex << psx.bus.interruptStat;
			DrawString(0, 36 * 9, ss.str());
		}

		if (g_isEmulationPaused)
			DrawString(800, 512 + 4, "EMULATION PAUSED!", olc::RED);

		if (g_GteInstructionsLogged)
			DrawString(600, 512 + 4, "GTE INSTRUCTIONS LOGGED", olc::YELLOW);

		std::stringstream ss;

		ss << "System\n";
		ss << "Minutes: " << g_systemMinutes << '\n';
		ss << "Seconds: " << g_systemSeconds << '\n';
		ss << "Frames: " << g_systemFrames << '\n';
		ss << "Clocks: " << psx.gpu.cpuClocks;
		DrawString(9, 512 + 4 - 1 * 9, ss.str(), olc::WHITE);

		return true;
	}

	bool OnUserDestroy() override {
		olc::SOUND::DestroyAudio();
		return true;
	}
};

ApplicationEmulator* ApplicationEmulator::instancePtr = nullptr;

int main(int argumentCount, char* argumetVector[]) {

	if (argumentCount == 2) {
		g_sideload = true;
		g_sideloadPath = argumetVector[1];
	}

	ApplicationEmulator emulator;

#if DEBUG_VIEW
	if (emulator.Construct(1024, 512 + 45, 1, 1, false, false))
#else
	if (emulator.Construct(640, 480, 2, 2, false, false))
#endif
		emulator.Start();

	return EXIT_SUCCESS;
}