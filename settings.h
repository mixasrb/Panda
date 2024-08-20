#pragma once

#define CONSOLE_LOGGING 1
#define SCREEN 0

bool g_emulationPaused = false;
bool g_executedCpuInstructionsLogged = false;
bool g_biosCallbacksLogged = false;
bool g_GteInstructionsLogged = false;
bool g_GpuCommandsLogged = false;
bool g_buttonPressed = false;
bool g_logPixel = true;
bool g_manualInterruptDMA = false;

//clocker settings
constexpr uint32_t CPU_CLOCK = 44100 * 0x300; //33 868 800
constexpr uint32_t CPU_CLOCKS_PER_FRAME_NTSC = ((float)CPU_CLOCK / 59.29f);
constexpr uint32_t CPU_CLOCKS_PER_FRAME_PAL = ((float)CPU_CLOCK / 49.76f);
constexpr uint32_t CPU_CLOCKS_PER_FRAME = CPU_CLOCKS_PER_FRAME_NTSC;

const char* g_biosPath = ("D:/projects/emulation_psx/binaries/bios/scph101.bin");
//const char* g_biosPath = ("D:/projects/emulation_psx/binaries/bios/SCPH-102B.BIN");//PAL

const char* g_memCardPaths[2] = { "D:/projects/emulation_psx/binaries/memory_cards/memCard1.bin", "D:/projects/emulation_psx/binaries/memory_cards/memCard2.bin" };

bool g_sideload = false;
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/rectangles.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/8bit.exe"); //dma write mdec
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/frame-15bit.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/frame-15bit-dma.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/frame-24bit.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/frame-24bit-dma.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/DCTBlockDecodeCLUT4BPP.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/DCTBlockDecodeCLUT8BPP.exe"); // cpu write mdec
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/psxtest_cpu.exe");
//const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/access-time.exe");
const char* g_sideloadPath = ("D:/projects/emulation_psx/binaries/tests/tests/timers/timers.exe");
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

//Games that are not working

//Panzer Bandit, Conveni Special
//Gunners Heaven
//tokimeki memorial 2



//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/.bin");


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Formula One 99 (Europe) (En,Fr,De,It).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vampire Hunter D (USA).bin");//no menue graphic
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spot Goes to Hollywood (USA)/Spot Goes to Hollywood (USA) (Track 01).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (Europe) (En,Fr,De,Es,It).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (v1.1).bin");//GetTN, Mdec // no input fixed//get stuck// DMA 2 linked-list loop ??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo (USA) (v1.0).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Rebel Assault II - The Hidden Empire (USA) (Disc 1).bin");// DMA3 CDROM 0x11400100h //unhandled timer0 mode 0x14b //mdec //hangs??->mdec

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Hot Wheels - Turbo Racing (USA)/Hot Wheels - Turbo Racing (USA) (Track 01).bin"); // get stuck //needs but stil doesnt boot/*if (pc == 0x80101310) r[2] = 1; */

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Perfect Assassin (Europe) (EnFrDeEsIt)/Perfect Assassin (Europe) (En,Fr,De,Es,It) (Track 1).bin"); //no sound fixed//something is wrong with gameplay

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ape Escape (USA).bin"); //require dualshock controller

//have game breaking bugs

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/007 - The World Is Not Enough (USA).bin");// DMA3 CDROM 0x11400100h //low fps //mdec intro mission problem needs manual firing dma1 irq // gets stuck after pressing start can be bypassed by comenting emulation paused in r32 and w32 ram access

//worked before


