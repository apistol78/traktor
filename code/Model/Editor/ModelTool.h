#ifndef traktor_model_ModelTool_H
#define traktor_model_ModelTool_H

#include <Editor/IEditorTool.h>

namespace traktor
{
	namespace model
	{

class ModelToolDialog;

class ModelTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual ~ModelTool();

	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor);

private:
	Ref< ModelToolDialog > m_dialog;
};

	}
}

#endif	// traktor_model_ModelTool_H
