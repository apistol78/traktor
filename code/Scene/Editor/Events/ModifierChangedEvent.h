#ifndef traktor_scene_ModifierChangedEvent_H
#define traktor_scene_ModifierChangedEvent_H

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

class T_DLLCLASS ModifierChangedEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	ModifierChangedEvent(ui::EventSubject* sender);
};

	}
}

#endif	// traktor_scene_ModifierChangedEvent_H
