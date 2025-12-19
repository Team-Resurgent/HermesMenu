#include "StringUtility.h"
#include "Integers.h"

#include <algorithm>
#include <xtl.h>

std::string StringUtility::FormatString(std::string format, ...) 
{
    va_list args;
    va_start(args, format);

    uint32_t length = _vsnprintf(NULL, 0, format.c_str(), args);

    char* buffer = (char*)malloc(length + 1);
    _vsnprintf(buffer, length, format.c_str(), args);
    buffer[length] = 0;

    va_end(args);

    std::string result = std::string(buffer);
    free(buffer);
    return result;
}

std::string StringUtility::LowerCase(std::string const& value)
{
	std::string result = std::string(value);
	std::transform(result.begin(), result.end(), result.begin(), tolower);
	return result;
}

bool StringUtility::EndsWith(std::string const& value, std::string const& ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> StringUtility::Split(std::string const& value, char const& delimiter)
{
	std::string buffer;
	std::vector<std::string> items;
	for (size_t i = 0; i < value.size(); i++)
	{
		if (value.at(i) == delimiter)
		{
			items.push_back(buffer);
			buffer.clear();
			continue;
		}
		buffer.push_back(value.at(i));
	}
	if (!buffer.empty())
	{
		items.push_back(buffer);
	}
	return items;
}