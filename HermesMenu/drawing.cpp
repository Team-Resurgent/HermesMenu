#include "drawing.h"
#include "context.h"
#include "math.h"
#include "utils.h"
#include "meshUtility.h"
#include "StringUtility.h"
#include "theme.h"

#include <map>
#include <xgraphics.h>

#define SSFN_IMPLEMENTATION
#define SFFN_MAXLINES 8192
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free
#include "ssfn.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#include "stb_image.h"

namespace
{
	ssfn_t* mFontContext = NULL;
	//ssfn_buf_t* mFontBuffer = NULL;

	//SSFN::Font* mFont = NULL;
}

inline unsigned char lerp(unsigned  char a, unsigned char b, float t)
{
	return (unsigned char)(a + t * (b - a));
}

inline int length(int a, int b)
{
	return (a * a) + (b * b);
}

void drawing::swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest)
{
  for (UINT y = 0; y < height; y++)
  {
    UINT sy = 0;
    if (y < width)
    {
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
    }
    else
    {
      UINT y_mask = y % width;
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y_mask >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
      sy += (y / width) * width * width;
    }
    BYTE *s = (BYTE *)src + y * width * depth;
    for (UINT x = 0; x < width; x++)
    {
      UINT sx = 0;
      if (x < height * 2)
      {
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x >> bit) & 1) << (2*bit);
      }
      else
      {
        int x_mask = x % (2*height);
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x_mask >> bit) & 1) << (2*bit);
        sx += (x / (2 * height)) * 2 * height * height;
      }
      BYTE *d = (BYTE *)dest + (sx + sy)*depth;
      for (unsigned int i = 0; i < depth; ++i)
        *d++ = *s++;
    }
  }
}

	//uint8_t* pixels = imageData;
	//for (int i = 0; i < width * height * 4; i+=4)
	//{
	//	// swap bgra to rgba
	//	unsigned char temp = pixels[i + 0];
	//	pixels[i + 0] = pixels[i + 2];
	//	pixels[i + 2] = temp;
	//}

image* drawing::createImage(uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	image* imageToAdd = new image();
	imageToAdd->width = width;
	imageToAdd->height = height;

	if (FAILED(D3DXCreateTexture(context::getD3dDevice(), imageToAdd->width, imageToAdd->height, 1, 0, format, D3DPOOL_DEFAULT, &imageToAdd->texture)))
	{
		return false;
	}

	D3DSURFACE_DESC surfaceDesc;
	imageToAdd->texture->GetLevelDesc(0, &surfaceDesc);

	imageToAdd->uvRect = math::rectF(0, 0, imageToAdd->width / (float)surfaceDesc.Width, imageToAdd->height / (float)surfaceDesc.Height);

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(imageToAdd->texture->LockRect(0, &lockedRect, NULL, 0)))
	{
		uint8_t* tempBuffer = (uint8_t*)malloc(surfaceDesc.Size);
		memset(tempBuffer, 0, surfaceDesc.Size);
		uint8_t* src = imageData;
		uint8_t* dst = tempBuffer;
		for (int32_t y = 0; y < imageToAdd->height; y++)
		{
			memcpy(dst, src, imageToAdd->width * 4);
			src += imageToAdd->width * 4;
			dst += surfaceDesc.Width * 4;
		}
		swizzle(tempBuffer, 4, surfaceDesc.Width, surfaceDesc.Height, lockedRect.pBits);
		free(tempBuffer);
		imageToAdd->texture->UnlockRect(0);
	}

	return imageToAdd;
}

void drawing::addImage(std::string key, uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	image* imageToAdd = createImage(imageData, format, width, height);
    std::map<std::string, image*>* map = context::getImageMap();
	(*map)[key] = imageToAdd;
}

bool drawing::loadImage(const uint8_t* buffer, uint32_t length, std::string key)
{
	int width;
	int height;
	uint8_t* imageData = (uint8_t*)stbi_load_from_memory((const stbi_uc*)buffer, length, &width, &height, NULL, STBI_rgb_alpha);
	if (imageData == NULL)
	{
		return false;
	}
	addImage(key, imageData, D3DFMT_A8B8G8R8, width, height);
	free(imageData);
	return true;
}

bool drawing::loadFont(const uint8_t* data)
{
	if (mFontContext == NULL)
	{
		mFontContext = (ssfn_t*)malloc(sizeof(ssfn_t));                         
		memset(mFontContext, 0, sizeof(ssfn_t));
	}

	int result = ssfn_load(mFontContext, data);
	return result == 0;
}

