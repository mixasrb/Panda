#include "debug_utilities.h"

debugUtilities::debugUtilities() {
	std::fstream file("logs/TTY_output.txt");
	if (file.is_open()) {
		file.close();
		remove("logs/TTY_output.txt");
	}
}

void debugUtilities::dump(const char* path, void* p_memory, const size_t size) {
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("failed to create file");
	file.write((char*)p_memory, size);
	std::cout << path << " dumped.\n";
	file.close();
}

void debugUtilities::log(const std::string& s) {
	std::ofstream file("logs/TTY_output.txt", std::ios::app);
	if (!file.is_open())
		throw std::runtime_error("failed to open log.txt\n");
	file.write(s.c_str(), s.size());
	file.close();
}