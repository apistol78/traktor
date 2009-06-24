#ifndef traktor_editor_IEditorPageFactory_H
#define traktor_editor_IEditorPageFactory_H

#include <list>
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

class Command;

	}

	namespace editor
	{

class IEditor;
class IEditorPage;

/*! \brief Interface for creating editor pages.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPageFactory : public Object
{
	T_RTTI_CLASS(IEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const = 0;

	virtual IEditorPage* createEditorPage(IEditor* editor) const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;
};

	}
}

#endif	// traktor_editor_IEditorPageFactory_H