void drawing::clearBackground(uint32_t index)
{
	uint32_t backgroundColor = theme::getBackgroundColor();
	context::getD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, (0xff << 24) | backgroundColor, 1.0f, 0L);
}

bool drawing::imageExists(std::string key)
{
    std::map<std::string, image*>* map = context::getImageMap();
    return map->find(key) != map->end();
}

image* drawing::getImage(std::string key)
{
    std::map<std::string, image*>* map = context::getImageMap();
	return (*map)[key];
}

void drawing::setTint(unsigned int color)
{
	context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, color);
}

void drawing::drawImage(image* image, uint32_t tint, int x, int y, int width, int height)
{
	if (image == NULL)
	{
		return;

	}
	context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
	float newY = (float)context::getBufferHeight() - (y + height);
	context::getD3dDevice()->SetTexture(0, image->texture);
	utils::dataContainer* vertices = meshUtility::createQuadXY(math::vec3F((float)x - 0.5f, newY - 0.5f, 0), math::sizeF((float)width, (float)height), image->uvRect);
	context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, (vertices->size / sizeof(meshUtility::vertex)) / 3, vertices->data, sizeof(meshUtility::vertex));
	delete(vertices);
}

void drawing::drawImage(image* image, uint32_t tint, int x, int y)
{
	if (image == NULL)
	{
		return;

	}
	drawImage(image, tint, x, y, image->width, image->height);
}

void drawing::drawImage(std::string imageKey, uint32_t tint, int x, int y, int width, int height)
{
	image* imageToDraw = getImage(imageKey);
	if (imageToDraw == NULL)
	{
		return;
	}
	drawImage(imageToDraw, tint, x, y, width, height);
}

void drawing::drawImage(std::string imageKey, uint32_t tint, int x, int y)
{
	image* imageToDraw = getImage(imageKey);
	if (imageToDraw == NULL)
	{
		return;
	}
	drawImage(imageToDraw, tint, x, y);
}

void drawing::drawNinePatch(std::string imageKey, uint32_t tint, int x, int y, int width, int height)
{
	image* imageToDraw = getImage(imageKey);
	if (imageToDraw == NULL)
	{
		return;
	}
	context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
	float newY = (float)context::getBufferHeight() - (y + height);
	context::getD3dDevice()->SetTexture(0, imageToDraw->texture);
	utils::dataContainer* vertices = meshUtility::createNinePatchXY(math::vec3F((float)x - 0.5f, newY - 0.5f, 0), math::sizeF((float)width, (float)height), imageToDraw->uvRect);
	context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, (vertices->size / sizeof(meshUtility::vertex)) / 3, vertices->data, sizeof(meshUtility::vertex));
	delete(vertices);
}

image* drawing::renderHorizontalLine(std::string cacheKey, unsigned int color, int width, int height)
{
	if (imageExists(cacheKey) == false)
	{
		uint32_t* imageData = (uint32_t*)malloc(width * height * 4);
		for (int i = 0; i < width * height; i++)
		{
			imageData[i] = color;
		}
		addImage(cacheKey, (uint8_t*)imageData, D3DFMT_A8R8G8B8, width, height);
		free(imageData);
	}
	return getImage(cacheKey);
}

