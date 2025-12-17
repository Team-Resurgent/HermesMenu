#pragma once

#include "scene.h"

#include "..\xboxInternals.h"

class mainScene : public scene
{
public:
	mainScene();
	void update();
	void render();
	static bool launchXbe(const char* path);
private:
	int mSelectedControl;
	int mCounter;
};
