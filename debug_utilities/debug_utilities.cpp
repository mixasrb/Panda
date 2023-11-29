#include "debug_utilities.h"

debugUtilities::debugUtilities() {
	std::fstream file("D:/projects/emulation_psx/vs/Panda/logs/TTY_output.txt");
	if (file.is_open()) {
		file.close();
		remove("D:/projects/emulation_psx/vs/Panda/logs/TTY_output.txt");
	}
}

void debugUtilities::dump(const char* path, void* p_memory, const size_t size) {
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open())
		std::cout << "failed to create file\n";
	file.write((char*)p_memory, size);
	std::cout << path << " dumped.\n";
	file.close();
}

void debugUtilities::log(const std::string& s) {
	std::ofstream file("D:/projects/emulation_psx/vs/Panda/logs/TTY_output.txt", std::ios::app);
	if (!file.is_open())
		std::cout << "failed to open logs/TTY_output.txt\n";
	file.write(s.c_str(), s.size());
	file.close();
}