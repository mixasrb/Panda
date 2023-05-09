#pragma once

#include <iostream>
#include <fstream>

class debugUtilities {
public:
	debugUtilities();
	void dump(const char* path, void* p_memory, const size_t size);
	void log(const std::string& s);
};

