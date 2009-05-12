#ifndef traktor_editor_EditorPage_H
#define traktor_editor_EditorPage_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;
class Event;

	}

	namespace ui
	{

class Container;
class Point;
class Command;

	}

	namespace editor
	{

/*! \brief Editor page base.
 *
 * The editor use editor pages as the base for all supported editors.
 * EditorPage objects are created through specialized EditorPageFactory classes.
 */
class T_DLLCLASS EditorPage : public Object
{
	T_RTTI_CLASS(EditorPage)

public:
	virtual bool create(ui::Container* parent) = 0;

	virtual void destroy() = 0;

	virtual void activate() = 0;

	virtual void deactivate() = 0;

	virtual	bool setDataObject(db::Instance* instance, Object* data) = 0;

	virtual Object* getDataObject() = 0;

	virtual void propertiesChanged() = 0;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void handleDatabaseEvent(const Guid& eventId) = 0;
};

	}
}

#endif	// traktor_editor_EditorPage_H
