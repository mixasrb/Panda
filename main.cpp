#include <iostream>
#include <sstream>

#define OLC_PGE_APPLICATION
#include "dependencies/olcPixelGameEngine.h"

#include "console.h"

bool g_emulationPaused = false;
bool g_executedCpuInstructionsLogged = false;
bool g_GteInstructionsLogged = false;
bool g_buttonPressed = false;

const char* g_biosPath = ("D:/projects/emulation_psx/binaries/bios/scph101.bin");

bool g_sideload = false;
const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/psxtest_cpx.exe");

uint8_t g_driveStatus = _LICENCED_MODE_2;

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy VII (USA) (Disc 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/tekken3/Tekken 3 (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spec Ops - Airborne Commando.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/gta/Grand Theft Auto (USA) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mortal Kombat 4.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombatII/Mortal Kombat II (Japan) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombat3/Mortal Kombat 3 (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/crash.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Castlevania - Symphony of the Night (USA) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (v1.1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot - Warped (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ridge Racer (USA) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Puzzle Bobble 2 (Japan) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Raiden Project, The (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Metal Gear Solid (Disc 1) (v1.1).bin");
//bypass mdec amm 1 ass 2 assec 15 => amm 2 ass 44 asect 15 ???
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Bubble Bobble also featuring Rainbow Islands (Track 1).bin");

class Demo : public olc::PixelGameEngine {
public:
	Demo() {
		sAppName = "Panda by Miloseviceva Milicija";
	}

	Console PSX;

	bool isDebuggerViewed = false;

public:
	bool OnUserCreate() override {
		PSX.reset();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		try {
			Clear(olc::VERY_DARK_BLUE);

			//gamepad input 

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

			if (!g_emulationPaused) {
				if (GetKey(olc::Key::F).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_SELECT;
				if (GetKey(olc::Key::H).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_START;

				else if (GetKey(olc::Key::W).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_UP;
				else if (GetKey(olc::Key::D).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_RIGHT;
				else if (GetKey(olc::Key::S).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_DOWN;
				else if (GetKey(olc::Key::A).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_LEFT;

				if (GetKey(olc::Key::I).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_TRIANGLE;
				if (GetKey(olc::Key::L).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_CIRCLE;
				if (GetKey(olc::Key::K).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_CROSS;
				if (GetKey(olc::Key::J).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_SQUARE;

				if (GetKey(olc::Key::NP1).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_SELECT;
				if (GetKey(olc::Key::NP2).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_START;

				else if (GetKey(olc::Key::UP).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_UP;
				else if (GetKey(olc::Key::RIGHT).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_RIGHT;
				else if (GetKey(olc::Key::DOWN).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_DOWN;
				else if (GetKey(olc::Key::LEFT).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_LEFT;

				if (GetKey(olc::Key::NP8).bHeld)
					PSX.joyMemCard.swhiPad2 ^= PSX_TRIANGLE;
				if (GetKey(olc::Key::NP6).bHeld)
					PSX.joyMemCard.swhiPad2 ^= PSX_CIRCLE;
				if (GetKey(olc::Key::NP5).bHeld)
					PSX.joyMemCard.swhiPad2 ^= PSX_CROSS;
				if (GetKey(olc::Key::NP4).bHeld)
					PSX.joyMemCard.swhiPad2 ^= PSX_SQUARE;
			}


			//clocker

			const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
			const uint32_t CPU_CLOCKS_PER_FRAME = CPU_CLOCK / 60;

			if (!g_emulationPaused)
				for (uint32_t i = 0; i < CPU_CLOCKS_PER_FRAME; i++) {
					PSX.clock();

					if (g_emulationPaused) break;
				}


			//debugger

			if (GetKey(olc::Key::Q).bPressed) {
				//HAHHAHA BYPASS
				if (PSX.bus.dma.interruptRegister.reg.IRQ_enable_dma1 && PSX.bus.dma.interruptRegister.reg.IRQ_master_enable) {
					PSX.bus.dma.interruptRegister.reg.IRQ_master_flag = 1;
					PSX.bus.dma.interruptRegister.reg.IRQ_flag_dma1 = 1;
					PSX.bus.pCpu->cp0.interruptHandler(_IRQ_DMA);
				}
				/*PSX.bus.dma.interruptRegister.reg.IRQ_master_flag = 1;
				PSX.bus.dma.interruptRegister.reg.IRQ_flag_dma4 = 1;
				PSX.bus.pCpu->cp0.interruptHandler(_IRQ_DMA);*/
			}

			if (GetKey(olc::Key::SPACE).bPressed)
				g_emulationPaused = !g_emulationPaused;

			if (GetKey(olc::Key::X).bPressed)
				g_executedCpuInstructionsLogged = !g_executedCpuInstructionsLogged;

			if (GetKey(olc::Key::C).bPressed)
				g_GteInstructionsLogged = !g_GteInstructionsLogged;

			if (GetKey(olc::Key::B).bPressed)
				g_buttonPressed = !g_buttonPressed;

			if (GetKey(olc::Key::R).bPressed)
				PSX.debugger.dump("logs/ram.bin", PSX.bus.ram, 2048 * 1024);

			if (GetKey(olc::Key::V).bPressed)
				PSX.debugger.dump("logs/vram.bin", PSX.gpu.vram, 2048 * 512 * 2);

			bool executeOneInstruction = false;
			if (GetKey(olc::Key::O).bPressed)
				executeOneInstruction = true;

			if (GetKey(olc::Key::P).bHeld)
				executeOneInstruction = true;

			if (executeOneInstruction) {
				do { PSX.clock(); } while (!PSX.cpuSOC.isInstructionExecuted);
				PSX.cpuSOC.isInstructionExecuted = false;
				executeOneInstruction = !executeOneInstruction;
			}

			bool isScanlineDrawn = false;
			if (GetKey(olc::Key::S).bPressed)
				isScanlineDrawn = true;

			if (isScanlineDrawn) {
				for (int i = 0; i < 1000; i++) {
					PSX.clock();
				}
				isScanlineDrawn = false;
			}

			if (GetKey(olc::Key::E).bPressed)
				isDebuggerViewed = !isDebuggerViewed;

			if (isDebuggerViewed) {
				for (int i = 0; i < 32; i++) {
					uint32_t data;
					uint8_t cycels;

					PSX.bus.cpuRead32(PSX.cpuSOC.pc_old - 16 * 4 + i * 4, data, cycels, true);
					uint32_t pc = PSX.cpuSOC.pc_old - 16 * 4 + i * 4;
					DrawString(489, 0 + 9 * i, PSX.cpuSOC.getInstructionStr(data, pc));
					if (i == 16) {
						DrawString(489, 0 + 9 * i, PSX.cpuSOC.getInstructionStr(data, PSX.cpuSOC.pc_old - 16 * 4 + i * 4), olc::RED);
						DrawString(464, 108 + 36, "pc:", olc::WHITE);
						DrawLine(480, 88 + 72, 800, 88 + 72, olc::RED);
					}
				}
				for (int i = 0; i < 32; i++) {
					std::string s = PSX.cpuSOC.executedOpcodes[i];
					DrawString(489, 0 + 9 * (33 + i), s);
				}
				for (int i = 0; i < 32; i++) {
					std::stringstream ss;
					if (i < 10)
						ss << "r[0" << i << "] 0x" << std::hex << PSX.cpuSOC.get(i);
					else
						ss << "r[" << i << "] 0x" << std::hex << PSX.cpuSOC.get(i);
					DrawString(0, i * 9, ss.str());
				}
				std::stringstream ss;
				ss << "hi 0x" << std::hex << PSX.cpuSOC.hi;
				DrawString(0, 32 * 9, ss.str());
				ss.str("\0");
				ss << "lo 0x" << std::hex << PSX.cpuSOC.lo;
				DrawString(0, 33 * 9, ss.str());
				ss.str("\0");
				ss << "Cop0 SR 0x" << std::hex << PSX.cpuSOC.cp0.cop0r[12];
				DrawString(0, 34 * 9, ss.str());
				ss.str("\0");
				ss << "I_MASK 0x" << std::hex << PSX.bus.interruptMask;
				DrawString(0, 35 * 9, ss.str());
				ss.str("\0");
				ss << "I_STAT 0x" << std::hex << PSX.bus.interruptStat;
				DrawString(0, 36 * 9, ss.str());
			}
			else
				DrawSprite(0, 0, PSX.gpu.getVramSprite());

			if (g_emulationPaused)
				DrawString(800, 500 + 16, "EMULATION PAUSED!", olc::RED);

			if (g_GteInstructionsLogged)
				DrawString(600, 500 + 16, "GTE INSTRUCTIONS LOGGED", olc::YELLOW);

		}
		catch (const std::exception& e) {
			std::cerr << "ERROR: " << e.what() << std::endl;
			system("pause");
			return EXIT_FAILURE;
		}
		return true;
	}
};

int main() {
	Demo app;
	if (app.Construct(1024, 512 + 160, 1, 1, false, true))
		app.Start();
	return EXIT_SUCCESS;
}