image* drawing::renderRoundedRect(std::string cacheKey, uint32_t width, uint32_t height, uint32_t cornerRadius, uint32_t fillColor, uint32_t strokeColor, uint32_t strokeWidth)
{
	if (imageExists(cacheKey) == false)
	{
		uint32_t* imageData = (uint32_t*)malloc(width * height * 4);
		
		int widthMinus1 = width - 1;
		int heightMinus1 = height - 1;
		for (unsigned int y = 0; y < height; y++) 
		{
			float gradient = (float)y / height;
			for (unsigned int x = 0; x < width; x++) 
			{
				unsigned int color = 0x00000000;

				bool isInsideRect = (y >= cornerRadius && y < height - cornerRadius) || (x >= cornerRadius && x < width - cornerRadius);
				int topLeftCornerLength = length(y - cornerRadius, x - cornerRadius);
				int topRightCornerLength = length(y - cornerRadius, x - (widthMinus1 - cornerRadius));
				int bottomLeftCornerLength = length(y - (heightMinus1 - cornerRadius), x - cornerRadius);
				int bottomRightCornerLength = length(y - (heightMinus1 - cornerRadius), x - (widthMinus1 - cornerRadius));
				int cornerRadiusSquared = cornerRadius * cornerRadius;

				bool isInsideTopLeftCorner = topLeftCornerLength < cornerRadiusSquared;
				bool isInsideTopRightCorner = topRightCornerLength < cornerRadiusSquared;
				bool isInsideBottomLeftCorner = bottomLeftCornerLength < cornerRadiusSquared;
				bool isInsideBottomRightCorner = bottomRightCornerLength < cornerRadiusSquared;
				if (isInsideRect || isInsideTopLeftCorner || isInsideTopRightCorner || isInsideBottomLeftCorner || isInsideBottomRightCorner) 
				{
					color = fillColor;
				}
				imageData[(y * width) + x] = color;
			}
		}

		if (strokeWidth > 0)
		{
			for (uint32_t y = 0; y < height; y++) 
			{
				int count = 0;
				for (uint32_t x = 0; x < width; x++) 
				{
					uint32_t color = imageData[(y * width) + x];
					if (count == strokeWidth)
					{
						break;
					}
					if (color > 0)
					{
						imageData[(y * width) + x] = strokeColor;
						imageData[(y * width) + (width - 1) - x] = strokeColor;
						count++;
					}
				}
			}

			for (uint32_t x = 0; x < width; x++)  
			{
				int count = 0;
				for (uint32_t y = 0; y < height; y++)
				{
					uint32_t color = imageData[(y * width) + x];
					if (count == strokeWidth)
					{
						break;
					}
					if (color > 0)
					{
						imageData[(y * width) + x] = strokeColor;
						imageData[(((height - 1) - y) * width) + x] = strokeColor;
						count++;
					}
				}
			}
		}
	
		//stbi_write_jpg("App:\\test.jpg", width, height, 4, imageData, 100);
		addImage(cacheKey, (uint8_t*)imageData, D3DFMT_A8R8G8B8, width, height);
		free(imageData);
	}
	return getImage(cacheKey);
}

void drawing::drawHorizontalLine(uint32_t color, int x, int y, int width, int height)
{
	std::string cacheKey = StringUtility::FormatString("drawHorizontalLine-%02x-%02x", width, height).c_str();
	image* image = renderHorizontalLine(cacheKey, color, width, height);
	drawImage(image, color, x, y);
}

bitmapFont* drawing::generateBitmapFont(std::string fontName, int fontStyle, int fontSize, int lineHeight, int spacing, int textureDimension)
{
	bitmapFont* font = new bitmapFont();

	ssfn_select(mFontContext, SSFN_FAMILY_ANY, fontName.c_str(), fontStyle, fontSize);

	int textureWidth = textureDimension;
	int textureHeight = textureDimension; 

	uint32_t* imageData = (uint32_t*)malloc(textureWidth * textureHeight * 4);
	memset(imageData, 0, textureWidth * textureHeight * 4);  

	int x = 2;
	int y = 2;

	char* charsToEncode = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4\xC2\xA5\xC2\xA6\xC2\xA7\xC2\xA8\xC2\xA9\xC2\xAA\xC2\xAB\xC2\xAC\xC2\xAD\xC2\xAE\xC2\xAF\xC2\xB0\xC2\xB1\xC2\xB2\xC2\xB3\xC2\xB4\xC2\xB5\xC2\xB6";
	char* currentCharPos = charsToEncode;
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

		Bounds bounds;
		int ret = ssfn_bbox(mFontContext, currentChar, &bounds.width, &bounds.height, &bounds.left, &bounds.top);
		if (ret != 0)
		{
			continue;
		}

		if ((x + bounds.width + 2) > textureWidth)
		{
			x = 2;
			y = y + bounds.height + 2;
		}

        std::string unicodeString = StringUtility::FormatString("%i", unicode);
		math::rectI* rect = &font->charMap[unicodeString];
		rect->x = x;
		rect->y = y;
		rect->width = bounds.width;
		rect->height = bounds.height;

		ssfn_buf_t buffer; 
		memset(&buffer, 0, sizeof(buffer));
		buffer.ptr = (uint8_t*)imageData;       
		buffer.x = x + bounds.left;
		buffer.y = y + bounds.top;
		buffer.w = textureWidth;                        
		buffer.h = textureHeight;                     
		buffer.p = textureWidth * 4;                          
		buffer.bg = 0xffffffff;
		buffer.fg = 0xffffffff;   

		ssfn_render(mFontContext, &buffer, currentChar);

		x = x + bounds.width + 2;   
		free(currentChar);
	}

	font->image = createImage((uint8_t*)imageData, D3DFMT_A8R8G8B8, textureWidth, textureHeight);
	font->lineHeight = lineHeight;
	font->spacing = spacing;
	free(imageData);

	return font;
}

