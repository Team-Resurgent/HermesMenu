#include "utils.h"
#include "External.h"

#include <xtl.h>
#include <stdio.h>
#include <string>
#include <cmath>

void utils::debugPrint(std::string format, ...)
{
#ifdef _DEBUG
	va_list args;
    va_start(args, format);

	uint32_t length = _vsnprintf(NULL, 0, format.c_str(), args);

	char* message = (char*)malloc(length + 1);
	_vsnprintf(message, length, format.c_str(), args);
	message[length] = 0;

    va_end(args);

	OutputDebugStringA(message);
	free(message);
#endif
}

void* utils::mallocWithTerminator(uint32_t size)
{
	char* result = (char*)malloc(size + 1);
	result[size] = 0;
	return (void*)result;
}

void* utils::mallocCopyWithTerminator(void* source, uint32_t size, uint32_t copySize)
{
	void* result = mallocWithTerminator(size);
	memcpy(result, source, copySize);
	return result;
}

uint32_t utils::roundUpToNextPowerOf2(uint32_t value) 
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}