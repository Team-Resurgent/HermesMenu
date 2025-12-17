#pragma once

#include "meshUtility.h"

class stl_loader
{
public:
	static utils::dataContainer* loadFromBinary(const unsigned char* data, uint32_t size);
};

