#include "errors.hpp"

void throwf(const char *format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 512, format, args);
	const std::string output = buffer;
	va_end(args);
	throw std::runtime_error(output);
}

