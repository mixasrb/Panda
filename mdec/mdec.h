#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

class busInterface_t;

class mdec_t {
public:

	mdec_t();

	busInterface_t* pBus;

	void readMdec32(const uint32_t& addr, uint32_t& data);
	void writeMdec32(const uint32_t& addr, const uint32_t& data);

	void clock();
	uint32_t clocks;


	union mdecStatusRegister_t {
		uint32_t data;
		struct {
			uint32_t numberOfParam : 16;
			uint32_t currentBlock : 3;
			uint32_t unused : 4;
			uint32_t dataOutputBit15 : 1;
			uint32_t dataOutputSigned : 1;
			uint32_t dataOutputDepth : 2;
			uint32_t dataOutRequest : 1;
			uint32_t dataInRequest : 1;
			uint32_t commandBusy : 1;
			uint32_t dataInFifoFull : 1;
			uint32_t dataOutFifoEmpty : 1;
		}elem;
	}statusRegister;

	union mdecControlRegister_t {
		uint32_t data;
		struct {
			uint32_t unused : 29;
			uint32_t dataOutRequest : 1;
			uint32_t dataInRequest : 1;
			uint32_t resetMdec : 1;
		}elem;
	}controlRegister;

	enum blockType_t {
		Y1,
		Y2,
		Y3,
		Y4,
		Cr,
		Cb
	};

	uint8_t* quantTableY = new (uint8_t)(64);
	uint8_t* quantTableUV = new (uint8_t)(64);
	uint16_t* scaleTable = new (uint16_t)(64);
	uint8_t* source = new(uint8_t)(128);

	void decodeMacroblock(uint32_t command);
	void setQuantTable(uint32_t command);
	void setScaleTable(uint32_t command);
	void noFunction(uint32_t command);
	void invalid(uint32_t command);

	struct command_t {
		void (mdec_t::* command)(uint32_t command);
		std::string name;
	};

	std::vector<command_t> commandLookup;

	std::vector<uint32_t> recievedData32;
	uint32_t recievedData32Count = 0;
	uint32_t command;

	bool bEnabledDma0 = false;
	bool bEnabledDma1 = false;

	uint16_t* src;
	uint32_t* dst = new (uint32_t)((8+8)*16);
	int dst_num;

	uint16_t Crblk[64];
	uint16_t Cbblk[64];
	uint16_t Yblk[64];

	std::vector<uint8_t> zigzag;
	std::vector<uint8_t> zagzig;

	void rl_decode_block(uint16_t* blk, uint16_t*& src, uint16_t* qt);

	uint16_t* temb_buffer = new (uint16_t)(64);
	void real_idct_core(uint16_t* blk);
	void yuv_to_rgb(int xx, int yy);
};