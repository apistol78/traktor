#ifndef traktor_script_SearchControl_H
#define traktor_script_SearchControl_H

#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Edit;

		namespace custom
		{

class ToolBar;
class ToolBarButton;

		}
	}

	namespace script
	{

class SearchControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	SearchControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void setAnyMatchingHint(bool hint);

	virtual void setFocus() T_OVERRIDE T_FINAL;

	virtual void show() T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferedSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< ui::Edit > m_editSearch;
	Ref< ui::custom::ToolBar > m_toolBarMode;
	Ref< ui::custom::ToolBarButton > m_toolCaseSensitive;
	Ref< ui::custom::ToolBarButton > m_toolWholeWord;
	Ref< ui::custom::ToolBarButton > m_toolWildCard;

	void eventEditSearchKeyDown(ui::KeyDownEvent* event);

	void eventEditChange(ui::ContentChangeEvent* event);
};

	}
}

#endif	// traktor_script_SearchControl_H
