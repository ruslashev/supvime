#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>
#include <cstdarg>

void throwf(const char *format, ...);
void assertf(bool condition, const char *format, ...);

#endif
