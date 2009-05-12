#ifndef traktor_editor_ObjectEditor_H
#define traktor_editor_ObjectEditor_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace db
	{

class Instance;

	}

	namespace editor
	{

/*! \brief Object editor base.
 *
 * Object editors are created as modal-less dialogs.
 */
class T_DLLCLASS ObjectEditor : public Object
{
	T_RTTI_CLASS(ObjectEditor)

public:
	virtual bool create(ui::Widget* parent, db::Instance* instance, Object* object) = 0;

	virtual void destroy() = 0;

	virtual void apply() = 0;
};

	}
}

#endif	// traktor_editor_ObjectEditor_H
