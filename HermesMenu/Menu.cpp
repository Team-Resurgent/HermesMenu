#include "Menu.h"

#include "InputManager.h"
#include "Component.h"
#include "Context.h"
#include "Drawing.h"
#include "Theme.h"
#include "Defines.h"
#include "Trainer.h"
#include "Patcher.h"
#include "IsoChecker.h"
#include "IsoAttacher.h"

#include <vector>

Menu::Menu()
{
	selectedControl_ = 0;
    trainerData_ = NULL;
    trainerSize_ = 0;
}

Menu::~Menu()
{
	if (trainerData_ != NULL)
    {
        free(trainerData_);
        trainerData_ = NULL;
    }
}

bool Menu::LoadTrainer(std::string mountPath)
{
    mountPath_ = mountPath;

	if (trainerData_ != NULL)
    {
        free(trainerData_);
        trainerData_ = NULL;
    }

    trainerData_ = Trainer::LoadTrainer(mountPath_, &trainerSize_);
    if (trainerData_ != NULL)
    {
        trainerOptions_ = Trainer::GetOptions(trainerData_, trainerSize_);
        return trainerOptions_.size() > 2;
    }
    return false;
}

void Menu::Update()
{
	// Select Actions

	if (inputManager::buttonPressed(JoystickButtonA))
	{
        Trainer::ToggleOption(trainerData_, trainerSize_, selectedControl_);
	}

    if (inputManager::buttonPressed(JoystickButtonX))
	{
        Patcher::InstallTrainer(trainerData_, trainerSize_);

	    bool isCci = false;
	    std::vector<std::string> isos = IsoChecker::ValidateIsosInPath(mountPath_, XboxKrnlVersion->Build < 8008, isCci);
        
        if(isos.size() == 0) 
	    {
		    utils::debugPrint("Attach: No ISO's found.\n");
		    HalReturnToFirmware(2);
	    }

	    if (XboxKrnlVersion->Build >= 8008) {
		    utils::debugPrint("Detected Cerbios.\n");
            IsoAttacher::AttachCerbios(mountPath_, isos, isCci);
	    } else {
		    utils::debugPrint("Cerbios Not Detected.\n");
            IsoAttacher::AttachLegacy(mountPath_, isos, XboxKrnlVersion->Build);
	    }
    }

    // Up / Down Actions

    uint32_t numOptions = trainerOptions_.size() > 2 ? trainerOptions_.size() - 2 : 0;

	if (inputManager::buttonPressed(JoystickButtonDpadUp))
	{
		selectedControl_ = (selectedControl_ + numOptions - 1) % numOptions; 
	}

	if (inputManager::buttonPressed(JoystickButtonDpadDown))
	{
		selectedControl_ = (selectedControl_ + 1) % numOptions;
	}
}

void Menu::Render()
{
    uint32_t numOptions = trainerOptions_.size() > 2 ? trainerOptions_.size() - 2 : 0;

    drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "Hermes Menu", theme::getFooterTextColor(), horizAlignmentCenter, 193, theme::getHeaderY(), 322);
    drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), trainerOptions_.at(0), theme::getFooterTextColor(), horizAlignmentCenter, 193, theme::getHeaderY() + 35, 322);
    drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), trainerOptions_.at(1), theme::getFooterTextColor(), horizAlignmentCenter, 193, theme::getHeaderY() + 60, 322);

    int32_t start = 0;
    if ((int32_t)numOptions >= MAX_MENU_ITEMS) {
        start = min(max(selectedControl_ - (MAX_MENU_ITEMS / 2), 0), (int32_t)numOptions - MAX_MENU_ITEMS);
    }

    int32_t itemCount = min(start + MAX_MENU_ITEMS, (int32_t)numOptions) - start;
    if (itemCount > 0) {
        uint32_t yPos = (context::getBufferHeight() - ((itemCount * 40) - 10)) / 2;
        yPos += theme::getCenterOffset();
        for (int32_t i = 0; i < itemCount; i++) {
            uint32_t index = start + i;
            bool enabled = Trainer::GetOption(trainerData_, trainerSize_, selectedControl_);
            component::button(selectedControl_ == index, enabled, trainerOptions_.at(2 + i), 120, yPos, 480, 30);
        }
    }

    drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "\xC2\xA1 Toggle \xC2\xA3 Launch", theme::getFooterTextColor(), horizAlignmentCenter, 40, theme::getFooterY(), 640);
}