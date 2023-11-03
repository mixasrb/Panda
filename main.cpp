#include <iostream>
#include <sstream>
#include <string>

#define OLC_PGE_APPLICATION
#include "dependencies/olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "dependencies/olcPGEX_Sound.h"

#include "console.h"

bool g_emulationPaused = false;
bool g_executedCpuInstructionsLogged = false;
bool g_biosCallbacksLogged = false;
bool g_GteInstructionsLogged = false;
bool g_GpuCommandsLogged = false;
bool g_buttonPressed = false;
int g_mousePossitionX = 0;
int g_mousePossitionY = 0;
bool g_logPixel = false;

uint32_t g_systemClocks = 0;
uint32_t g_systemFrames = 0;
uint32_t g_systemSeconds = 0;
uint32_t g_systemMinutes = 0;

#define CONSOLE_LOGGING 1

const char* g_biosPath = ("D:/projects/emulation_psx/binaries/bios/scph1001.bin");

bool g_sideload = false;
const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/psxtest_cpu.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/psxtest_gte.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/demo/hit-green.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/demo/PRINTGPU.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/demo/PSXNICCC.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/demo/ROOTEXMP.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/demo/VBLANK.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gte/test-all/test-all.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gte-fuzz/gte-fuzz.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/texture-overflow/texture-overflow.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/animated-triangle/animated-triangle.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/lines/lines.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/quad/quad.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/rectangles/rectangles.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/texture-flip/texture-flip.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/transparency/transparency.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/triangle/triangle.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/version-detect/version-detect.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/clipping/clipping.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/gpu/gp0-e1/gp0-e1.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/GTENCLIP.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/GTEAVSZ.exe");

//uint8_t g_driveStatus = _NO_DISC;
uint8_t g_driveStatus = _LICENCED_MODE_2;

const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Yu-Gi-Oh! - Forbidden Memories (Europe).bin");//probably stucked because unhandled spu regs
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Castlevania Chronicles (USA).bin");//SPU:T/O [wait (IRQ/OFF)] //Pads??



//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rayman/Rayman (USA) (Track 01).bin");//VSync: timeout
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Black Dawn/Black Dawn (USA) (Track 01).bin");//SsVabOpenHead : failed! ~needs spu to boot??

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Skullmonkeys.bin");//~[DMA2] EMULATION PAUSED! (GPU) madr: 0 and same for cd drive dma // load delay MAYBE


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vib-Ribbon/Vib-Ribbon (Europe) (En,Fr,De,Es,It) (Track 1).bin");//getlocp loop or gte // problem with tracks

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombat3/Mortal Kombat 3 (Track 01).bin"); //mdec //test event cdrom (waitClocksDataResponse + 0x36cd2*2) // cache  //SeekP //play?? exeption 04//problem with getTn get Td
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Alundra (USA) (v1.1).bin");//mute//unhandled timer1 mode 0x149//going enterier get stuck

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Rebel Assault II - The Hidden Empire (USA) (Disc 1).bin");// DMA3 CDROM 0x11400100h //unhandled timer0 mode 0x14b //mdec //hangs??->mdec
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (v1.1).bin");//GetTN, Mdec // no input fixed//get stuck// DMA 2 linked-list loop ??


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tiny Tank [U] [SCUS-94427].bin");//problem with input

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Need for Speed III - Hot Pursuit (USA).bin"); //invalid setlock amm value 0xe3 getTD problem // reads problem, needs spu regs

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tekken 2 (USA) (v1.1)/Tekken 2 (USA) (v1.1) (Track 1).bin"); // DMA 2 linked-list loop 

//working ones
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Silent Hill (v1.1).bin");//stuck // problem with mem card//hangs maybe spu regs?? needs reverb
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Silent Hill (USA).bin");//stuck // problem with mem card // like no input// maybe spu regs ?? needs reverb

// 
// 
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Castlevania - Symphony of the Night (USA) (Track 1).bin"); //mdec //getstat loop // mem card reading problem?? //setfilter
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Metal Gear Solid (Disc 1) (v1.1).bin"); //gte // np input //changeThreadHandle


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/WipEout 2097 (Europe)/WipEout 2097 (Europe) (Track 01).bin"); //DMA2 LOOP???// something goes wrong with gpu fifo //GetTN,

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Metal Slug X.bin"); // DMA 2 linked-list loop 


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/CT Special Forces - Back to Hell (Europe) (En,Fr,De,Es,It,Nl).bin");// setfilter


// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Bubble Bobble also featuring Rainbow Islands (Track 1).bin"); //bypass mdec amm 1 ass 2 assec 15 => amm 2 ass 44 asect 15 ???
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Puzzle Bobble 2 (Japan) (Track 01).bin");
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Valkyrie Profile (Disc 1).bin");//probably stuct at 1F801DAAh 2  SPU Control Register (SPUCNT)??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Digimon World 3.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Chippoke Ralph no Daibouken - The Adventure of Little Ralph (Japan).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Arc the Lad Collection - Arc the Lad.bin");//[BUS] EMULATION PASUED! unhandled addr read32 0x1f801014 1F801014h 4    SPU_DELAY   Delay/Size (usually 200931E1h) // gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lunar - Silver Star Story Complete (USA) (Disc 1)/Lunar - Silver Star Story Complete (USA) (Disc 1) (Track 1).bin"); //code execution goes wrong // 0x80000080 overwriten 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lunar 2 - Eternal Blue Complete (USA) (Disc 1).bin");// get stuck?? fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Adventures of Lomax, The (USA)/Adventures of Lomax, The (USA) (Track 01).bin"); //GetTN , addr 0x1f8005e0 ?? fixed maybe // gte??
	//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Grandia (Europe) (Disc 1).bin");//Getparam //gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Wild Arms 2 (USA) (Disc 1).bin");//STuCK getTN fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter Alpha 3/Street Fighter Alpha 3 (Track 1).bin"); //GetTN-, invalid setlock asect value 0x8c setmode CDDA bit set?? //but works
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rapid Reload (Europe).bin");// setfilter //GetlocP // gte // but works
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Harmful Park (English v1.1).bin");//Stop //Setfilter // but works???
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gundam Battle Assault 2/Gundam Battle Assault 2 (Track 1).bin");//mdec??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Breath of Fire III/Breath of Fire III (v1.1) (Track 1).bin"); //cd drive out of sync ?? fixed??//~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x4c787878 Undocumented/Nonsense


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Discworld Noir (Europe) (En,Es,It).bin"); //get stuck mdec sometimes?
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Raiden Project, The (USA).bin");//gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo 2 (USA) (Simulation Mode) (Rev 1).bin");//gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/gta/Grand Theft Auto (USA) (Track 01).bin"); // DMA3 CDROM 0x11400100h (chopped) //~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x4c7f7f7f Undocumented/Nonsense
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Grand Theft Auto 2 (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Oddworld - Abe's Oddysee (Europe).bin"); // DMA3 CDROM 0x11400100h (chopped)..//but woks //mdec
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Oddworld - Abe's Exoddus (USA) (Disc 1).bin"); // DMA3 CDROM 0x11400100h (chopped)

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/007 - The World Is Not Enough (USA).bin");// DMA3 CDROM 0x11400100h//low fps
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mega Man X3/Mega Man X3 (Europe) (Track 01).bin"); // DMA3 CDROM 0x11400100h (chopped)
// //const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mega man x4.bin"); // setfilter // getlocp //mdec?  // but works
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Colin McRae Rally 2.0/Colin McRae Rally 2.0 (Europe) (Track 1).bin");//timer mode 0x158



//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spec Ops - Airborne Commando.bin"); //ChangeThread stuck fixed probably
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Doom/Doom (Track 1).bin"); // [BUS] EMULATION PAUSED! unhandled write16 adress 0x1f80105a data 0x50??  1F80105Ah 2  SIO_CTRL Serial Port Control (R/W) //~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x8a// Play // probaly timing issues (waitClocksFirstResponse)//gte


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Monkey Hero (USA).bin");//gte//problem with character sprite

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lucky_Luke_(USA)/Lucky Luke (USA) (Track 01).bin"); // setfilter //mdec // but works //getTN firstResponse = { stat, 0x1, 0x23 }; //getTD firstResponse = { stat, 20, 40 };

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombatII/Mortal Kombat II (Japan) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mortal Kombat 4.bin");//mdec //no input/gte


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot (Europe) (EDC).bin");//gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/crash2.bin")
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot - Warped (USA).bin"); //gte? //1f8008cf unhandled addr read8
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/ctr.bin");//probably stuct at 1F801DAAh 2  SPU Control Register (SPUCNT) SPU:T/O [wait (IRQ/OFF)]
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/crash bash.bin"); //GetTN ?! //cdrom probably 1F801803h.Index1 read stuck fixed

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro the Dragon (Europe) (En,Fr,De,Es,It).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro the Dragon (USA).bin");//no input//trows an exeption
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro 2 - Ripto's Rage! (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro - Year of the Dragon (USA) (v1.1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/lt.bin"); //setfilter// setmode 0xca// getlocL // dont pass nowhere select gte related//stops execution
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/bb.bin"); //setfilter// setmode 0x4a// getlocL//GTE

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Tarzan/Disney's Tarzan (v1.1) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Hercules Action Game (v1.1).bin"); //mdec //gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Aladdin in Nasira's Revenge.bin");//gte // no input
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mickey's Wild Adventure (Europe)/Mickey's Wild Adventure (Europe) (Track 01).bin"); // hangs fixed


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Looney Tunes - Sheep Raider.bin"); //mdec? //GetlocP // 0xffff93f0 unhandled fixed??//gets stuck at level 1


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ridge Racer (USA) (Track 01).bin"); //gte //play

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Resident Evil (Europe).bin");//getlocp and setfilter//CD_init:addr=800ad268 fixed?? //low fps
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/resident evil 2/Resident Evil 2 (Disc 1) (Track 1).bin");//gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Resident Evil 3 - Nemesis (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/ISS Pro Evolution/ISS Pro Evolution (Track 01).bin"); //no input // sendingClocks = 0x1200;//receivingClocks = 0x800; //ackClocks = 0x20;//transfer clocks
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tony Hawk's Pro Skater (USA).bin");//very low fps
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/tekken3/Tekken 3 (Track 1).bin"); //mdec //no input?

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Driver - You Are the Wheelman.bin");//no input


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/World's Scariest Police Chases (USA).bin"); // invalid addr [BUS] EMULATION PAUSED! invalid ram addr write8 0x3000061 data 0x1 fixed maybe // no input


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomb Raider (v1.1)/Tomb Raider (v1.1) (Track 01).bin"); // GetlocP loop??//VSync: timeout
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomb Raider II - Starring Lara Croft (Europe)/Tomb Raider II - Starring Lara Croft (Europe) (Track 01).bin");//hangs/VSync: timeout
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/tomb raider 3/Tomb Raider III - Adventures of Lara Croft (Europe).bin"); //getloocp loop, no input //but works


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spider-Man (USA).bin");//very low fps
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Captain Commando (Japan).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Quake II/Quake II (USA) (Track 1).bin");//gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tom Clancy's Rainbow Six - Lone Wolf (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ace Combat 3 - Electrosphere (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Formula 1 (Europe) (En,Fr,De,Es,It) (Rev 1)/Formula 1 (Europe) (En,Fr,De,Es,It) (Rev 1) (Track 01).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Next Tetris, The (USA)/Next Tetris, The (USA) (Track 01).bin");//hangs
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Medal of Honor (USA).bin");//gte //stuck
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crusader - No Remorse.bin"); //cd drive out of sync // cutscenes need to be skipped 

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy VII (USA) (Disc 1).bin"); //mdec//~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x10000002

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Twisted Metal 2/Twisted Metal 2 (USA) (Track 01).bin"); //[KERNEL] A(3Ah) SystemErrorExit(exitcode) not if intro is skipped


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tank Racer (Europe) (En,Fr,De,Es,It)/Tank Racer (Europe) (En,Fr,De,Es,It) (Track 01).bin");
//demos

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Euro Demo 67 (Europe).bin"); //Ms pacman problem



