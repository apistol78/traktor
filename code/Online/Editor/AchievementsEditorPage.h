#ifndef traktor_online_AchievementsEditorPage_H
#define traktor_online_AchievementsEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Event;

		namespace custom
		{

class GridView;

		}
	}

	namespace online
	{

class T_DLLCLASS AchievementsEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	AchievementsEditorPage(editor::IEditor* editor);

	virtual bool create(ui::Container* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Ref< db::Instance > getDataInstance();

	virtual Ref< Object > getDataObject();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::GridView > m_gridAchievements;
};

	}
}

#endif	// traktor_online_AchievementsEditorPage_H
