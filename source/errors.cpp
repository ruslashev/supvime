#include "errors.hpp"

void throwf(const char *format, ...)
{
	char buffer[512];
	va_list list;
	va_start(list, format);
	vsnprintf(buffer, 512, format, list);
	va_end(list);
	std::string output = buffer;
	output += '\n';
	throw std::runtime_error(output);
}

void assertf(bool condition, const char *format, ...)
{
	if (condition)
		return;
	char buffer[512];
	va_list list;
	va_start(list, format);
	vsnprintf(buffer, 512, format, list);
	va_end(list);
	std::string output = buffer;
	output += '\n';
	throw std::runtime_error(output);
}

