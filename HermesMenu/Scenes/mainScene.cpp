#include "mainScene.h"
#include "sceneManager.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\fileSystem.h"
#include "..\ssfn.h"
#include "..\inputManager.h"
#include "..\stringUtility.h"
#include "..\driveManager.h"
#include "..\theme.h"
#include "..\audioPlayer.h"


// --- Cerbios version helper
extern "C" ULONG __stdcall AvSendTVEncoderOption(PVOID, ULONG, ULONG, PULONG);

static const char* getCerbiosVersionString()
{
    static bool   initialized = false;
    static char   buf[32];

    if (!initialized)
    {
        initialized = true;

		ULONG version = 0;
		AvSendTVEncoderOption(NULL, 0x14, 0, &version);

        if (version != 0)
        {
            unsigned int maj = (version >> 24) & 0xFF;
            unsigned int min = (version >> 16) & 0xFF;
            unsigned int pat = (version >>  8) & 0xFF;

			_snprintf(buf, sizeof(buf), "Cerbios: V%u.%u.%u", maj, min, pat);
        }
        else
        {
            // Cerb 3.0.0 (and below)or other bios don't support this option
			strcpy(buf, "Cerbios: Unknown");
        }
    }

    return buf;
}


mainScene::mainScene()
{
	mSelectedControl = 0;
}

const int TotalMenuItems = 6;

void mainScene::update()
{
	// Select Actions

	if (inputManager::buttonPressed(JoystickButtonA))
	{
        if (mSelectedControl == 0) 
		{
			//sceneManager::pushScene(sceneItemCerbiosIniEditorScene);
			return;
		}
		
		
	}

    // Down Actions

	if (inputManager::buttonPressed(JoystickButtonDpadDown))
	{
		mSelectedControl = (mSelectedControl + 1) % TotalMenuItems;
	}

	// Up Actions

	if (inputManager::buttonPressed(JoystickButtonDpadUp))
	{
		mSelectedControl = (mSelectedControl + TotalMenuItems - 1) % TotalMenuItems; 
	}
}

void mainScene::render()
{
    // Panel + menu
    component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);

    int32_t yPos = (context::getBufferHeight() - (6 * 40) - 10) / 2;
    yPos += theme::getCenterOffset();

    drawing::drawBitmapStringAligned(context::getBitmapFontMedium(), "Hermes Menu", theme::getFooterTextColor(), horizAlignmentCenter, 193, theme::getHeaderY(), 322);
    drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "007 Agent Under Fire +1", theme::getFooterTextColor(), horizAlignmentCenter, 193, theme::getHeaderY() + 30, 322);
    
    component::button(mSelectedControl == 0, false, "Infinte Ammo", 120, yPos, 480, 30); 
    yPos += 40;
    component::button(mSelectedControl == 1, false, "Ininite Health", 120, yPos, 480, 30); 
    yPos += 40;
    component::button(mSelectedControl == 2, false, "Max Damage ", 120, yPos, 480, 30); 
    yPos += 40;

}



bool mainScene::launchXbe(const char* path)
{
	int result = 0;

	char* mountPointToDelete = strdup("\\??\\D:");
	STRING sMountPointToDelete = {(USHORT)strlen(mountPointToDelete), (USHORT)strlen(mountPointToDelete) + 1, mountPointToDelete};
	result |= IoDeleteSymbolicLink(&sMountPointToDelete);

	char* mountPoint = strdup("\\??\\D:");
	STRING sMountPoint = {(USHORT)strlen(mountPoint), (USHORT)strlen(mountPoint) + 1, mountPoint};
	char* tempPath = fileSystem::getDirectory(path);
	char* systemPath = driveManager::convertMountPointToSystemPath(tempPath);
	STRING sSystemPath = {(USHORT)strlen(systemPath), (USHORT)strlen(systemPath) + 1, systemPath};
	result |= IoCreateSymbolicLink(&sMountPoint, &sSystemPath);

	if (result != 0)
	{
		return false;
	}

	free(mountPointToDelete);
	free(mountPoint);
	free(tempPath);
	free(systemPath);

	char* fileName = fileSystem::getFileName(path);
	char* launchPath = stringUtility::formatString("D:\\%s", fileName);
	free(fileName);

	XLaunchNewImage(launchPath, NULL);
	return false;
}