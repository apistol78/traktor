/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Dialog_H
#define traktor_ui_Dialog_H

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

class ChildEvent;
class IBitmap;

/*! \brief Dialog
 * \ingroup UI
 */
class T_DLLCLASS Dialog : public Container
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsCenterParent = 0,
		WsCenterDesktop = WsUser, 
		WsDefaultFixed = WsCenterParent | WsSystemBox | WsMinimizeBox | WsCloseBox | WsCaption,
		WsDefaultResizable = WsCenterParent | WsResizable | WsSystemBox | WsMinimizeBox | WsMaximizeBox | WsCloseBox | WsCaption
	};

	Dialog();

	bool create(Widget* parent, const std::wstring& text, int width, int height, int style = WsDefaultResizable, Layout* layout = 0);

	void setIcon(IBitmap* icon);
	
	virtual int showModal();

	virtual void endModal(int result);

	bool isModal() const;

	virtual bool acceptLayout() const T_OVERRIDE;

private:
	bool m_modal;

	void eventChild(ChildEvent* event);
};

	}
}

#endif	// traktor_ui_Dialog_H
