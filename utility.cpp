#include "utility.h"
#include <sstream>
std::string to_hex_string(const char * data, size_t length)
{
	std::stringstream ss;
	for(size_t i = 0; i < length; ++i)
	{
		ss << std::hex << (unsigned short)((unsigned char)data[i]) << " ";
	}
	return ss.str();
}
