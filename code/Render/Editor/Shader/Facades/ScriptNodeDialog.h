#ifndef traktor_editor_ScriptNodeDialog_H
#define traktor_editor_ScriptNodeDialog_H

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;

		namespace custom
		{

class SyntaxRichEdit;

		}
	}

	namespace render
	{

class Script;

class ScriptNodeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	ScriptNodeDialog(Script* script);

	bool create(ui::Widget* parent);

private:
	Ref< Script > m_script;
	RefArray< ui::custom::SyntaxRichEdit > m_edit;

	void eventClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_ScriptNodeDialog_H
