#pragma once

#include "External.h"
#include <string>

class utils
{
public:

    typedef struct textContainer 
	{
        std::string message;
		uint32_t width;
        uint32_t height;

		textContainer() : message(""), width(0), height(0) {}

		textContainer(std::string message, uint32_t width, uint32_t height)
		{
			this->message = message;
			this->width = width; 
            this->height = height; 
		}

	} textContainer;

	typedef struct dataContainer 
	{

		char* data;
		uint32_t size;

		dataContainer() : data(NULL), size(0) {}

		dataContainer(uint32_t size)
		{
			this->data = (char*)utils::mallocWithTerminator(size);
			this->size = size; 
		}

		dataContainer(char* data, uint32_t size, uint32_t copySize)
		{
			this->data = (char*)utils::mallocCopyWithTerminator(data, size, copySize);
			this->size = size; 
		}

		~dataContainer()
		{
			free(this->data);
		}

	} dataContainer;

	typedef struct intContainer 
	{
		int32_t value;

		intContainer(int32_t value)
		{
			this->value = value;
		}

	} intContainer;

	static void debugPrint(std::string format, ...);
	static void* mallocWithTerminator(uint32_t size);
	static void* mallocCopyWithTerminator(void* source, uint32_t size, uint32_t copySize);
	static uint32_t roundUpToNextPowerOf2(uint32_t value);
};
