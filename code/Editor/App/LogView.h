#ifndef traktor_editor_LogView_H
#define traktor_editor_LogView_H

#include "Ui/Container.h"
#include "Ui/Custom/LogList/LogList.h"

namespace traktor
{

class ILogTarget;

	namespace ui
	{

class PopupMenu;

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

		}
	}

	namespace editor
	{

class IEditor;

class LogView
:	public ui::Container
,	public ui::custom::LogList::ISymbolLookup
{
	T_RTTI_CLASS;

public:
	LogView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	ILogTarget* getLogTarget() const { return m_logTarget; }

private:
	IEditor* m_editor;
	Ref< ui::custom::ToolBarButton > m_toolToggleInfo;
	Ref< ui::custom::ToolBarButton > m_toolToggleWarning;
	Ref< ui::custom::ToolBarButton > m_toolToggleError;
	Ref< ui::custom::ToolBar > m_toolFilter;
	Ref< ui::custom::LogList > m_log;
	Ref< ui::PopupMenu > m_popup;
	Ref< ILogTarget > m_logTarget;

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	virtual bool lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_editor_LogView_H