//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Polaris SnoCross (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (USA) (v1.0).bin");
// 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/SpyroCustomLogo.bin");

const char* g_memCardPaths[2] = { "D:/projects/emulation_psx/binaries/memory_cards/memCard1.bin", "D:/projects/emulation_psx/binaries/memory_cards/memCard2.bin" };


class engine : public olc::PixelGameEngine {
public:

	engine() {
		sAppName = "Panda by Miloseviceva Milicija";
	}

	console_t PSX;

	static engine* pInstance;

	bool isDebuggerViewed = false;

	static float soundOut(int nChanel, float fGlobalTime, float fTimeStep) {
		//if (!g_emulationPaused)
		    return pInstance->PSX.bus.spu.clock();
	}

	bool OnUserCreate() override {

#if !CONSOLE_LOGGING
		std::cout.setstate(std::ios_base::failbit);
#endif

		PSX.reset();

		pInstance = this;


		olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
		olc::SOUND::SetUserSynthFunction(soundOut);

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

				PSX.joyMemCard.swloPad1 = 0xff;
				PSX.joyMemCard.swhiPad1 = 0xff;

				if (GetKey(olc::Key::F).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_SELECT;
				if (GetKey(olc::Key::K3).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_L3;
				if (GetKey(olc::Key::K8).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_R3;
				if (GetKey(olc::Key::H).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_START;

				if (GetKey(olc::Key::W).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_UP;
				if (GetKey(olc::Key::D).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_RIGHT;
				if (GetKey(olc::Key::S).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_DOWN;
				if (GetKey(olc::Key::A).bHeld)
					PSX.joyMemCard.swloPad1 ^= PSX_JOYPAD_LEFT;

				if (GetKey(olc::Key::K2).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_L2;
				if (GetKey(olc::Key::K9).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_R2;
				if (GetKey(olc::Key::K1).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_L1;
				if (GetKey(olc::Key::K0).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_R1;

				if (GetKey(olc::Key::I).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_TRIANGLE;
				if (GetKey(olc::Key::L).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_CIRCLE;
				if (GetKey(olc::Key::K).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_CROSS;
				if (GetKey(olc::Key::J).bHeld)
					PSX.joyMemCard.swhiPad1 ^= PSX_SQUARE;

				PSX.joyMemCard.swloPad2 = 0xff;
				PSX.joyMemCard.swhiPad2 = 0xff;

				if (GetKey(olc::Key::NP1).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_SELECT;
				if (GetKey(olc::Key::NP2).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_START;

				if (GetKey(olc::Key::UP).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_UP;
				if (GetKey(olc::Key::RIGHT).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_RIGHT;
				if (GetKey(olc::Key::DOWN).bHeld)
					PSX.joyMemCard.swloPad2 ^= PSX_JOYPAD_DOWN;
				if (GetKey(olc::Key::LEFT).bHeld)
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


			if (IsFocused() && GetMouse(0).bHeld) {
				g_logPixel = true;
				g_mousePossitionX = GetMouseX();
				g_mousePossitionY = GetMouseY();

				if (g_mousePossitionY > 511)
					g_logPixel = false;
			}

			//clocker

			const uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
			const uint32_t CPU_CLOCKS_PER_FRAME = CPU_CLOCK / 60;

			if (!g_emulationPaused) {
				for (g_systemClocks = 0; g_systemClocks < CPU_CLOCKS_PER_FRAME; g_systemClocks++) {
					PSX.clock();
					if (g_emulationPaused) break;
				}
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


			//debugger
			bool executeOneInstruction = false;

			if (GetKey(olc::Key::CTRL).bHeld) {

				if (GetKey(olc::Key::Q).bHeld) {
					//HAHHAHA BYPASS
					/*if (PSX.bus.dma.interruptRegister.reg.IRQ_enable_dma1 && PSX.bus.dma.interruptRegister.reg.IRQ_master_enable) {
						PSX.bus.dma.interruptRegister.reg.IRQ_master_flag = 1;
						PSX.bus.dma.interruptRegister.reg.IRQ_flag_dma1 = 1;
						PSX.bus.pCpu->cp0.interruptHandler(_IRQ_DMA);
					}*/
					/*if (PSX.bus.dma.interruptRegister.reg.IRQ_enable_dma1 && PSX.bus.dma.interruptRegister.reg.IRQ_master_enable) {
						PSX.bus.dma.interruptRegister.reg.IRQ_master_flag = 1;
						PSX.bus.dma.interruptRegister.reg.IRQ_flag_dma1 = 1;
						PSX.bus.pCpu->cp0.interruptHandler(_IRQ_DMA);
					}*/
					/*PSX.bus.dma.interruptRegister.reg.IRQ_master_flag = 1;
					PSX.bus.dma.interruptRegister.reg.IRQ_flag_dma4 = 1;
					PSX.bus.pCpu->cp0.interruptHandler(_IRQ_DMA);*/

					//Metal Slug DMA 2 linked list loop bypass
					PSX.bus.dma.dma2.channelControl.reg.startBusy = ~PSX.bus.dma.dma2.channelControl.reg.startBusy;
					PSX.bus.dma.isCpuStopped = !PSX.bus.dma.isCpuStopped;
					for (uint32_t i = 0; i < CPU_CLOCKS_PER_FRAME / 10000; i++) {
						PSX.cpuSOC.clock();
					}

					//Crash Bash
					//PSX.cdDrive.irq_flag_read.reg.int_1_7 = 3;
					//PSX.cdDrive.pBus->pCpu->cp0.interruptHandler(_IRQ_CD_ROM);
				}

				if (GetKey(olc::Key::SPACE).bPressed)
					g_emulationPaused = !g_emulationPaused;

				if (GetKey(olc::Key::X).bPressed)
					g_executedCpuInstructionsLogged = !g_executedCpuInstructionsLogged;

				if (GetKey(olc::Key::N).bPressed)
					g_biosCallbacksLogged = !g_biosCallbacksLogged;

				if (GetKey(olc::Key::C).bPressed)
					g_GteInstructionsLogged = !g_GteInstructionsLogged;

				if (GetKey(olc::Key::Z).bPressed)
					g_GpuCommandsLogged = !g_GpuCommandsLogged;

				if (GetKey(olc::Key::B).bPressed) {
					g_buttonPressed = !g_buttonPressed;
					/*for (int i = 0; i < PSX.cpuSOC.loggedOpcodes.size(); i++) {
						std::cout << PSX.cpuSOC.loggedOpcodes[i];
					}*/
				}

				if (GetKey(olc::Key::R).bPressed)
					PSX.debugger.dump("logs/ram.bin", PSX.bus.ram, 2048 * 1024);

				if (GetKey(olc::Key::W).bPressed)
					PSX.debugger.dump("logs/scratchpad.bin", PSX.bus.scratchpad, 1024);

				if (GetKey(olc::Key::V).bPressed)
					PSX.debugger.dump("logs/vram.bin", PSX.gpu.vram, 2048 * 512 * 2);

				if (GetKey(olc::Key::S).bPressed)
					PSX.debugger.dump("logs/dram.bin", PSX.bus.spu.dram, 512 * 1024);

				if (GetKey(olc::Key::O).bPressed)
					executeOneInstruction = true;

				if (GetKey(olc::Key::P).bHeld)
					executeOneInstruction = true;

				if (GetKey(olc::Key::E).bPressed)
					isDebuggerViewed = !isDebuggerViewed;

				if (GetKey(olc::Key::T).bPressed) {
					PSX.gpu.konstant = (PSX.gpu.konstant + 1) & 0x7;
				}
			}

			if (executeOneInstruction) {
				do { PSX.clock(); } while (!PSX.cpuSOC.isInstructionExecuted);
				PSX.cpuSOC.isInstructionExecuted = false;
				executeOneInstruction = !executeOneInstruction;
			}


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
			else {
				DrawSprite(0, 0, PSX.gpu.getVramSprite());

				/*uint16_t x1 = PSX.gpu.display_start.x;
				uint16_t y1 = PSX.gpu.display_start.y;
				uint16_t x2;
				uint16_t y2;
				if (PSX.gpu.gpuStat.elem.horizontalRes2)
					x2 = x1 + 367;
				else switch (PSX.gpu.gpuStat.elem.horizontalRes1) {
				case 0:
					x2 = x1 + 255;
					break;
				case 1:
					x2 = x1 + 319;
					break;
				case 2:
					x2 = x1 + 511;
					break;
				case 3:
					x2 = x1 + 639;
					break;
				}
				if (PSX.gpu.gpuStat.elem.verticalRes)
					y2 = y1 + 479;
				else
					y2 = y1 + 239;

				DrawPartialSprite(0, 0, PSX.gpu.getVramSprite(), x1, y1, x2 - x1, y2 - y1);*/
			}

			if (g_emulationPaused)
				DrawString(800, 512 + 4, "EMULATION PAUSED!", olc::RED);

			if (g_GteInstructionsLogged)
				DrawString(600, 512 + 4, "GTE INSTRUCTIONS LOGGED", olc::YELLOW);

			std::stringstream ss;
			ss << "system clocks: " << PSX.gpu.cpuFrameClocks;
			DrawString(0, 512 + 4, ss.str(), olc::WHITE);
			ss.str("\0");
			ss << "system frames: " << g_systemFrames;
			DrawString(0, 512 + 4 + 9, ss.str(), olc::WHITE);
			ss.str("\0");
			ss << "system seconds: " << g_systemSeconds;
			DrawString(0, 512 + 4 + 2 * 9, ss.str(), olc::WHITE);
			ss.str("\0");
			ss << "system minutes: " << g_systemMinutes;
			DrawString(0, 512 + 4 + 3 * 9, ss.str(), olc::WHITE);

		}
		catch (const std::exception& e) {
			std::cerr << "ERROR: " << e.what() << std::endl;
			system("pause");
			return EXIT_FAILURE;
		}
		return true;
	}

	bool OnUserDestroy()
	{
		olc::SOUND::DestroyAudio();

		return true;
	}
};

engine* engine::pInstance = nullptr;

int main(int argumentCount, char* argumetVector[]) {

	if (argumentCount == 2) {
		g_sideload = true;
		g_sideloadPath = argumetVector[1];
	}

	engine app;
	if (app.Construct(1024, 512 + 160, 1, 1, false, false))
		app.Start();
	return EXIT_SUCCESS;
}