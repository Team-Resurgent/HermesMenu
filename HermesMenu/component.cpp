#include "component.h"
#include "drawing.h"
#include "ssfn.h"
#include "theme.h"

#include <string>

void component::panel(uint32_t fill, uint32_t stroke, int x, int y, int width, int height)
{
	drawing::drawNinePatch("panel-fill", fill, x, y, width, height);
	drawing::drawNinePatch("panel-stroke", stroke, x, y, width, height);
}

void component::button(bool selected, bool active, std::string label, int x, int y, int width, int height)
{
	int textWidth;
	int textHeight;
	drawing::measureBitmapString(context::getBitmapFontSmall(), label, &textWidth, &textHeight);

	if (active)
	{
		if (selected) 
		{
			panel(theme::getButtonActiveHoverFillColor(), theme::getButtonActiveHoverStrokeColor(), x, y, width, height);
			drawing::drawBitmapString(context::getBitmapFontSmall(), label, theme::getButtonActiveHoverTextColor(), x + ((width - textWidth) / 2), y + ((height - textHeight) / 2) - 3);
		}
		else
		{
			panel(theme::getButtonActiveFillColor(), theme::getButtonActiveStrokeColor(), x, y, width, height);
			drawing::drawBitmapString(context::getBitmapFontSmall(), label, theme::getButtonActiveTextColor(), x + ((width - textWidth) / 2), y + ((height - textHeight) / 2) - 3);
		}
	}
	else
	{
		if (selected) 
		{
			panel(theme::getButtonInactiveHoverFillColor(), theme::getButtonInactiveHoverStrokeColor(), x, y, width, height);
			drawing::drawBitmapString(context::getBitmapFontSmall(), label, theme::getButtonInactiveHoverTextColor(), x + ((width - textWidth) / 2), y + ((height - textHeight) / 2) - 3);
		}
		else
		{
			panel(theme::getButtonInactiveFillColor(), theme::getButtonInactiveStrokeColor(), x, y, width, height);
			drawing::drawBitmapString(context::getBitmapFontSmall(), label, theme::getButtonInactiveTextColor(), x + ((width - textWidth) / 2), y + ((height - textHeight) / 2) - 3);
		}
	}
}

void component::text(std::string label, bool disabled, horizAlignment hAlign, int x, int y, int width, int height)
{
	int textWidth;
	int textHeight;
	drawing::measureBitmapString(context::getBitmapFontSmall(), label, &textWidth, &textHeight);

	int xPos = x;
	if (hAlign == horizAlignmentCenter)
	{
		xPos = x + ((width - textWidth) / 2);
	}
	else if (hAlign == horizAlignmentRight)
	{
		xPos = x + (width - textWidth);
	}

	drawing::drawBitmapString(context::getBitmapFontSmall(), label, disabled ? theme::getTextDisabledColor() : theme::getTextColor(), xPos, y + ((height - textHeight) / 2) - 3);
}
