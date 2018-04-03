/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SearchControl_H
#define traktor_ui_custom_SearchControl_H

#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class ToolBar;
class ToolBarButton;

class T_DLLCLASS SearchControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	SearchControl();

	bool create(ui::Widget* parent);

	void setAnyMatchingHint(bool hint);

	virtual void setFocus() T_OVERRIDE T_FINAL;

	virtual void show() T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferedSize() const T_OVERRIDE T_FINAL;

private:
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
}

#endif	// traktor_ui_custom_SearchControl_H
