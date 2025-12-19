#pragma once

#include "context.h"
#include "External.h"
#include "math.h"
#include "alignment.h"

#include <string>
#include <map>

typedef struct Bounds
{
	int width;
	int height;
	int left;
	int top;
} Bounds;

class drawing
{
public:
	static void swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest);
	static image* createImage(uint8_t* imageData, D3DFORMAT format, int width, int height);
	static void addImage(std::string key, uint8_t* imageData, D3DFORMAT format, int width, int height);
	static bool loadImage(const uint8_t* buffer, uint32_t length, std::string key);
	static bool loadFont(const uint8_t* data);
	static void clearBackground(uint32_t index);
	static bool imageExists(std::string key);
	static image* getImage(std::string key);
	static void setTint(unsigned int color);
	static void drawImage(image* image, uint32_t tint, int x, int y, int width, int height);
	static void drawImage(image* image, uint32_t tint, int x, int y);
	static void drawImage(std::string imageKey, uint32_t tint, int x, int y, int width, int height);
	static void drawImage(std::string imageKey, uint32_t tint, int x, int y);
	static image* renderRoundedRect(std::string cacheKey, uint32_t width, uint32_t height, uint32_t cornerRadius, uint32_t fillColor, uint32_t strokeColor, uint32_t strokeWidth);
	static image* renderHorizontalLine(std::string cacheKey, unsigned int color, int width, int height);
	static void drawHorizontalLine(uint32_t color, int x, int y, int width, int height);
	static void drawNinePatch(std::string imageKey, uint32_t tint, int x, int y, int width, int height);
	static bitmapFont* generateBitmapFont(std::string fontName, int fontStyle, int fontSize, int lineHeight, int spacing, int textureDimension);
	static void measureBitmapString(bitmapFont* font, std::string message, int* width, int* height);
	static void drawBitmapString(bitmapFont* font, std::string message, uint32_t color, int x, int y);
	static void drawBitmapStringAligned(bitmapFont* font, std::string  message, uint32_t color, horizAlignment hAlign, int x, int y, int width);
};
