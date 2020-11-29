#pragma once

#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
	
class Button;
class CheckBox;
class ProgressBar;
class Static;

	}

	namespace shape
	{

class TracerPanel : public ui::Container
{
	T_RTTI_CLASS;

public:
	TracerPanel(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

private:
	editor::IEditor* m_editor;
	Ref< ui::CheckBox > m_checkEnable;
	Ref< ui::CheckBox > m_checkParallel;
	Ref< ui::Static > m_staticDescription;
	Ref< ui::ProgressBar > m_progressBar;
	Ref< ui::Button > m_buttonAbort;
};

	}
}
