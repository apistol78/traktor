#ifndef traktor_editor_SearchTool_H
#define traktor_editor_SearchTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class SearchToolDialog;

class SearchTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, IEditor* editor);

private:
	Ref< SearchToolDialog > m_searchDialog;
};

	}
}

#endif	// traktor_editor_SearchTool_H
