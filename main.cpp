#include <iostream>
#include <sstream>

#define OLC_PGE_APPLICATION
#include "dependencies/olcPixelGameEngine.h"

#include "console.h"

bool isEmulationPaused = false;
bool areExecutedCpuInstructionsLogged = false;
bool areGTEInstructionsLogged = false;
bool isButtonPressed = false;

const char* biosPath = ("D:/projects/emulation_psx/binaries/bios/scph1001.bin");

bool bSideload = false;
const char* sideloadPath = ("D:/projects/emulation_psx/binaries/tests/psxtest_cpx.exe");

const char* cdPath = ("D:/projects/emulation_psx/binaries/roms/Ridge Racer (USA) (Track 01).bin");
//const char* cdPath = ("D:/projects/emulation_psx/binaries/roms/Puzzle Bobble 2 (Japan) (Track 01).bin");
//const char* cdPath = ("D:/projects/emulation_psx/binaries/roms/crash.bin");
//const char* cdPath = ("D:/projects/emulation_psx/binaries/roms/helloworld.ps-exe");
//const char* cdPath = ("D:/projects/emulation_psx/binaries/roms/Bubble Bobble also featuring Rainbow Islands (Track 1).bin");

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

			if (!isEmulationPaused) {
				if (GetKey(olc::Key::SHIFT).bHeld)
					PSX.joyMemCard.swlo ^= PSX_SELECT;
				if (GetKey(olc::Key::CTRL).bHeld)
					PSX.joyMemCard.swlo ^= PSX_START;

				else if (GetKey(olc::Key::UP).bHeld)
					PSX.joyMemCard.swlo ^= PSX_JOYPAD_UP;
				else if (GetKey(olc::Key::RIGHT).bHeld)
					PSX.joyMemCard.swlo ^= PSX_JOYPAD_RIGHT;
				else if (GetKey(olc::Key::DOWN).bHeld)
					PSX.joyMemCard.swlo ^= PSX_JOYPAD_DOWN;
				else if (GetKey(olc::Key::LEFT).bHeld)
					PSX.joyMemCard.swlo ^= PSX_JOYPAD_LEFT;

				if (GetKey(olc::Key::NP8).bHeld)
					PSX.joyMemCard.swhi ^= PSX_TRIANGLE;
				if (GetKey(olc::Key::NP6).bHeld)
					PSX.joyMemCard.swhi ^= PSX_CIRCLE;
				if (GetKey(olc::Key::NP5).bHeld)
					PSX.joyMemCard.swhi ^= PSX_CROSS;
				if (GetKey(olc::Key::NP4).bHeld)
					PSX.joyMemCard.swhi ^= PSX_SQUARE;
			}


			//clocker

			const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
			const uint32_t CPU_CLOCKS_PER_FRAME = CPU_CLOCK / 60;

			if (!isEmulationPaused)
				for (uint32_t i = 0; i < CPU_CLOCKS_PER_FRAME; i++) {
					PSX.clock();

					if (isEmulationPaused) break;
				}


			//debugger

			if (GetKey(olc::Key::Q).bPressed) {
				//PSX.bus.interruptMask |= 0x200;
				//PSX.bus.pCdDrive->irq_flag_read.reg.int_1_7 = 0x3;
				PSX.cpuSOC.cp0.interruptHandler(0x8);
			}

			if (GetKey(olc::Key::SPACE).bPressed)
				isEmulationPaused = !isEmulationPaused;

			if (GetKey(olc::Key::L).bPressed)
				areExecutedCpuInstructionsLogged = !areExecutedCpuInstructionsLogged;

			if (GetKey(olc::Key::G).bPressed)
				areGTEInstructionsLogged = !areGTEInstructionsLogged;

			if (GetKey(olc::Key::H).bPressed)
				isButtonPressed = !isButtonPressed;

			if (GetKey(olc::Key::D).bPressed)
				PSX.debugger.dump("logs/ram.bin", PSX.bus.ram, 2048 * 1024);

			if (GetKey(olc::Key::V).bPressed)
				PSX.debugger.dump("logs/vram.bin", PSX.gpu.vram, 2048 * 512 * 2);

			bool executeOneInstruction = false;
			if (GetKey(olc::Key::I).bPressed)
				executeOneInstruction = true;

			if (GetKey(olc::Key::O).bHeld)
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
				DrawSprite(0, 0, PSX.gpu.getVram());

			if (isEmulationPaused)
				DrawString(800, 500 + 16, "EMULATION PAUSED!", olc::RED);

			if (areGTEInstructionsLogged)
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
	if (app.Construct(1024, 512 + 16, 1, 1, false, true))
		app.Start();
	return EXIT_SUCCESS;
}