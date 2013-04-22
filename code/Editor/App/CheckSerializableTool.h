#ifndef traktor_editor_CheckSerializableTool_H
#define traktor_editor_CheckSerializableTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class CheckSerializableTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, IEditor* editor);
};

	}
}

#endif	// traktor_editor_CheckSerializableTool_H
