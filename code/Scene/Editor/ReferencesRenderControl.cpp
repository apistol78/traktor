#include "Scene/Editor/ReferencesRenderControl.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ReferencesRenderControl", ReferencesRenderControl, ISceneRenderControl)

ReferencesRenderControl::ReferencesRenderControl()
{
}

bool ReferencesRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	return true;
}

void ReferencesRenderControl::destroy()
{
}

void ReferencesRenderControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
}

bool ReferencesRenderControl::handleCommand(const ui::Command& command)
{
	return false;
}

void ReferencesRenderControl::update()
{
}

	}
}
