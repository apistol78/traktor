#ifndef traktor_editor_EditorTool_H
#define traktor_editor_EditorTool_H

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

class Widget;

	}

	namespace editor
	{

class Editor;

/*! \brief Editor tool base class.
 * \ingroup Editor
 */
class T_DLLCLASS EditorTool : public Object
{
	T_RTTI_CLASS(EditorTool)

public:
	virtual std::wstring getDescription() const = 0;

	virtual bool launch(ui::Widget* parent, Editor* editor) = 0;
};

	}
}

#endif	// traktor_editor_EditorTool_H
