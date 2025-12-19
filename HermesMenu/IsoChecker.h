#pragma once

#include <vector>
#include <string>

class IsoChecker
{
public:
	static std::vector<std::string> ValidateIsosInPath(std::string path, bool isLegacy, bool &isCci);
};
