#ifndef traktor_editor_GenerateClassListTool_H
#define traktor_editor_GenerateClassListTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class GenerateClassListTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, IEditor* editor);
};

	}
}

#endif	// traktor_editor_GenerateClassListTool_H
