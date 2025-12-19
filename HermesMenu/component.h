#pragma once

#include "External.h"
#include "alignment.h"

#include <string>

class component
{
public:
	static void panel(uint32_t fill, uint32_t stroke, int x, int y, int width, int height);
	static void button(bool selected, bool active, std::string label, int x, int y, int width, int height);
    static void text(std::string label, bool disabled, horizAlignment hAlign, int x, int y, int width, int height);
};
