#ifndef traktor_editor_IEditorTool_H
#define traktor_editor_IEditorTool_H

#include <string>
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

class IBitmap;
class Widget;

	}

	namespace editor
	{

class IEditor;

/*! \brief Editor tool base class.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorTool : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const = 0;

	virtual Ref< ui::IBitmap > getIcon() const = 0;

	virtual bool launch(ui::Widget* parent, IEditor* editor) = 0;
};

	}
}

#endif	// traktor_editor_IEditorTool_H
