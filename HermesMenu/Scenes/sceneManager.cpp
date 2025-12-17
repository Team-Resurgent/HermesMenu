#include "scenemanager.h"
#include "scene.h"
#include "mainScene.h"

#include "..\context.h"
#include "..\stringUtility.h"
#include "..\pointerVector.h"

#include <xtl.h>

namespace 
{
	pointerVector<sceneContainer*>* mScenes = new pointerVector<sceneContainer*>(true);
}

scene* sceneManager::getScene()
{
	sceneContainer* result = mScenes->get(mScenes->count() - 1);
	return result->scene;
}

void sceneManager::pushScene(sceneItemEnum sceneItem)
{
	if (sceneItem == sceneItemMainScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new mainScene(), "V1");
		addScene(container);
	}
}

void sceneManager::pushScene(sceneContainer* container)
{
	addScene(container);
}

void sceneManager::popScene(sceneResult result)
{
	sceneContainer* popContainer = mScenes->get(mScenes->count() - 1);
	if (popContainer->onSceneClosingCallback != NULL)
	{
		popContainer->onSceneClosingCallback(result, popContainer->context, popContainer->scene);
	}

	mScenes->remove(mScenes->count() - 1);

}


// Private

void sceneManager::addScene(sceneContainer* sceneContainer)
{
	mScenes->add(sceneContainer);
}


bool sceneManager::isSceneItemActive(sceneItemEnum item)
{
	if (mScenes->count() == 0)
		return false;

	sceneContainer* top = mScenes->get(mScenes->count() - 1);
	return top->sceneItem == item;
}