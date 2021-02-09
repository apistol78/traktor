#pragma once

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class ISceneRenderControl;

class T_DLLCLASS RedrawEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	explicit RedrawEvent(ui::EventSubject* sender, ISceneRenderControl* renderControl);

	ISceneRenderControl* getRenderControl() const { return m_renderControl; }

private:
	ISceneRenderControl* m_renderControl;
};

	}
}

