#pragma once

#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>

#define BIOS_SIZE 512 * 1024

class biosChip_t {
public:
	biosChip_t();

	uint8_t* biosData = new uint8_t[BIOS_SIZE];

	struct functions {
		std::string info;
	};

	std::vector<functions> A;
	std::vector<functions> B;
	std::vector<functions> C;
};