void drawing::measureBitmapString(
    bitmapFont* font,
    std::string message,
    int* width,
    int* height)
{
    image* image = font->image;

    int xPosMax = 0;
    int xPos = 0;
    int yPos = 0;

    const char* currentCharPos = message.c_str();

    while (*currentCharPos)
    {
        const char* nextCharPos = currentCharPos;
        uint32_t unicode = ssfn_utf8((char**)&nextCharPos);

        int32_t length = static_cast<int32_t>(nextCharPos - currentCharPos);

        // Advance cursor early
        currentCharPos = nextCharPos;

        // Handle newline (ASCII only, same behavior as original)
        if (length == 1 && unicode == '\n')
        {
            xPos = 0;
            yPos += font->lineHeight;
            continue;
        }

        std::string unicodeString = StringUtility::FormatString("%i", unicode);

        std::map<std::string, math::rectI>::iterator it = font->charMap.find(unicodeString);
        if (it == font->charMap.end())
            continue;

        math::rectI* rect = &it->second;

        xPos += rect->width + font->spacing;
        xPosMax = max(xPosMax, xPos);
    }

    if (width)
        *width = xPosMax - 2;

    if (height)
        *height = yPos + font->lineHeight;
}

void drawing::drawBitmapString(bitmapFont* font, std::string message, uint32_t color, int x, int y)
{
    image* image = font->image;

    int xPos = x;
    int yPos = y;

    const char* currentCharPos = message.c_str();

    while (*currentCharPos)
    {
        const char* nextCharPos = currentCharPos;
        uint32_t unicode = ssfn_utf8((char**)&nextCharPos);

        int32_t length = static_cast<int32_t>(nextCharPos - currentCharPos);
        currentCharPos = nextCharPos;

        // Newline handling
        if (length == 1 && unicode == '\n')
        {
            xPos = x;
            yPos += font->lineHeight;
            continue;
        }

        // Lookup glyph (SAFE: no insertion)
        std::string unicodeString = StringUtility::FormatString("%i", unicode);
        std::map<std::string, math::rectI>::iterator it = font->charMap.find(unicodeString);
        if (it == font->charMap.end())
            continue;

        const math::rectI& rect = it->second;

        // Build UVs
        math::rectF uvRect;
        uvRect.x      = rect.x / static_cast<float>(image->width);
        uvRect.y      = rect.y / static_cast<float>(image->height);
        uvRect.width  = rect.width  / static_cast<float>(image->width);
        uvRect.height = rect.height / static_cast<float>(image->height);

        // Per-character color overrides
        uint32_t charColor = color;
        switch (unicode)
        {
            case 161: charColor = theme::getJoyButtonAColor(); break;
            case 162: charColor = theme::getJoyButtonBColor(); break;
            case 163: charColor = theme::getJoyButtonXColor(); break;
            case 164: charColor = theme::getJoyButtonYColor(); break;
            case 182: charColor = theme::getJoyButtonBlackColor(); break;
            case 181: charColor = theme::getJoyButtonWhiteColor(); break;
        }

        context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, charColor);

        float newY = static_cast<float>(context::getBufferHeight())
                   - (yPos + rect.height);

        context::getD3dDevice()->SetTexture(0, image->texture);

        utils::dataContainer* vertices =
            meshUtility::createQuadXY(
                math::vec3F(
                    static_cast<float>(xPos) - 0.5f,
                    newY - 0.5f,
                    0.0f),
                math::sizeF(
                    static_cast<float>(rect.width),
                    static_cast<float>(rect.height)),
                uvRect);

        context::getD3dDevice()->DrawPrimitiveUP(
            D3DPT_TRIANGLELIST,
            (vertices->size / sizeof(meshUtility::vertex)) / 3,
            vertices->data,
            sizeof(meshUtility::vertex));

        delete vertices;

        xPos += rect.width + font->spacing;
    }
}


void drawing::drawBitmapStringAligned(bitmapFont* font, std::string  message, uint32_t color, horizAlignment hAlign, int x, int y, int width)
{
	int textWidth = 0;
	measureBitmapString(font, message, &textWidth, NULL);

	int xPos = x;
	if (hAlign == horizAlignmentCenter)
	{
		xPos = x + ((width - textWidth) / 2);
	}
	else if (hAlign == horizAlignmentRight)
	{
		xPos = x + (width - textWidth);
	}

	drawBitmapString(font, message, color, xPos, y);
}
