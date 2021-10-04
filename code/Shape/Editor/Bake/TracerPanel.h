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

	}

	namespace shape
	{

class TracerPanel : public ui::Container
{
	T_RTTI_CLASS;

public:
	explicit TracerPanel(editor::IEditor* editor);

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< ui::CheckBox > m_checkEnable;
	Ref< ui::ProgressBar > m_progressBar;
	Ref< ui::Button > m_buttonAbort;
	bool m_idle;
};

	}
}
