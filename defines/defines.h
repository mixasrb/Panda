#pragma once

//exeption codes
#define _INT 0x00
#define _MOD 0x01
#define _TLBL 0x02
#define _TLBS 0x03
#define _ADEL 0x04
#define _ADES 0x05
#define _IBE 0x06
#define _DBE 0x07
#define _SYSCALL 0x08
#define _BP 0x09
#define _RI 0x0a
#define _CPU 0x0b
#define _OV 0x0c

//interrupts
#define _IRQ_V_SYNC_ 0x01
#define _IRQ_CD_ROM 0x04
#define _IRQ_DMA 0x08
#define _IRQ_TIMER_0 0x00
#define _IRQ_TIMER_1 0x20
#define _IRQ_TIMER_2 0x40
#define _IRQ_7 0x80
#define _IRQ_SPU 0x200

#define BAD_V_ADDR cop0r[8]
#define SR cop0r[12]
#define CAUSE cop0r[13]
#define EPC cop0r[14] 


//timer clock sources
#define _SYSTEM_CLOCK 0
#define _HBLANK_CLOCK 1
#define _VBLANK_CLOCK 2
#define _DOT_CLOCK 3

//drive status
#define _DOOR_OPEN 0
#define _SPIN_UP 1
#define _DETECT_BUSY 2
#define _NO_DISC 3
#define _AUDIO_DISC 4
#define _UNLICENCED_DISC_MODE_1 5
#define _UNLICENCED_DISC_MODE_2 6
#define _UNLICENCED_DISC_MODE_2_AUDIO 7
#define _DEBUG_YAROZE_MODE_2 8
#define _LICENCED_MODE_2 9
#define _MODCHIP_AUDIO_MODE_1 10


