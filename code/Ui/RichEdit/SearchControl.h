#pragma once

#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

class T_DLLCLASS SearchControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void setNeedle(const std::wstring& needle);

	std::wstring getNeedle() const;

	bool caseSensitive() const;

	bool wholeWord() const;

	bool wildcard() const;

	void setAnyMatchingHint(bool hint);

	virtual void setFocus() override final;

	virtual void show() override final;

	virtual ui::Size getPreferredSize(const Size& hint) const override final;

private:
	Ref< ui::Edit > m_editSearch;
	Ref< ui::ToolBar > m_toolBarMode;
	Ref< ui::ToolBarButton > m_toolCaseSensitive;
	Ref< ui::ToolBarButton > m_toolWholeWord;
	Ref< ui::ToolBarButton > m_toolWildCard;

	void eventEditSearchKeyDown(ui::KeyDownEvent* event);

	void eventEditChange(ui::ContentChangeEvent* event);

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventContentChange(ui::ContentChangeEvent* event);
};

	}
}

