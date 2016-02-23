#ifndef traktor_editor_HttpGetTool_H
#define traktor_editor_HttpGetTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class HttpGetTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, IEditor* editor);
};

	}
}

#endif	// traktor_editor_HttpGetTool_H