//Working ones

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Planet of the Apes (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Soukyuu Gurentai - Oubushutsugeki (Japan)/Soukyuu Gurentai - Oubushutsugeki (Japan) (Track 01).bin");// ~[CD_DRIVE] EMULATION PAUSED!invalid setlock ass value 0x60

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Threads of Fate (USA).bin");//Pitch modulation

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Shadow Madness (USA) (Disc 1)/Shadow Madness (USA) (Disc 1) (Track 1).bin"); // get stuck on mdec needs hack in dma 1

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Fade to Black (USA) (En,Fr,De,Es,It).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/WWF Attitude (USA)/WWF Attitude (USA) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Koudelka (USA) (Disc 1).bin"); //no fmvs, mdec-dma timing //sound buffer overflow, Pitch Modulation

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ace Combat 2 (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Air Combat (USA)/Air Combat (USA) (Track 01).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Harry Potter and the Sorcerer's Stone (USA) (En,Fr,Es).bin");//spu IRQ senzitive and crashes emulator
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Doom/Doom (Track 1).bin"); // [BUS] EMULATION PAUSED! unhandled write16 adress 0x1f80105a data 0x50??  1F80105Ah 2  SIO_CTRL Serial Port Control (R/W) 
// //~[CD_DRIVE] EMULATION PAUSED! invalid setlock asect value 0x8a// Play // probaly timing issues (waitClocksFirstResponse)//gte //problems with drawing after implementing new rasterizer algorithm //sends setlock before pause is executed??

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vigilante 8 - 2nd Offense (USA)/Vigilante 8 - 2nd Offense (USA) (Track 01).bin");// uses sweep
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vigilante 8 - 2nd Offense (Europe)/Vigilante 8 - 2nd Offense (Europe) (Track 01).bin");// get stuck// uses sweep

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/NHL FaceOff 2000 (USA).bin"); // needs waitClocksDataResponse + 0x36cd2 * 14 / 10 fixed??, and crashes //fixed
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/NHL FaceOff 2001.bin"); //crashes//fixed

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Jersey Devil (USA)/Jersey Devil (USA) (Track 01).bin"); //playing sound problem and gets stuck fixed // one pixel gaps

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Evil Dead - Hail to the King (USA) (Disc 1).bin");//[TIMER0] EMULATION PAUSED! unhandled timer mode 0x158, //not drawing gameplay //needs dma 1 syncClocks = 0x100 to draw;
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Evil Dead - Hail to the King (Europe) (Disc 1).bin");//[TIMER0] EMULATION PAUSED! unhandled timer mode 0x158, //not drawing gameplay //needs dma 1 syncClocks = 0x100 to draw;

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Fighting Force (USA) (v1.0)/Fighting Force (USA) (v1.0) (Track 01).bin"); //[CD_DRIVE] EMULATION PAUSED! inaccessible CD sector! probably calculates size wrong while playing tracks fixed??//and gets stuck??

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/SaGa Frontier (USA).bin"); // no sound - problem witx XA-ADPCM, and more sound problems fixed//pitch modulation //one pixel drawing offset

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Polaris SnoCross (USA).bin");// needs /*if (pc == 0x800133fc) r[2] = 1; */

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Die Hard Trilogy (USA) (v1.0)/Die Hard Trilogy (USA) (v1.0) (Track 01).bin"); //[CD_DRIVE] EMULATION PAUSED! not recognized CD sector! 0x7 fixed

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Syphon Filter 3 (USA).bin"); // needs firing dma4 irq fixed with sync block clocks and waitClocksDataResponse + 0x36cd2 * 1 / 10 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Syphon Filter 2 (USA) (Disc 1).bin"); // needs firing dma4 irq fixed and waitClocksDataResponse + 0x36cd2 * 1 / 10 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Jet Moto 3 (USA).bin"); // needs firing dma4 irq fixed

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Duke Nukem - Land of the Babes (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Fear Effect (USA) (Disc 1).bin"); //get stuck //mdec dma timing //needs dma 1 syncClocks = 0x100;

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Dino Crisis 2 (USA)/Dino Crisis 2 (USA) (Track 1).bin"); //get stuck //needs dma 1 syncClocks = 0x400;
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Dino Crisis [NTSC-U] [SLUS-00922]/Dino Crisis (USA) (v1.0) (Track 1).bin"); // get stuck in gameplay fixed by reworking gpu timing//no fmvs //needs dma 1 syncClocks = 0x400;

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lucky Luke - Western Fever (Europe) (En,Fr,De,Es,It,Nl).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ninja - Shadow of Darkness (USA) SLUS-00435/Ninja - Shadow of Darkness (USA) (Track 01).bin");//no playing music and crashes and needs gtn to return only one track fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Xena - Warrior Princess (USA).bin"); //[TIMER1] EMULATION PAUSED! unhandled timer mode 0x48
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Frogger (USA)/Frogger (USA) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Monkey Magic (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Legend of Mana.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Railroad Tycoon II (USA)/Railroad Tycoon II (USA) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Misadventures of Tron Bonne, The (USA)/Misadventures of Tron Bonne, The (USA) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Galerians (USA) (Disc 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Legend of Dragoon, The (USA) (Disc 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Jade Cocoon - Story of the Tamamayu (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Ocean - The Second Story (USA) (Disc 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Men in Black - The Series - Crashdown (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Men in Black - The Game (Europe).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tenchu - Stealth Assassins (USA) (v1.1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Alone in the Dark - The New Nightmare (USA) (Disc 1)/Alone in the Dark - The New Nightmare (USA) (Disc 1) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Shadow Master (USA)/Shadow Master (USA) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/True Pinball (USA)/True Pinball (USA) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/T.R.A.G. - Mission of Mercy (USA)/T.R.A.G. - Mission of Mercy (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Casper (U) SLUS-00162/Casper (Track 1).bin");//problems with sound fixed //relies on spu timing


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Galaxian 3 (Europe)/Galaxian 3 (Europe) (Track 1).bin"); // + 0x36cd2 * 20 / 10, woks too slow fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Dead or Alive (USA)/Dead or Alive (USA) (Track 01).bin"); // problem with background music after pausing fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Diablo (USA) (En,Fr,De,Sv).bin");//works better with waitClocksDataResponse + 0x36cd2 * 14 / 10 fixed?? //One pixel drawing offset 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomba! 2 - The Evil Swine Return/Tomba! 2 - The Evil Swine Return (Track 1).bin");//waitClocksDataResponse + + 0x36cd2 * 1 / 10, [CD_DRIVE] EMULATION PAUSED!Issued SeekL before Setlock! fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mizzurna Falls (English v0.8).bin"); //get stuck sometimes
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Chrono Cross (USA) (Disc 1).bin"); // [BUS] EMULATION PASUED! unhandled read8 0x1f801c08 - [BUS] EMULATION PASUED!unhandled read8 0x1f801d78

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/King's Field (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Nightmare Creatures II (USA)/Nightmare Creatures II (USA) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/WarCraft II - The Dark Saga (USA) (En,Fr,De,Es,It) [SLUS-00480]/WarCraft II - The Dark Saga (USA) (En,Fr,De,Es,It).bin"); // sound problems music is stoping for a litle time


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Alundra (USA) (v1.1).bin");//mute//unhandled timer1 mode 0x149 //going enterier get stuck -> waits adsrLevel to become 0

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Nickelodeon SpongeBob SquarePants - SuperSponge (Europe).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter II Movie (Japan) (Disc 1)/Street Fighter II Movie (Japan) (Disc 1) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter Alpha 2 (USA) (Rev 1)/Street Fighter Alpha 2 (USA) (Rev 1) (Track 1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter Collection (USA) (Disc 1)/Street Fighter Collection (USA) (Disc 1) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter Collection 2 (USA)/Street Fighter Collection 2 (USA) (Track 1).bin"); //graphic bugs with health bar


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy IX (USA) (Disc 1) (v1.1).bin"); //some mdec problems and no fmv sound //pitch modulation
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy VIII (USA) (Disc 1).bin"); //mdec timing issues and fmv sound // graphic issues due to texture window seems fixed //[BUS] EMULATION PASUED! unhandled read8 0x1f801d68//fmv sound problem
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy Tactics (USA).bin");//pitch modulation
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Final Fantasy VII (USA) (Disc 1).bin"); //mdec//~[GPU] EMULATION PAUSED! unhandled GPU1 command 10 subfunction : 0x10000002 //wierd one pixel short in credits picture

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tony Hawk's Pro Skater 4 (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tony Hawk's Pro Skater 3 (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tony Hawk's Pro Skater 2 (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tony Hawk's Pro Skater (USA).bin");//very low fps

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Alien Resurrection (Europe) (En,Fr,De,Es,It)/Alien Resurrection (Europe) (En,Fr,De,Es,It) (Track 1).bin");//screen problem fixed??


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Championship Motocross 2001 featuring Ricky Carmichael (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Syphon Filter (USA) (v1.1).bin");// DMA 2 linked-list loop // screen position problem

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Parasite Eve II (USA) (Disc 1).bin"); //sound not ending problem
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Parasite Eve (USA) (Disc 1).bin");// [BUS] EMULATION PASUED!unhandled read8 0x1f801d68 [BUS] EMULATION PASUED!unhandled read8 0x1f801d78 //Pitch Modulation


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vib-Ribbon/Vib-Ribbon (Europe) (En,Fr,De,Es,It) (Track 1).bin");//getlocp loop or gte // problem with tracks // no level generation //spu irq problem // does not play tracks sets spuControlRegister.elem.cdAudioEnable to 0 and cd volume to zero

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Need for Speed III - Hot Pursuit (USA).bin"); //invalid setlock amm value 0xe3 getTD problem // reads problem, needs spu regs

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Cave Story (Test 5).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Dark Forces (USA).bin"); //graphic bugs like in Doom
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Masters of Teras Kasi (USA)/Star Wars - Masters of Teras Kasi (USA) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Episode I - The Phantom Menace (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Episode I - Jedi Power Battles (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Klonoa - Door to Phantomile (USA).bin"); //graphic bugs in menue fixed by texture window rework


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/MediEvil (USA)/MediEvil (USA) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Vagrant Story (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/C - The Contra Adventure (USA)/C - The Contra Adventure (USA) (Track 01).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Contra - Legacy of War (USA)/Contra - Legacy of War (USA) (Track 01).bin");//~[DMA2] EMULATION PAUSED! (GPU) madr: 0 and same for cd drive dma // load delay MAYBE

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Jinx (Europe) (En,Fr,De,Es,It).bin"); //slowdowns fixed probably

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Skullmonkeys.bin");//~[DMA2] EMULATION PAUSED! (GPU) madr: 0 and same for cd drive dma // load delay MAYBE // works with patch read32 0x800131FC to 0;
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Skullmonkeys (Europe).bin"); // works with patch read32 0x800131FC to 0;


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rayman Rush (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rayman 2 - The Great Escape (USA) (En,Fr,Es)/Rayman 2 - The Great Escape (USA) (En,Fr,Es) (Track 1).bin"); // [CP0] EMULATION PAUSED! triggered exception code: 0xc -> cut scenes must be skipped // causes by spu irq //sync problem
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rayman/Rayman (USA) (Track 01).bin");//VSync: timeout // needs auto pause //mising sounds in fmv fixed??//one pixel offset graphic bug//no music in gameplay

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Yu-Gi-Oh! - Forbidden Memories (Europe).bin");//probably stucked because unhandled spu regs


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Black Dawn/Black Dawn (USA) (Track 01).bin");//SsVabOpenHead : failed! ~needs spu to boot?? // crashes if cpu is too fast //acceseing inaccesable sectors because og getTD//EMULATION PAUSED! invalid setlock ass value 0x61


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tiny Tank [U] [SCUS-94427].bin");//problem with input// fixed with implemted spu??

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Silent Hill (v1.1).bin");//stuck // problem with mem card//hangs maybe spu regs?? needs reverb
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Silent Hill (USA).bin");//stuck // problem with mem card // like no input// maybe spu regs ?? needs reverb


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Castlevania - Symphony of the Night (USA) (Track 1).bin"); //mdec //getstat loop // mem card reading problem?? //setfilter
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Castlevania Chronicles (USA).bin");//SPU:T/O [wait (IRQ/OFF)] //Pads?? //[KERNEL] B(3Fh) or A(3Eh) std_out_puts(src) //[KERNEL] B(5Bh) ChangeClearPad(int) //[KERNEL] C(0Ah) ChangeClearRCnt(t, flag) // gaphic glitches at caracter sprites

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Metal Gear Solid (Disc 1) (v1.1).bin"); //gte // np input //changeThreadHandle //faster code execution then audio can break game (more fps then 60)


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Wipeout XL (USA)/Wipeout XL (USA) (Track 01).bin"); //get stuck disk read errors // uses sweep // //sound buffer overflow //uses serial //muting cd audio
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/WipEout (USA)/WipEout (USA) (Track 1).bin"); // can be bypassed with tracks 1 1 and amm 0 ass 2 getTD and getTN // uses sweep // //sound buffer overflow //uses serial
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/WipEout 2097 (Europe)/WipEout 2097 (Europe) (Track 01).bin");// something goes wrong with gpu fifo// can be bypassed with tracks 1 and amm 0 ass 2 getTD and getTN // uses sweep // //sound buffer overflow //uses serial

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mickey's Wild Adventure (Europe)/Mickey's Wild Adventure (Europe) (Track 01).bin"); // hangs fixed?? // can be bypassed with tracks 1 and amm 0 ass 2 getTD and getTN fixed // dam 2 linked list loop
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Star Wars - Demolition (USA)/Star Wars - Demolition (USA) (Track 01).bin"); //get stuck // can be bypassed with tracks 1 and amm 0 ass 2 getTD and getTN


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Metal Slug X.bin"); // DMA 2 linked-list loop 


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/CT Special Forces - Back to Hell (Europe) (En,Fr,De,Es,It,Nl).bin");// setfilter


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Bubble Bobble also featuring Rainbow Islands (Track 1).bin"); //bypass mdec amm 1 ass 2 assec 15 => amm 2 ass 44 asect 15 ???
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Puzzle Bobble 2 (Japan) (Track 01).bin");


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Valkyrie Profile (Disc 1).bin");//probably stuct at 1F801DAAh 2  SPU Control Register (SPUCNT)?? // spu irq sync sensitive
// graphic bugs around character pictures at left side while in dialog looks fixed with adding leftover bits for 4bit and 8bit textures

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tactics Ogre (USA)/Tactics Ogre (USA) (Track 1).bin");//fps drop
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Wild Arms 2 (USA) (Disc 1).bin");//STuCK getTN fixed?? //STUCk// software terminated // graphic bugs in menue fixed by texture window rework
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Digimon World 3.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Chippoke Ralph no Daibouken - The Adventure of Little Ralph (Japan).bin"); //has one pixel texture off
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Arc the Lad Collection - Arc the Lad.bin");//[BUS] EMULATION PASUED! unhandled addr read32 0x1f801014 1F801014h 4    SPU_DELAY   Delay/Size (usually 200931E1h) // gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lunar - Silver Star Story Complete (USA) (Disc 1)/Lunar - Silver Star Story Complete (USA) (Disc 1) (Track 1).bin"); //code execution goes wrong // 0x80000080 overwriten 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lunar 2 - Eternal Blue Complete (USA) (Disc 1).bin");// get stuck?? fixed??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Adventures of Lomax, The (USA)/Adventures of Lomax, The (USA) (Track 01).bin"); //GetTN , addr 0x1f8005e0 ?? fixed maybe // gte??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Street Fighter Alpha 3/Street Fighter Alpha 3 (Track 1).bin"); //GetTN-, invalid setlock asect value 0x8c setmode CDDA bit set?? //but works
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Rapid Reload (Europe).bin");// setfilter //GetlocP // gte // but works
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Harmful Park (English v1.1).bin");//Stop //Setfilter // but works???
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gundam Battle Assault 2/Gundam Battle Assault 2 (Track 1).bin");//mdec??
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Breath of Fire III/Breath of Fire III (v1.1) (Track 1).bin"); //cd drive out of sync ?? fixed??//~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x4c787878 Undocumented/Nonsense
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Captain Commando (Japan).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Grandia (Europe) (Disc 1).bin");//Getparam //gte // wont boot , needs late INT1 // senzitive on too fAST CPU

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Discworld Noir (Europe) (En,Es,It).bin"); //get stuck mdec sometimes?
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Raiden Project, The (USA).bin");//gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo 2 (Europe) (En,Fr,De,Es,It) (Disc 1) (Arcade Mode).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Gran Turismo 2 (USA) (Simulation Mode) (Rev 1).bin");//gte //senzitive to stat bits of cd drive // get stuck on exiting sometimes// crashs when exiting race sometimes seems memcard timings fixes it// no intro fmv
//case 0xc:
//data = voices.voiceRegisters[voiceIndex].adsrLevel = 0;
//break;

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/gta/Grand Theft Auto (USA) (Track 01).bin"); // DMA3 CDROM 0x11400100h (chopped) //~[GPU] EMULATION PAUSED! unhandled GPU0 command 0x4c7f7f7f Undocumented/Nonsense
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Grand Theft Auto 2 (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Oddworld - Abe's Oddysee (Europe).bin"); // DMA3 CDROM 0x11400100h (chopped)..//but woks //mdec
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Oddworld - Abe's Exoddus (USA) (Disc 1).bin"); // DMA3 CDROM 0x11400100h (chopped)

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/007 - Tomorrow Never Dies (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mega Man X3/Mega Man X3 (Europe) (Track 01).bin"); // DMA3 CDROM 0x11400100h (chopped)
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mega man x4.bin"); // setfilter // getlocp //mdec?  // but works

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Colin McRae Rally 2.0/Colin McRae Rally 2.0 (Europe) (Track 1).bin");//timer mode 0x158

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spec Ops - Airborne Commando.bin"); //ChangeThread stuck fixed probably

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Monkey Hero (USA).bin");//gte//problem with character sprite seems it depends of cpu speed // problem with spu dram transfer

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Mortal Kombat 4.bin");//mdec //no input/gte
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombat3/Mortal Kombat 3 (Track 01).bin"); //mdec //test event cdrom (waitClocksDataResponse + 0x36cd2*2) fixed?? // cache  //SeekP //play?? exeption 04//problem with getTn getTd probably needs cue parser fixed??//match ending screen ends too fast 
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/mortal_kombatII/Mortal Kombat II (Japan) (Track 1).bin");//gets too hard too quickly

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot Collection.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/ctr.bin");//probably stuct at 1F801DAAh 2  SPU Control Register (SPUCNT) SPU:T/O [wait (IRQ/OFF)] //crashes sometimes
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot - Warped (USA).bin"); //gte? //1f8008cf unhandled addr read8 // needs 44100*46/64 sampling rate for cutscenes to work // or if ((g_systemClocks % 0x340) == 0)
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/crash2.bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crash Bandicoot (Europe) (EDC).bin");//gte // some sounds are sometimes missing

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/crash bash.bin"); //GetTN ?! //cdrom probably 1F801803h.Index1 read stuck fixed // pause screen missing textures

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro the Dragon (Europe) (En,Fr,De,Es,It).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro the Dragon (USA).bin");//no input//trows an exeption
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro 2 - Ripto's Rage! (USA).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro 2 - Gateway to Glimmer (Europe) (En,Fr,De,Es,It).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro - Year of the Dragon (USA) (v1.1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro - Year of the Dragon (Europe) (En,Fr,De,Es,It) (v1.1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spyro - Year of the Dragon (Europe) (En,Fr,De,Es,It) (v1.0).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/lt.bin"); //setfilter// setmode 0xca// getlocL // dont pass nowhere select gte related//stops execution //needs clocks >= waitClocksFirstResponse+ <= 0x36cd2 * 5 / 20 fixed??
const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/bb.bin"); //setfilter// setmode 0x4a// getlocL//GTE //needs + 0x36cd2 * 8 / 10 fixed

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Lucky_Luke_(USA)/Lucky Luke (USA) (Track 01).bin"); // setfilter //mdec // but works //getTN firstResponse = { stat, 0x1, 0x23 }; //getTD firstResponse = { stat, 20, 40 };

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Looney Tunes - Sheep Raider.bin"); //mdec? //GetlocP // 0xffff93f0 unhandled fixed??//gets stuck at level 1



//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Tarzan/Disney's Tarzan (v1.1) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Hercules Action Game (v1.1).bin"); //mdec //gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Aladdin in Nasira's Revenge.bin");//gte // no input!!! if cpu is too fast //spu irq problem

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's 102 Dalmatians - Puppies to the Rescue (USA).bin"); // has some strange draging

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Atlantis - The Lost Empire (USA)/Disney's Atlantis - The Lost Empire (USA) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Treasure Planet (USA).bin"); //missing sounds in fmvs fixed??

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Disney's Donald Duck - Quack Attack (Europe) (En,Fr,De,It).bin"); //does not pass loading screen get stuck //last time tested 9.5.2024 fixed // PAL clocks sensitive


//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ridge Racer (USA) (Track 01).bin"); //gte //play

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Resident Evil (Europe).bin");//getlocp and setfilter//CD_init:addr=800ad268 fixed?? //low fps //audio sync
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/resident evil 2/Resident Evil 2 (Disc 1) (Track 1).bin");//gte //spaming read mdec status addr == 0x1f801824
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Resident Evil 3 - Nemesis (USA).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/ISS Pro Evolution/ISS Pro Evolution (Track 01).bin"); //no input // sendingClocks = 0x1200;//receivingClocks = 0x800; //ackClocks = 0x20;//transfer clocks

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/tekken3/Tekken 3 (Track 1).bin"); //mdec //no input?
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tekken 2 (USA) (v1.1)/Tekken 2 (USA) (v1.1) (Track 1).bin"); // DMA 2 linked-list loop ??//gte load delays

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Driver - You Are the Wheelman.bin");//no input // crashes when entering film director
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Driver 2 (USA) (Disc 1) (v1.1)/Driver 2 (USA) (Disc 1) (v1.1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/World's Scariest Police Chases (USA).bin"); // invalid addr [BUS] EMULATION PAUSED! invalid ram addr write8 0x3000061 data 0x1 fixed maybe // no input

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/tomb raider 3/Tomb Raider III - Adventures of Lara Croft (Europe).bin"); //getloocp loop, no input //but works
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomb Raider II - Starring Lara Croft (Europe)/Tomb Raider II - Starring Lara Croft (Europe) (Track 01).bin");//hangs/VSync: timeout
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomb Raider - The Last Revelation (USA) (v1.1).bin");
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tomb Raider (v1.1)/Tomb Raider (v1.1) (Track 01).bin"); // GetlocP loop??//VSync: timeout

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Spider-Man (USA).bin");//very low fps

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tom Clancy's Rainbow Six - Lone Wolf (USA).bin"); //sound buffer overflow
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Quake II/Quake II (USA) (Track 1).bin");//gte

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Ace Combat 3 - Electrosphere (USA).bin"); //mem card problem // texture window problem // fixed by texture window rework

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Formula 1 (Europe) (En,Fr,De,Es,It) (Rev 1)/Formula 1 (Europe) (En,Fr,De,Es,It) (Rev 1) (Track 01).bin"); //sound buffer overflow

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Next Tetris, The (USA)/Next Tetris, The (USA) (Track 01).bin");//hangs // needs slower cd drive + 0x36cd2 * 14 / 10 fixed //problem with playing tracks // playes wrong track
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Medal of Honor (USA).bin");//gte //stuck
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Crusader - No Remorse.bin"); //cd drive out of sync // cutscenes need to be skipped 

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Twisted Metal 2/Twisted Metal 2 (USA) (Track 01).bin"); //[KERNEL] A(3Ah) SystemErrorExit(exitcode) not if intro is skipped
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Twisted Metal (USA)/Twisted Metal (USA) (Track 1).bin");

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Tank Racer (Europe) (En,Fr,De,Es,It)/Tank Racer (Europe) (En,Fr,De,Es,It) (Track 01).bin");

//demos

//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/Euro Demo 67 (Europe).bin"); //Ms pacman problem seems working now // input doesnt work asterix slowing down cpu helps the same for pacman and frogger and chicken run// time crisis getTD problem

//tests
//const char* g_cdPath = ("D:/projects/emulation_psx/binaries/roms/hello_cd.bin");