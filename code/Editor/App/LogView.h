#ifndef traktor_editor_LogView_H
#define traktor_editor_LogView_H

#include "Ui/Container.h"

namespace traktor
{

class ILogTarget;

	namespace ui
	{

class PopupMenu;

		namespace custom
		{

class LogList;
class ToolBar;
class ToolBarButton;

		}
	}

	namespace editor
	{

class LogView : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void destroy();

	ILogTarget* getLogTarget() const { return m_logTarget; }

private:
	Ref< ui::custom::ToolBarButton > m_toolToggleInfo;
	Ref< ui::custom::ToolBarButton > m_toolToggleWarning;
	Ref< ui::custom::ToolBarButton > m_toolToggleError;
	Ref< ui::custom::ToolBar > m_toolFilter;
	Ref< ui::custom::LogList > m_log;
	Ref< ui::PopupMenu > m_popup;
	Ref< ILogTarget > m_logTarget;

	void eventToolClick(ui::Event* event);

	void eventButtonDown(ui::Event* event);
};

	}
}

#endif	// traktor_editor_LogView_H
