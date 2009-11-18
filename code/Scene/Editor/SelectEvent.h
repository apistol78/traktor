#ifndef traktor_scene_SelectEvent_H
#define traktor_scene_SelectEvent_H

#include "Core/RefArray.h"
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class EntityAdapter;

class T_DLLCLASS SelectEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	SelectEvent(ui::EventSubject* sender, const RefArray< EntityAdapter >& selectedEntities);

	const RefArray< EntityAdapter >& getSelectedEntities() const;

private:
	RefArray< EntityAdapter > m_selectedEntities;
};

	}
}

#endif	// traktor_scene_SelectEvent_H
