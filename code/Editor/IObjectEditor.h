#ifndef traktor_editor_IObjectEditor_H
#define traktor_editor_IObjectEditor_H

#include "Core/Object.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

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
 * \ingroup Editor
 *
 * Object editors are created as modal-less dialogs.
 */
class T_DLLCLASS IObjectEditor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) = 0;

	virtual void destroy() = 0;

	virtual void apply() = 0;

	virtual ui::Size getPreferredSize() const = 0;
};

	}
}

#endif	// traktor_editor_IObjectEditor_H
